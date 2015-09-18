proj_path = $(shell dirname $(abspath $(MAKEFILE_LIST)))
bin_path = $(proj_path)/bin/

CC = clang
cargs = -std=c99 -I$(proj_path)

all: organizer $(bin_path)
	

organizer: organizer.c organizer.h $(bin_path)
	$(CC) organizer.c -o $(bin_path)/organizer $(cargs)

$(bin_path):
	mkdir $(bin_path)