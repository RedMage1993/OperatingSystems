//***************************************************
// Author: Fritz Ammon
// Date: 2 September 2014
// Program: Lecture 1 C Functions
// Description: Testing Linux-exclusive C functions.
// These are non-portable, & must be compiled with
// gcc (or g++).
//
// TODO:
//  - (9/2/14) Emacs has been customized as far as
//    the brace indentation. Now learn to use the
//    the "make" command along with "makefile."
//
// CHANGES:
//  - (9/3/14) Added example of reading in entire
//    file (and allocating memory for a buffer).
//***************************************************

//*********************
#include <stdio.h>
// Included for:
// puts (or printf)
// perror - If internal function fails, can use this.
// fgets - Reads num-1 chars, stops EOF or \n but includes it.
// fopen - r, w, b(inary mode, which affects newlines, fseek, etc.).
// fseek, ftell, rewind
// fclose
//*********************
#include <stdlib.h>
// calloc
// free
//*********************
#include <string.h>
// strtok - Subsequent calls expect NULL. Beginning is first char not in delimiter, end is at delimiter.
//          End of token is automatically replaced with a NULL.
//*********************
#include <unistd.h>
// fork - Can have WNOHANG or WUNTRACED. WUNTRACED ensures children complete.
// execvp - Replaces process space with new one, remember commands are programs.
// open, close, dup - UNIX (non c standard)
// pipe - send data to other process using file descriptor. [0] is input, [1] is output.
//*********************
#include <sys/types.h>
#include <sys/wait.h>
// waitpid
//*********************
#include <signal.h>
// signal - Hook a signal and assign a callback function.
//          SIGTSTP, SIGCHLD, +SIG_IGN, +SIG_DFL
//*********************
#include <termios.h>
// struct termios - Allows us to control timing for things such as function keys.
//                  
//*********************

#define ischild(p) (p == 0) // If child process, pid is 0.

pid_t pid;

void chStop(int n)
{
  printf("Child process %d has stopped with code %d.\n", pid, n);
}

int main(int argv, char **argc)
{
    FILE *fd = NULL;
    char *buff = NULL;
    long len = 0;
    char *token = NULL;
    int status = 0;
    char *file[3] = {NULL};
    int com[2];

    signal(SIGCHLD, chStop); // Install hook.

    pipe(com); // This must be done before the fork.
    pid = fork();

    if (argv < 2)
    {
	puts("A file name has not been provided.");
	return 0;
    }

    fd = fopen(argc[1], "r"); // Open file in read mode.
    if (fd == NULL)
    {
	perror("An error occurred: "); // Display DNE.
	return 0;
    }

    // Get file size in bytes.
    fseek(fd, 0, SEEK_END);
    len = ftell(fd);
    rewind(fd);

    // Allocate for all the bytes plus NULL terminator.
    buff = (char*) calloc(len + 1, sizeof(char));
    *(buff + len) = '\0'; // Set last byte to NULL.
    
    fgets(buff, len, fd); // Read in bytes from stream.
    fclose(fd);

    if (!ischild(pid))
    {
	// Without waitpid, these blocks execute in uncontrolled order.
	waitpid(pid, &status, WUNTRACED);

	printf("%s\n", buff);

	// Show example of tokenizing buffer.
	token = strtok(buff, ",");
	while (token != NULL)
	{
	    puts(token);
	    token = strtok(NULL, ",");
	}

	free(buff);

	close(com[1]); // Will not use output from parent POV.
	buff = (char*) calloc(16, 1);
	*(buff + 15) = '\0';
	read(com[0], buff, 15);
	puts(buff);
	free(buff);

	file[0] = (char*) "echo"; // Cast is fine--conversion is simply not handled correctly.
	file[1] = (char*) "\"Maybe not (execvp).\"";

	execvp(file[0], file);
    }
    else
    {
	// This will execute first (child) with waitpid WUNTRACED.
	token = strtok(buff, ".");
	token = strtok(NULL, ".");
	printf("%s", token + 1);
	puts(" (child)");

	free(buff); // We free here knowing execvp kills process.

	close(com[0]); // We will not use input from child POV.
	write(com[1], "The pipe works.", 15);
    }

    return 0;
}
