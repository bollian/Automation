#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>

int main(int argc, char** argv)
{
	// type sizes
	printf("sizeof(int) %u\n", (unsigned int)sizeof(int));
	printf("sizeof(uint32_t) %u\n", (unsigned int)sizeof(uint32_t));
	printf("sizeof(inotify_event) %u\n", (unsigned int)sizeof(struct inotify_event));
	
	// inotify mask values
	printf("IN_ACCESS: %d\n", IN_ACCESS);
	printf("IN_MODIFY: %d\n", IN_MODIFY);
	printf("IN_ATTRIB: %d\n", IN_ATTRIB);
	printf("IN_CLOSE_WRITE: %d\n", IN_CLOSE_WRITE);
	printf("IN_CLOSE_NOWRITE: %d\n", IN_CLOSE_NOWRITE);
	printf("IN_CREATE: %d\n", IN_CREATE);
	printf("IN_OPEN: %d\n", IN_OPEN);
	printf("IN_MOVED_FROM: %d\n", IN_MOVED_FROM);
	printf("IN_MOVED_TO: %d\n", IN_MOVED_TO);
	printf("IN_DELETE: %d\n", IN_DELETE);
	printf("IN_DELETE_SELF: %d\n", IN_DELETE_SELF);
	printf("IN_MOVE_SELF: %d\n", IN_MOVE_SELF);
	
	// test the access function
	char* name = "/some_file.txt (";
	printf("%s exists %d\n", name, access(name, F_OK));
	printf("%s is readable %d\n", name, access(name, R_OK));
	printf("%s is writable %d\n", name, access(name, W_OK));
	printf("%s is executable %d\n", name, access(name, X_OK));
	
	return 0;
}
