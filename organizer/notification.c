#include <stdbool.h>

#include <libnotify/notify.h>

#include <organizer/common.h>
#include <organizer/notification.h>

bool initNotification()
{
	return notify_init("organizer");
}

void sendMovingNotification(char* message)
{
	if (notify_is_initted())
	{
		char summary[] = "Files Organized";
		NotifyNotification* notification = notify_notification_new(summary, message, NULL);
		GError* gerror = NULL;
		if (notify_notification_show(notification, &gerror) == false)
		{
			writeWarning("Failed to send notification (%s: %s): %s", summary, message, gerror->message);
			g_error_free(gerror);
		}
	}
	else
	{
		writeWarning("Unitialized libnotify while attempting to display notification");
	}
}

void sendPausedNotification(char* message)
{
	sendMovingNotification(message); // TODO: implement paused state
}
