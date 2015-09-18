#include <stdbool.h>
#include <stdarg.h>
#include <sys/inotify.h>
#include <unistd.h>

#define NAME_MAX 256
#define BUF_SIZE (1024 * (sizeof(struct inotify_event) + NAME_MAX))

/////////////
// Logging //
/////////////

void writeError(char* format, ...);
void writeDebug(char* format, ...);

/////////////////////
// File management //
/////////////////////

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

/**
 * intercepts and handles events
 * @param fd inotify file descriptor
 * @return value indicating an error code, 0 if successful
 */
int process(int fd, char* watch_dir);

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

////////////////
// string ops //
////////////////

/**
 * finds the last instance of a character in a string
 * @param  str  the string to search
 * @param  find the character to find
 * @return      a pointer to the result
 */
char* strrfind(char* str, char find);

/**
 * check whether string 1 ends with string 2
 * @param  str the string to check against.  should be as long or longer than the end parameter
 * @param  end the string that should exist at the end of the first parameter
 * @return     true if parameter end is found at the end of parameter str
 */
bool strend(char* str, char* end);

size_t getDigitCount(int num);
