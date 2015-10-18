#ifndef ORGANIZER_NOTIFICATION_H_
#define ORGANIZER_NOTIFICATION_H_

#include <stdbool.h>

/**
 * initializes libnotify
 * @return returns true on success
 */
bool initNotification();

void freeNotification();

/**
 * cleans up all resources used by libnotify
 */
void closeNotification();

/**
 * checks to see if button responses are supported by the desktop notification server
 * @return true if button actions are supported, false otherwise
 */
bool notificationActionsAllowed();

/**
 * sends a notification of file organization, giving the options to cancel or pause
 * intended for use while not in the paused state
 * @param to new file name
 * @param from old file name
 */
void sendMovingNotification(char* to, char* from);

/**
 * sends a notification of file organization, giving the option to allow or unpause
 * intended for usage while in the paused state
 * @param to new file name
 * @param from old file name
 */
void sendPausedNotification(char* to, char* from);

#endif // ORGANIZER_NOTIFICATION_H_
