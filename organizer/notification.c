#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <libnotify/notify.h>
#include <pthread.h>

#include <organizer/common.h>
#include <organizer/files.h>
#include <organizer/notification.h>

pthread_t gtk_thread;

NotifyNotification* notification = NULL;

void* gtkRunner(void* data)
{
	gtk_main();
	writeDebug("gtk_main exited");
	return NULL;
}

bool initNotification()
{
	bool success = notify_init("Organizer");
	if (notificationActionsAllowed())
	{
		pthread_create(&gtk_thread, NULL, &gtkRunner, NULL);
	}
	return success;
}

void freeNotification()
{
	notify_uninit();
}

/**
 * used by libnotify to free some data after being passed to a button handler
 * TODO: implement resource deallocation on SIGTERM so this actually get called
 * @param data the FileChange to be freed
 */
void freeNotificationFileMovement(void* data)
{
	FileChange_free(data);
	g_free(data);
	if (pthread_equal(gtk_thread, pthread_self()))
	{
		writeDebug("freeNotificationFileMovement called from gtk thread");
	}
	else
	{
		writeDebug("freeNotificationFileMovement called from master thread");
	}
}

/**
 * called when the Undo button is pressed on a notification
 * !!! is called from the gtk thread, not the master !!!
 * @param notification the notification who's button got pushed (bit of a temper)
 * @param action       the name of the action performed
 * @param data         pointer to the FileChange that represents the movement
 */
void notificationUndoAction(NotifyNotification* notification, char* action, void* data)
{
	FileChange* file_change = (FileChange*)data;
	char* destination = dirname(file_change->from);
	moveFile(((FileChange*)file_change)->to, destination);
}

/**
 * called when the Open button is pressed on a notification
 * @param notification the notification who's button got pushed
 * @param action       the name of the action performed
 * @param data         pointer to the FileChange that represents the movement
 */
void notificationOpenAction(NotifyNotification* notification, char* action, void* data)
{
	char* cmd;

	if (asprintf(&cmd, "xdg-open \"%s\"", ((FileChange*)data)->to) == -1)
	{
		writeWarning("Failed memory allocation in notificationOpenAction");
		return;
	}

	FILE* _stdout = popen(cmd, "r");
	if (_stdout)
	{
		if (pclose(_stdout) == -1)
		{
			if (errno == ECHILD)
			{
				writeError("notificationOpenAction->pclose wasn't able to find the specified child PID (it may have ended too quickly)");
			}
			else if (errno == EINTR)
			{
				writeError("notificationOpenAction->pclose sent an invalid signal to the child");
			}
			else if (errno == EINVAL)
			{
				writeError("notificationOpenAction->pclose->wait4 received invalid options");
			}
		}
	}
	else if (errno == EINVAL)
	{
		writeError("notificationOpenAction->popen received an invalid type");
	}
	// pipe errors
	else if (errno == EFAULT)
	{
		writeError("notificationOpenAction->popen->pipe received an invalid file descriptor");
	}
	else if (errno == EINVAL)
	{
		writeError("notificationOpenAction->popen->pipe received invalid flags");
	}
	else if (errno == EMFILE)
	{
		writeError("notificationOpenAction->popen->pipe was unsuccessful because the process is using too many file descriptors");
	}
	else if (errno == ENFILE)
	{
		writeError("notificationOpenAction->popen->pipe was unsuccessful because the system limit on the total number of open files has been reached");
	}
	// fork errors
	else if (errno == EAGAIN)
	{
		writeError(
			"notificationOpenAction->popen->fork ran out of threads for the current user OR couldn't allocate enough memory for the parent's page tables and task structure");
	}
	else if (errno == ENOMEM)
	{
		writeError("notificationOpenAction->popen->fork failed to allocate the necessary kernel structures because memory is tight");
	}
	else if (errno == ENOSYS)
	{
		writeError("notificationOpenAction->popen->fork is not supported");
	}
	else
	{
		writeWarning("popen was unable to allocate memory in notificationOpenAction");
	}

	free(cmd);
}

void sendMovingNotification(char* from, char* dest)
{
	if (notify_is_initted())
	{
		char message[BUF_SIZE];
		snprintf(message, BUF_SIZE, "Moved \"%s\" to \"%s\"", from, dest);
		char summary[] = "File Organized";
		if (notification)
		{
			notify_notification_update(notification, summary, message, NULL);
		}
		else
		{
			notification = notify_notification_new(summary, message, NULL);

			if (notificationActionsAllowed())
			{
				FileChange* file_change = malloc(sizeof(FileChange));
				FileChange_init(file_change, dest, from);
				notify_notification_add_action(notification,
				                               "undo",
				                               "Undo",
				                               &notificationUndoAction,
				                               file_change,
				                               &freeNotificationFileMovement);
				notify_notification_add_action(notification,
				                               "open",
				                               "Open",
				                               &notificationOpenAction,
				                               file_change,
				                               &freeNotificationFileMovement);
			}
		}

		GError* gerror = NULL;
		if (notify_notification_show(notification, &gerror) == false)
		{
			writeWarning("Failed to send notification (%s - %s): %s", summary, message, gerror->message);
			g_error_free(gerror);
		}
	}
	else
	{
		writeWarning("libnotify was uninitialized while attempting to display a notification");
	}
}

void sendPausedNotification(char* from, char* dest)
{
	sendMovingNotification(from, dest); // TODO: implement paused state
}

bool notificationActionsAllowed()
{
	GList* capabilities = notify_get_server_caps();
	bool allowed;
	if (g_list_find_custom(capabilities, "actions", (GCompareFunc)g_strcmp0) == NULL)
	{
		allowed = false;
	}
	else
	{
		allowed = true;
	}
	g_list_free_full(capabilities, &g_free);

	return allowed;
}
