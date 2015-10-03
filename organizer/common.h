#ifndef ORGANIZER_COMMON_H_
#define ORGANIZER_COMMON_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define FILE_NAME_MAX 256
#define BUF_SIZE (1024 * (sizeof(struct inotify_event) + FILE_NAME_MAX))

/////////////
// Pausing //
/////////////

/**
 * change the paused state
 * @param pause true to stop file movement, false to allow
 */
void pauseMovement(bool pause);

/**
 * check the paused state
 * @return true if file movement disallowed, false if allowed
 */
bool isPaused();

/////////////
// Logging //
/////////////

/**
 * checks to see if the file is writable by the current process
 * @param  file open FILE
 * @return      false is file is NULL or nonwritable (user, group, & other)
 */
bool isFileWritable(FILE* file);

/**
 * opens the logging files
 * @param error   error file descriptor, defaults to stderr if NULL or nonwritable
 * @param warning warning file descriptor, defaults to stderr if NULL or if nonwritable
 * @param debug   debug file descriptor, defaults to stdout if NULL or if nonwritable
 */
void setLogFiles(FILE* error, FILE* warning, FILE* debug);

void writeError(char* format, ...);
void writeWarning(char* format, ...);
void writeDebug(char* format, ...);

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

#endif // ORGANIZER_COMMON_H_
