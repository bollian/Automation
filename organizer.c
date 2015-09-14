#include <alloca.h>
#include <errno.h>
// TODO: catch interrupts #include <linux/sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#define NAME_MAX 256
#define BUF_SIZE (1024 * (sizeof(struct inotify_event) + NAME_MAX))

int getFileSize(char*);
void moveFile(char*, char*);
const char* eventMaskString(uint32_t);
void printEvent(struct inotify_event*);

// string ops
char* strrfind(char*, char);
bool strend(char*, char*);

size_t getDigitCount(int);

int main(int argc, char** argv)
{
	int return_status = 0;
	char watch_dir[NAME_MAX];
	snprintf(watch_dir, NAME_MAX, "%s%s", getpwuid(getuid())->pw_dir, "/Downloads/");
	printf("Watch directory: %s\n", watch_dir);
	
	int inotify_fd = inotify_init1(0);
	if (inotify_fd < 0)
	{
		perror("Failed to create inotify object");
		return_status = 1;
		goto EXIT;
	}
	
	int watch_desc = inotify_add_watch(inotify_fd, watch_dir, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF);
	if (watch_desc < 0)
	{
		perror("Failed to add watch to inotify object");
		return_status = 1;
		goto EXIT;
	}
	printf("Created watch %d from inotify object %d\n", watch_desc, inotify_fd);
	
	struct inotify_event* event;
	int length = 0;
	char buffer[BUF_SIZE];
	
	while (true)
	{
		length = read(inotify_fd, buffer, BUF_SIZE);
		for (int i = 0; i < length;)
		{
			event = (struct inotify_event*)(buffer + i);
			printf("\n");
			printEvent(event);
			
			if (event->mask & IN_DELETE_SELF)
			{
				perror("Watch directory was deleted");
				return_status = 2;
				goto EXIT;
			}
			else // if (event->mask & (IN_MOVED_TO | IN_CLOSE_WRITE)) inotify allows event filtering, making condition unnecessary
			{
				char* extension = strstr(event->name, ".");
				
				size_t name_len = strlen(watch_dir) + strlen(event->name) + 1;
				char* fullname = malloc(name_len);
				snprintf(fullname, name_len, "%s%s", watch_dir, event->name);
				
				int file_size = getFileSize(fullname);
				printf("File size: %d\n", file_size);
				
				if (file_size) // firefox likes to create empty files for the temp files to be renamed to
				{
					printf("Extension: %s\n", extension);
					if (strend(extension, ".txt") || strend(extension, ".pdf"))
					{
						moveFile("/home/calcifer/Documents", fullname);
					}
					else if (strend(extension, ".tar.gz") || strend(extension, ".tar") || strend(extension, ".tar.bz2") ||
					      strend(extension, ".zip") || strend(extension, ".7z") || strend(extension, ".ar") ||
					      strend(extension, ".ace"))
					{
						moveFile("/home/calcifer/Documents/archives", fullname);
					}
					else if (strend(extension, ".deb") || strend(extension, ".rpm") ||
					      strend(extension, ".msi") ||
					      strend(extension, ".apk") ||
					      strend(extension, ".app"))
					{
						moveFile("/home/calcifer/Documents/installers", fullname);
					}
					else if (strend(extension, ".torrent"))
					{
						moveFile("/home/calcifer/Downloads/torrents", fullname);
					}
					else if (strend(extension, ".mp4") || strend(extension, ".video") || strend(extension, ".mkv") || strend(extension, ".avi"))
					{
						moveFile("/home/calcifer/Videos", fullname);
					}
					else if (strend(extension, ".odt") || strend(extension, ".ods") || strend(extension, ".odp") ||
					      strend(extension, ".doc") || strend(extension, ".xls") || strend(extension, ".ppt") ||
					      strend(extension, ".docx") || strend(extension, ".xlsx") || strend(extension, ".pptx"))
					{
						moveFile("/home/calcifer/Documents/office", fullname);
					}
					else if (strend(extension, ".ovpn"))
					{
						moveFile("/home/calcifer/Web", fullname);
					}
					else if (strend(extension, ".mp3"))
					{
						moveFile("/home/calcifer/Music", fullname);
					}
					else if (strend(extension, ".gif"))
					{
						moveFile("/home/calcifer/Pictures/gifs", fullname);
					}
					else if (strend(extension, ".jpg") || strend(extension, ".jpeg") || 
					      strend(extension, ".png") || strend(extension, ".bmp") || 
					      strend(extension, ".svg") || 
					      strend(extension, ".tif") || strend(extension, ".tiff"))
					{
						moveFile("/home/calcifer/Pictures", fullname);
					}
					else if (strend(extension, ".c") || strend(extension, ".h") || 
					      strend(extension, ".cpp") || strend(extension, ".hpp") || 
					      strend(extension, ".py") || strend(extension, ".py3") || 
					      strend(extension, ".rb") || 
					      strend(extension, ".cs") || 
					      strend(extension, ".php") || strend(extension, ".js") || strend(extension, ".html") || 
					      strend(extension, ".java"))
					{
						moveFile("/home/calcifer/Documents/code/unorganized", fullname);
					}
					else if (strend(extension, ".iso") || strend(extension, ".vbox") || strend(extension, ".vdi"))
					{
						moveFile("/home/calcifer/Documents/disc-images", fullname);
					}
					else if (strend(extension, ".desktop"))
					{
						moveFile("/home/calcifer/Desktop", fullname);
					}
				}
				free(fullname);
			}
			
			i += sizeof(struct inotify_event) + event->len;
		}
	}
	
	EXIT:
	inotify_rm_watch(inotify_fd, watch_desc);
	close(inotify_fd);
	return return_status;
}

/**
 * Determine the size of the file specified by the name as found by fopen
 * @param  filename the name of the file, follows the same guidelines as fopen
 * @return          -1 if the file's not found, the byte count otherwise
 */
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

/**
 * moves file from parameter 1 to the directory in parameter 2
 * Performs basic error checking
 * @param destination a directory name
 * @param from        a file name, can be a directory
 */
void moveFile(char* destination, char* from) // TODO: try to reduce the amount of dynamic memory allocation
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
		printf("%s exists\n", safe_name);
		free(safe_name);
		safe_len = name_len + getDigitCount(m) + 3;
		safe_name = malloc(safe_len);
		snprintf(safe_name, safe_len, "%s (%d)", end_name, m);
	}
	free(end_name);
	end_name = safe_name;
	name_len = safe_len; // shouldn't be necessary, but it keeps a safe state
	
	printf("Rename \"%s\" to \"%s\"\n", from, end_name);
	/*
	if (rename(from, end_name) < 0)
	{
		perror("Unable to move file to destination");
	}
	*/
	free(end_name);
}

/**
 * finds the last instance of a character in a string
 * @param  str  the string to search
 * @param  find the character to find
 * @return      a pointer to the result
 */
char* strrfind(char* str, char find)
{
	size_t len = strlen(str);
	for (int x = 1; x <= len; ++x)
	{
		if (str[len - x] == find)
		{
			return str + len - x;
		}
	}
	return str + len;
}

/**
 * check whether string 1 ends with string 2
 * @param  str the string to check against.  should be as long or longer than the end parameter
 * @param  end the string that should exist at the end of the first parameter
 * @return     true if parameter end is found at the end of parameter str
 */
bool strend(char* str, char* end)
{
	size_t end_len = strlen(end);
	size_t str_len = strlen(str);
	if (end_len > str_len)
	{
		return false;
	}
	
	return 0 == strncmp(str + str_len - end_len, end, end_len);
}

/**
 * prints a basic string representation of an inotify event to stdout
 * @param event the event to print
 */
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

/**
 * get the string representation of the rightmost set event bit found in the mask
 * @param  mask the event mask
 * @return      an immutable string, "" if no event is found
 */
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

size_t getDigitCount(int num)
{
	if (num == 0)
	{
		return 1;
	}
	
	size_t count = 0;
	while (num != 0)
	{
		++count;
		num /= 10;
	}
	return count;
}
