#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#include <organizer/common.h>
#include <organizer/files.h>
#include <organizer/notification.h>

char* watch_dir = NULL;
int inotify_fd = -1;
int watch_desc = -1;

FileChange* FileChange_init(FileChange* fchange, char* to, char* from)
{
	fchange->to = malloc(strlen(to) + 1);
	if (fchange->to)
	{
		strcpy(fchange->to, to);
		fchange->from = malloc(strlen(from) + 1);
		if (fchange->from)
		{
			strcpy(fchange->from, from);
		}
		else
		{
			writeError("FileChange.from allocation failed with size %d", strlen(from) + 1);
			return NULL;
		}
	}
	else
	{
		writeError("FileChange.to allocation failed with size %d", strlen(to) + 1);
		return NULL;
	}

	return fchange;
}

void FileChange_free(FileChange* fchange)
{
	free(fchange->to);
	free(fchange->from);
}

int process(int fd, char* watch_dir)
{
	struct inotify_event* event;
	int length = 0;
	char buffer[BUF_SIZE];

	length = read(fd, buffer, BUF_SIZE);
	for (int i = 0; i < length;)
	{
		event = (struct inotify_event*)(buffer + i);
		printf("\n");
		printEvent(event);
		char* extension;

		if (event->mask & IN_DELETE_SELF)
		{
			writeWarning("Watch directory was deleted");
			return 1;
		}
		else if ((extension = strstr(event->name, ".")) != NULL)
		{
			size_t name_len = strlen(watch_dir) + strlen(event->name) + 1;
			char* fullname = malloc(name_len);
			snprintf(fullname, name_len, "%s%s", watch_dir, event->name);

			int file_size = getFileSize(fullname);
			printf("File size: %d\n", file_size);

			if (file_size > 0) // firefox likes to create empty files for the temp files to be renamed to
			{
				printf("Extension: %s\n", extension);
				if (strend(extension, ".txt") || strend(extension, ".pdf"))
				{
					moveFile(fullname, "/home/calcifer/Documents");
				}
				else if (strend(extension, ".tar.gz") || strend(extension, ".tar") || strend(extension, ".tar.bz2") ||
				         strend(extension, ".zip") || strend(extension, ".7z") || strend(extension, ".ar") ||
				         strend(extension, ".ace"))
				{
					moveFile(fullname, "/home/calcifer/Documents/archives");
				}
				else if (strend(extension, ".deb") || strend(extension, ".rpm") ||
				         strend(extension, ".msi") ||
				         strend(extension, ".apk") ||
				         strend(extension, ".app"))
				{
					moveFile(fullname, "/home/calcifer/Documents/installers");
				}
				else if (strend(extension, ".torrent"))
				{
					moveFile(fullname, "/home/calcifer/Downloads/torrents");
				}
				else if (strend(extension, ".mp4") || strend(extension, ".video") || strend(extension, ".mkv") || strend(extension, ".avi"))
				{
					moveFile(fullname, "/home/calcifer/Videos");
				}
				else if (strend(extension, ".odt") || strend(extension, ".ods") || strend(extension, ".odp") ||
				         strend(extension, ".doc") || strend(extension, ".xls") || strend(extension, ".ppt") ||
				         strend(extension, ".docx") || strend(extension, ".xlsx") || strend(extension, ".pptx"))
				{
					moveFile(fullname, "/home/calcifer/Documents/office");
				}
				else if (strend(extension, ".ovpn"))
				{
					moveFile(fullname, "/home/calcifer/Web");
				}
				else if (strend(extension, ".mp3"))
				{
					moveFile(fullname, "/home/calcifer/Music");
				}
				else if (strend(extension, ".gif"))
				{
					moveFile(fullname, "/home/calcifer/Pictures/gifs");
				}
				else if (strend(extension, ".jpg") || strend(extension, ".jpeg") ||
				         strend(extension, ".png") || strend(extension, ".bmp") ||
				         strend(extension, ".svg") ||
				         strend(extension, ".tif") || strend(extension, ".tiff"))
				{
					moveFile(fullname, "/home/calcifer/Pictures");
				}
				else if (strend(extension, ".c") || strend(extension, ".h") ||
				         strend(extension, ".cpp") || strend(extension, ".hpp") ||
				         strend(extension, ".py") || strend(extension, ".py3") ||
				         strend(extension, ".rb") ||
				         strend(extension, ".cs") ||
				         strend(extension, ".php") || strend(extension, ".js") || strend(extension, ".html") ||
				         strend(extension, ".java"))
				{
					moveFile(fullname, "/home/calcifer/Documents/code/unorganized");
				}
				else if (strend(extension, ".iso") || strend(extension, ".vbox") || strend(extension, ".vdi"))
				{
					moveFile(fullname, "/home/calcifer/Documents/disc-images");
				}
				else if (strend(extension, ".desktop"))
				{
					moveFile(fullname, "/home/calcifer/Desktop");
				}
			}
			free(fullname);
		}

		i += sizeof(struct inotify_event) + event->len;
	}

	return 0; // shouldn't ever be possible, infinite loop
}

int getFileSize(char* filename)
{
	if (access(filename, F_OK) == -1) // check to make sure the file exists
	{
		return -1;
	}
	else
	{
		struct stat st;
		stat(filename, &st);
		return st.st_size;
	}
}

void moveFile(char* from, char* destination) // TODO: try to reduce the amount of dynamic memory allocation
{
	char* filename = strrfind(from, '/') + 1;
	size_t name_len = strlen(destination) + strlen(filename) + 1;
	char* end_name;
	if (!strend(destination, "/"))
	{
		++name_len;
		end_name = malloc(name_len);
		snprintf(end_name, name_len, "%s/%s", destination, filename);
	}
	else
	{
		end_name = malloc(name_len);
		snprintf(end_name, name_len, "%s%s", destination, filename);
	}

	// add (some number) to the end of the filename to avoid overwriting data
	char* safe_name = malloc(name_len);
	strcpy(safe_name, end_name);
	size_t safe_len;
	for (int m = 1; access(safe_name, F_OK) != -1; ++m)
	{
		free(safe_name);
		safe_len = name_len + getDigitCount(m) + 3;
		safe_name = malloc(safe_len);
		snprintf(safe_name, safe_len, "%s (%d)", end_name, m);
	}
	free(end_name);
	end_name = safe_name;
	name_len = safe_len; // shouldn't be necessary, but it keeps a safe state

	// write to status log
	writeDebug("Rename \"%s\" to \"%s\"", from, end_name);
	if (rename(from, end_name) < 0)
	{
		writeWarning("Unable to move file to destination");
	}
	else
	{
		// send notification
		char notify_message[FILE_NAME_MAX];
		snprintf(notify_message, FILE_NAME_MAX, "Rename \"%s\" to \"%s\"", from , end_name);
		if (isPaused())
		{
			sendPausedNotification(from, end_name);
		}
		else
		{
			sendMovingNotification(from, end_name);
		}
	}
	free(end_name);
}

InotifyDetail initInotify(char* dirname)
{
	InotifyDetail detail = {
		.inotify_fd = -1,
		.watch_desc = -1
	};

	detail.inotify_fd = inotify_init1(0);
	if (detail.inotify_fd < 0)
	{
		writeError("Failed to create inotify object");
		return detail;
	}

	detail.watch_desc = inotify_add_watch(detail.inotify_fd, dirname, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF);
	if (detail.watch_desc < 0)
	{
		writeError("Failed to add watch to inotify object");
		return detail;
	}

	inotify_fd = detail.inotify_fd;
	watch_desc = detail.watch_desc;
	watch_dir = malloc(strlen(dirname) + 1);
	strcpy(watch_dir, dirname);

	return detail;
}

void closeInotify()
{
	if (inotify_fd >= 0)
	{
		if (watch_desc >= 0)
		{
			inotify_rm_watch(inotify_fd, watch_desc);
			free(watch_dir);
		}
		close(inotify_fd);
	}
}

void printEvent(struct inotify_event* event)
{
	printf("inotify_event(Watch: %d, Mask(", event->wd);

	const char* type_str;
	uint32_t i = 0;
	for (; i < 32; ++i)
	{
		type_str = eventMaskString(event->mask & (1 << i));
		if (strlen(type_str) > 0)
		{
			printf("%s", type_str);
			i++;
			break;
		}
	}

	for (; i < 32; ++i)
	{
		type_str = eventMaskString(event->mask & (1 << i));
		if (strlen(type_str) > 0)
		{
			printf("|%s", type_str);
		}
	}

	printf("), ID: %d, Name: %s)\n", event->cookie, event->name);
}

const char* const ACCESS = "IN_ACCESS";
const char* const ATTRIB = "IN_ATTRIB";
const char* const CLOSE_WRITE = "IN_CLOSE_WRITE";
const char* const CLOSE_NOWRITE = "IN_CLOSE_NOWRITE";
const char* const CREATE = "IN_CREATE";
const char* const DELETE = "IN_DELETE";
const char* const DELETE_SELF = "IN_DELETE_SELF";
const char* const MODIFY = "IN_MODIFY";
const char* const MOVE_SELF = "IN_MOVE_SELF";
const char* const MOVED_FROM = "IN_MOVED_FROM";
const char* const MOVED_TO = "IN_MOVED_TO";
const char* const OPEN = "IN_OPEN";
const char* const NO_EVENT = "";

const char* eventMaskString(uint32_t mask)
{
	if (mask & IN_ACCESS)
	{
		return ACCESS;
	}
	else if (mask & IN_MODIFY)
	{
		return MODIFY;
	}
	else if (mask & IN_ATTRIB)
	{
		return ATTRIB;
	}
	else if (mask & IN_CLOSE_WRITE)
	{
		return CLOSE_WRITE;
	}
	else if (mask & IN_CLOSE_NOWRITE)
	{
		return CLOSE_NOWRITE;
	}
	else if (mask & IN_CREATE)
	{
		return CREATE;
	}
	else if (mask & IN_OPEN)
	{
		return OPEN;
	}
	else if (mask & IN_MOVED_FROM)
	{
		return MOVED_FROM;
	}
	else if (mask & IN_MOVED_TO)
	{
		return MOVED_TO;
	}
	else if (mask & IN_DELETE)
	{
		return DELETE;
	}
	else if (mask & IN_DELETE_SELF)
	{
		return DELETE_SELF;
	}
	else if (mask & IN_MOVE_SELF)
	{
		return MOVE_SELF;
	}
	else
	{
		return NO_EVENT;
	}
}
