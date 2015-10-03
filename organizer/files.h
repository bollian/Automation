#ifndef ORGANIZER_FILES_H_
#define ORGANIZER_FILES_H_

#include <sys/inotify.h>

/////////////////////
// File management //
/////////////////////

/**
 * intercepts and handles events
 * @param fd inotify file descriptor
 * @return value indicating an error code, 0 if successful
 */
int process(int fd, char* watch_dir);

/**
 * Determine the size of the file specified by the name as found by fopen
 * @param  filename the name of the file, follows the same guidelines as fopen
 * @return          -1 if the file's not found, the byte count otherwise
 */
int getFileSize(char* filename);

/**
 * moves file from parameter 1 to the directory in parameter 2
 * Performs basic error checking
 * @param destination a directory name
 * @param from        a file name, can be a directory
 */
void moveFile(char* destination, char* from);

///////////////////
// Event helpers //
///////////////////

typedef struct
{
	int inotify_fd;
	int watch_desc;
} InotifyDetail;

/**
 * creates an inotify watch on the directory specified
 * @param  dirname the full directory path
 * @return         the inotify file descriptor and watch descriptor.  both will be negative on fail
 */
InotifyDetail initInotify(char* dirname);

/**
 * cleans up all inotify resources
 */
void closeInotify();

/**
 * get the string representation of the rightmost set event bit found in the mask
 * @param  mask the event mask
 * @return      an immutable string, "" if no event is found
 */
const char* eventMaskString(uint32_t mask);

/**
 * prints a basic string representation of an inotify event to stdout
 * @param event the event to print
 */
void printEvent(struct inotify_event* event);

#endif // ORGANIZER_FILES_H_
