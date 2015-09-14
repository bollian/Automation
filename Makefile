bin_path = bin/
CC = clang

all: $(bin_path)
	$(CC) organizer.c -std=c99 -o $(bin_path)/organizer

$(bin_path):
	mkdir $(bin_path)