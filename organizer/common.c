#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <organizer/common.h>

FILE* err_file = NULL;
FILE* warning_file = NULL;
FILE* debug_file = NULL;

bool paused = false;

void pauseMovement(bool pause)
{
	paused = pause;
}

bool isPaused()
{
	return paused;
}

bool isFileWritable(FILE* file)
{
	if (file)
	{
		struct stat file_stat;
		fstat(fileno(file), &file_stat);
		
		if (file_stat.st_mode & S_IXOTH) // i(s) (e)x(ecutable) to oth(ers)
		{
			return true;
		}
		else if (getgid() == file_stat.st_gid && (file_stat.st_mode & S_IXGRP)) // to gr(ou)p
		{
			return true;
		}
		else if (getuid() == file_stat.st_uid && (file_stat.st_mode & S_IXUSR)) // to us(e)r
		{
			return true;
		}
	}
	
	return false;
}

void setLogFiles(FILE* error, FILE* warning, FILE* debug)
{
	if (error && isFileWritable(error))
	{
		err_file = error;
	}
	else
	{
		err_file = stderr;
	}
	
	if (warning && isFileWritable(warning))
	{
		warning_file = warning;
	}
	else
	{
		warning_file = stderr;
	}
	
	if (debug && isFileWritable(debug))
	{
		debug_file = debug;
	}
	else
	{
		debug_file = stdout;
	}
}

void writeError(char* format, ...)
{
	char buffer[1024];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf(buffer, 1024, format, arg_list);
	va_end(arg_list);

	if (err_file)
	{
		fprintf(err_file, "Error: %s; %s\n", buffer, strerror(errno));
	}
	else
	{
		fprintf(stderr, "Failed to write error to error file \"%s\"\n", buffer);
	}
}

void writeWarning(char* format, ...)
{
	char buffer[1024];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf(buffer, 1024, format, arg_list);
	va_end(arg_list);

	if (warning_file)
	{
		fprintf(warning_file, "Warning: %s\n", buffer);
	}
	else
	{
		printf("Warning: %s\n", buffer);
	}
}

void writeDebug(char* format, ...)
{
	char buffer[1024];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf(buffer, 1024, format, arg_list);
	va_end(arg_list);

	if (debug_file)
	{
		fprintf(debug_file, "Debug: %s\n", buffer);
	}
	else
	{
		writeWarning("Failed to write message to debug file \"%s\"", buffer);
	}
}

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
