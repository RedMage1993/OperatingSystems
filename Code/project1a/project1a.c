//********************************************
// Author: Fritz Ammon
// Date: 3 September 2014
// Program: Project 1 A
// Description: Prints stuff for child process
// and demonstrates knowledge of basic Unix
// commands.
// CS 370
//********************************************

#include <stdio.h> // For printf
#include <unistd.h> // For execvp, getpid
#include <sys/types.h> // getpid, waitpid
#include <sys/wait.h>

int main() // No arguments.
{
    pid_t pid;
    int status = 0;
    char *command[3] = {NULL};

    pid = fork();

    if (pid == 0)
    {
	// Can create pipe to pass pid from parent over here
	// or can just simply call getpid.
	// OR you can call  both prints from the parent.
	printf("Child[%d] is executing the tail command.\n", getpid());

	command[0] = (char*) "grep";
	command[1] = (char*) "c";
	command[2] = NULL;

	execvp(command[0], command);
    }
    else
    {
     	waitpid(pid, &status, WUNTRACED); // Child should finish first.

	printf("Parent[%d] has finished waiting for its child\
 process to complete.\n", getpid());
    }

    return 0;
}
