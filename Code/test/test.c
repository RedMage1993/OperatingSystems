#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main()
{
    pid_t pid;
    int com[2];
    char *command[10];
    int status;

    pipe(com);
    dup(0); // stdin at 5
    dup(1); // stdout at 6

    command[0] = (char*) "ls";
    command[1] = NULL;

    close(1); // make next command write to our pipe
    dup(com[1]);

    pid = fork();
    if (pid == 0)
    {
	execvp(command[0], command);
	return 0; // error
    }
    else
	waitpid(pid, &status, WUNTRACED);

    close(1); // restore stdout so we can see output
    dup(6);

    close(0);
    dup(com[0]); // make next command read from our pipe

    command[0] = (char*) "grep";
    command[1] = (char*) "c";
    command[2] = NULL;

    close(com[1]);

    pid = fork();
    if (pid == 0)
    {
	execvp(command[0], command);
	return 0; // error
    }
    else
    {
	waitpid(pid, &status, WUNTRACED);
	close(com[0]);
    }

    close(0); // restore stdin
    dup(5);

    return 0;
}
