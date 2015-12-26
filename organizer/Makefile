proj_path := $(shell dirname $(abspath $(MAKEFILE_LIST)))
check_dir = tmp/
bin_dir = bin/
bin_path = $(proj_path)/$(bin_dir)

CC := clang
cflags := -std=c99 -Wall -D_POSIX_C_SOURCE=2 -D_GNU_SOURCE -I$(shell dirname $(proj_path)) $(shell pkg-config --cflags libnotify gtk+-3.0)
clibs :=$(shell pkg-config --libs libnotify gtk+-3.0)
#clibs := -lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0
#cinclude := -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libpng12
#cflags := -std=c99 -Wall -I$(shell dirname $(proj_path)) -pthread -D_POSIX_SOURCE

all: $(bin_dir) organizer
	$(CC) $(bin_path)/*.o -o $(bin_path)/organizer $(cflags) $(clibs)

check: bin_dir = $(check_dir)
check: $(check_dir) organizer
	rm -rf $(check_dir)

organizer: $(bin_dir) organizer.c files common notification
	$(CC) organizer.c -o $(bin_path)/organizer.o -c $(cflags)

notification: $(bin_dir) notification.c notification.h
	$(CC) notification.c -o $(bin_path)/notification.o -c $(cflags)

files: $(bin_dir) files.c files.h notification
	$(CC) files.c -o $(bin_path)/files.o -c $(cflags)

common: $(bin_dir) common.c common.h
	$(CC) common.c -o $(bin_path)/common.o -c $(cflags)

$(bin_dir):
	if [ -e $(proj_path)/$(bin_dir) ]; then\
		if [ ! -d $(proj_path)/$(bin_dir) ]; then\
			rm $(proj_path)/$(bin_dir);\
			mkdir $(proj_path)/$(bin_dir);\
		fi;\
	else\
		mkdir $(proj_path)/$(bin_dir);\
	fi

$(check_dir):
	if [ -e $(proj_path)/$(check_dir) ]; then\
		if [ ! -d $(proj_path)/$(check_dir) ]; then\
			rm $(proj_path)/$(check_dir);\
			mkdir $(proj_path)/$(check_dir);\
		fi;\
	else\
		mkdir $(proj_path)/$(check_dir);\
	fi
