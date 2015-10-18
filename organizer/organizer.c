#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <organizer/common.h>
#include <organizer/files.h>
#include <organizer/notification.h>

int main(int argc, char** argv)
{
	int return_status = 0;
	char* user_dir = getpwuid(getuid())->pw_dir;
	char watch_dir[FILE_NAME_MAX];
	char err_file_name[FILE_NAME_MAX];

	snprintf(err_file_name, FILE_NAME_MAX, "%s/%s", user_dir, "organizer.log");
	FILE* err_file = fopen(err_file_name, "w");
	setLogFiles(err_file, err_file, NULL);

	snprintf(watch_dir, FILE_NAME_MAX, "%s/%s", user_dir, "Downloads/");

	writeDebug("Watch directory: %s", watch_dir);
	writeDebug("Log file: %s", err_file_name);

	InotifyDetail inotify_detail = initInotify(watch_dir);
	if (inotify_detail.inotify_fd < 0)
	{
		return_status = 1;
		goto EXIT;
	}
	else if (inotify_detail.watch_desc < 0)
	{
		return_status = 2;
		goto EXIT;
	}
	writeDebug("Created watch %d from inotify object %d", inotify_detail.watch_desc, inotify_detail.inotify_fd);

	if (!initNotification())
	{
		writeError("Unable to initialize inotify");
	}

	return_status = process(inotify_detail.inotify_fd, watch_dir);

	EXIT:
	closeInotify();
	freeNotification();
	return return_status;
}
