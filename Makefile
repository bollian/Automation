proj_path = $(shell dirname $(abspath $(MAKEFILE_LIST)))
bin_path = $(proj_path)/bin/

CC = clang
cargs = -std=c99 -I$(proj_path) -pthread -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libpng12 -lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0

all: organizer $(bin_path)
	

organizer: organizer.c organizer.h $(bin_path)
	$(CC) organizer.c -o $(bin_path)/organizer $(cargs)

$(bin_path):
	mkdir $(bin_path)