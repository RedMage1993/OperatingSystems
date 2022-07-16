//******************************************
// Author: Fritz Ammon
// Date: 7 September 2014
// Program: project2b
// Description: We will recreate a terminal
// using UNIX commands and functions.
//******************************************

// Features
//---------
//
// Prompt
//  - 9/7: Display directory and wait for input
//
// History
//  - 9/7: Store lines in a linked list
//  - 9/8: Display previous line on UP/DOWN
//  - 9/9: Allow modifications in history and reset upon ENTER
//
// Commands
//  - 9/10: Accept commands, cd, exit, and pipe operator

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum {FALSE, TRUE} BOOL;

typedef struct history
{
    struct history *prev;
    char *line;
    unsigned int len;
} hisType;

// 3-byte key codes stored in little-endian.
// Represented in hex.
#define LEFT  0x00445B1B
#define RIGHT 0x00435B1B
#define UP    0x00415B1B
#define DOWN  0x00425B1B

// Single byte key codes.
#define BACKSPACE 0x08
#define DELETE    0x7F

// To use when casting from memory to 3-byte.
#define arrow(h) (0x00FFFFFF & h)

int error(const char *w, int c);
void customInput(BOOL enable);
void pcwd();
void prompt(char *line, int *lineSize, hisType *h);
void removeChar(int c);
void addHis(hisType **h, char *line, int lineSize);
void delHis(hisType *h);
void copyHis(hisType **d, hisType *h);
hisType * getHis(hisType *h, int l);

const unsigned int MAX_BUFFER = 256;
const unsigned int MAX_HIST = 10;
const unsigned int MAX_COMMAND = 10;

int hisLen = -1; // Important for addHis and prompt (UP/DOWN).
hisType working;

const int STDIN  = 0;
const int SCREEN = 1; // stdout

int main()
{
    hisType *his = NULL; // Important to initialize!
    char *line; // We'll say this is the final inputted line to parse.
    int size = 0; // Of line inputted.
    BOOL done = FALSE;  // When exit is found.
    int com[2];    // Pipe in and out.
    char **commands;
    char **arguments;
    int cmd, argc; // Number of commands, arguments (for indexes).
    int iCmd = 0;  // Index for iterating commands.
    pid_t pid = 0;
    int status;
    char *exitPrompt = (char*) "Are you sure you want to exit? (Y/n) ";
    int response;

    line = (char*) malloc(MAX_BUFFER);
    commands = (char**) malloc(MAX_COMMAND * MAX_BUFFER);
    arguments = (char**) malloc(MAX_COMMAND * MAX_BUFFER);

    customInput(TRUE);
    
    do
    {
	prompt(line, &size, his);
	addHis(&his, line, size); // Save new line.

	// Check for single commands.
	if (!strcmp(line, "exit"))
	{
	    write(SCREEN, exitPrompt, strlen(exitPrompt));
	    response = getchar();

	    if (response != 'n')
		done = TRUE;
	
	    write(SCREEN, "\n", 1);

	    continue;
	}

	// Tokenize by pipe operator.
	cmd = 0;
	*commands = strtok(line, "|"); // Will fill line with NULLs FYI.
	while (*(commands + cmd) != NULL && cmd < MAX_COMMAND)
	{
	    cmd++;

	    *(commands + cmd) = strtok(NULL, "|");
	}

	memset(arguments, 0, MAX_COMMAND * MAX_BUFFER); // Just to get rid of ptrs.

	if (cmd > 1)
	{
	    // Initializes fd's for swapping. Need this for every prompt.
	    pipe(com); // 3 & 4 now in use.
	    dup(0);    // stdin at 5
	    dup(1);    // stdout at 6
	}

	// For every command, seperate arguments out.
	for (iCmd = 0; iCmd < cmd; iCmd++)
	{   
	    argc = 0;
	    *arguments = strtok(*(commands + iCmd), " ");
	    while (*(arguments + argc) != NULL && argc < MAX_COMMAND)
	    {
		 argc++;

		 *(arguments + argc) = strtok(NULL, " ");
	    }

	    if (cmd > 1)
	    {
	       	if (iCmd == 0) // Begin writing to com[1]
		{
		    close(1);
		    dup(com[1]);
		}
		else if (iCmd == cmd - 1)
		{
		    // Restore output so it prints to screen
		    close(1);
		    dup(6);

		    // Set input from com[0]
		    close(0);
		    dup(com[0]); // Restore once child returns

		    close(com[1]); // Need to close for EOF !
		}
	    }
	    else if (cmd == 1)
	    {
		if (!strcmp(*arguments, "cd"))
		{
		    if (chdir(*(arguments + 1)) == -1)
			error("Could not follow path", 0);
		    
		    continue;
		}
	    }

	    pid = fork();
	    if (pid == 0)
	    {
	     	execvp(*arguments, arguments);
		return error("The command failed to execute", 1);
	    }
	    else
	    {
		waitpid(pid, &status, WUNTRACED);

		if (cmd > 1 && iCmd == cmd - 1)
		{
		    close(0);
		    dup(5); // Restore stdin to 0.

		    close(com[0]); // done with input
		}
	    }
	}
    } while (!done);

    customInput(FALSE);

    delHis(his);
    free(arguments);
    free(commands);
    free(line);

    return 0;
}

int error(const char *w, int c)
{
    perror(w);

    return c;
}

void customInput(BOOL enable)
// enable tells whether to configure our own input settings.
// Enables or disables our input configurations.
{
    static struct termios orig;
    static BOOL enabled = FALSE;
    struct termios new;

    if (enable && !enabled)
    {
	tcgetattr(0, &orig); // Will be saved after function return.

	new = orig;

	new.c_lflag &= ~(ICANON | ECHO);
	new.c_cc[VMIN] = 1;  // At least a newline character.
	new.c_cc[VTIME] = 2; // How long to wait (VTIME * 100ms) before returning.

	tcsetattr(0, TCSANOW, &new);

	enabled = TRUE; // Needed in order for else to run!
    }
    else if (!enable && enabled)
    {
	tcsetattr(0, TCSANOW, &orig);
    }
}

void pcwd()
{
    char *cwd = NULL;
   
    cwd = (char*) malloc(MAX_BUFFER);

    memset(cwd, 0, MAX_BUFFER);

    getcwd(cwd, MAX_BUFFER);

    write(SCREEN, cwd, MAX_BUFFER);
    write(SCREEN, "> ", 2);

    free(cwd);
}

void prompt(char *line, int *lineSize, hisType *h)
{
    char *buffer;
    unsigned int size = 0; // Will be used for size of working buffer.
    int *t; // Probably not necessary, but for casting buffer to 4 bytes.
    unsigned int c; // For loop iterator.
    BOOL done = FALSE;
    int currHisLine = -1; // -1 means user is typing, 0 is first saved line, etc.
    hisType *hisLine = NULL;
    hisType *modify = NULL;

    copyHis(&modify, h);

    memset(line, 0, MAX_BUFFER);

    buffer = (char*) malloc(MAX_BUFFER);

    working.line = (char*) malloc(MAX_BUFFER);
    working.len = 0;

    memset(working.line, 0, MAX_BUFFER);

    (*lineSize) = 0;

    pcwd();

    do
    {
	memset(buffer, 0, MAX_BUFFER);
	read(0, buffer, MAX_BUFFER - 1); // Reads up to 255 (doesn't set NULL).

	// A possible problem with this is that our special keys
	// may be at the end of the buffer, split between two calls
	// to read. Thus, we miss the 3-byte characters. However,
	// this would only occur if the user is able to fill up the
	// buffer with enough bytes under 200ms.

	size = strlen(buffer);
	for (c = 0; c < size; c++)
	{
	    // Check if we need to remove character.
	    switch (*(buffer + c))
	    {
	    case BACKSPACE:
	    case DELETE:
		// Print \b and set previous index char to NULL.
		if (currHisLine == -1)
		{
		    if (working.len > 0)
		    {
			removeChar(1);
			*(working.line + --working.len) = '\0';
		    }
		}
		else
		{
		    if (hisLine->len > 0)
		    {
			removeChar(1);
			*(hisLine->line + --hisLine->len) = '\0';
		    }
		}

		continue;
	    }

	    // Check if any of the arrows.
	    switch (arrow(*((int*) (buffer + c))))
	    {
	    case LEFT:
	    case RIGHT:
		c += 2; // Ignore next two bytes.
		break;
	    case UP:
		c += 2;

	        // Move back history.
		if (currHisLine < hisLen)
		{
		    // Remove current line len.
		    if (currHisLine == -1)
			removeChar(working.len);
		    else
			removeChar(hisLine->len);

		    hisLine = getHis(modify, currHisLine + 1);

		    if (hisLine)
		    {
		 	write(SCREEN, hisLine->line, hisLine->len);
			currHisLine++;
		    }
		}

		break;
	    case DOWN:
		c += 2;

		// Move forward history.
		if (currHisLine > -1)
		{
		    removeChar(hisLine->len);

		    if (currHisLine - 1 == -1)
		    {
			write(SCREEN, working.line, working.len);
			currHisLine--;
		    }
		    else
		    {
			hisLine = getHis(modify, currHisLine - 1);

			if (hisLine)
			{
			    write(SCREEN, hisLine->line, hisLine->len);
			    currHisLine--;
			}
		    }
		}

		break;
	    default:
		write(SCREEN, buffer + c, 1); // Echo character.

		// End of line found.
		if (*(buffer + c) == '\n')
		{
		    done = TRUE;
		    break;
		}

		// Keep saved in current command.
		if (currHisLine == -1)
		{
		    if (working.len < MAX_BUFFER) // May want to exit on else.
			*(working.line + working.len++) = *(buffer + c);
		}
		else
		{
		    if (hisLine->len < MAX_BUFFER)
			*(hisLine->line + hisLine->len++) = *(buffer + c);
		}
     
		break;
	    }	   
	}
     } while (done == FALSE);

    if (currHisLine == -1)
    {
	strcpy(line, working.line); // Return final line of input.
	(*lineSize) = working.len;
    }
    else
    {
	strcpy(line, hisLine->line);
	(*lineSize) = hisLine->len;
    }

    free(working.line);
    free(buffer);

    delHis(modify);
}

void removeChar(int c)
// c is the number of backspaces (and whitespace) to print.
// Prints out "\b \b" c times.
{
    int i;

    for (i = 0; i < c; i++)
    {
	// We need to go back and forward and back.
	write(SCREEN, "\b \b", 3);
    }
}

void addHis(hisType **h, char *line, int lineSize)
// h should be the head of the linked list (for history).
// This will either start the history list, add a new line,
// or loop around (destroy tail, set new tail prev to NULL, and
// create a new head).
{
    hisType *temp;
    hisType *tail, *tailnext = 0;

    if ((*h) == NULL) // Hasn't been initialized yet.
    {
	(*h) = (hisType*) malloc(sizeof(hisType));
	(*h)->prev = NULL; // Will end up being last line until destroyed.
	(*h)->line = (char*) malloc(MAX_BUFFER);

	memset((*h)->line, 0, MAX_BUFFER); // Not necessary, only safety.

	strcpy((*h)->line, line); // Save into spot.
	(*h)->len = lineSize;

	hisLen++;
    }
    else
    {
       	// Make sure we haven't reached MAX_HIST.
	if (hisLen < MAX_HIST - 1) // -1 is for our temporary line index.
	{
	    hisLen++;
	}
	else
	{
	    // We need to start looping around.
	    // First look for tail and keep it's next to make new tail.
	    tail = (*h);

	    // Find tail and tailnext.
	    while (tail->prev != NULL)
	    {
		tailnext = tail;
		tail = tail->prev;
	    }

	    free(tail->line);
	    free(tail);

	    tailnext->prev = NULL;
	}

	// We're good to make new line.
	temp = (hisType*) malloc(sizeof(hisType));
	temp->prev = (*h);
	temp->line = (char*) malloc(MAX_BUFFER);

	memset(temp->line, 0, MAX_BUFFER); // Not neccessary ^.

	strcpy(temp->line, line); // Save into spot.
	temp->len = lineSize;

	(*h) = temp; // Give back new head node.
    }
}

void delHis(hisType *h)
// h is the head of history list.
// Delets any allocated line space and the individual nodes.
{
    hisType *temp;

    while (h)
    {
	free(h->line);
	temp = h->prev;
	free(h);
	h = temp;
    }
}

void copyHis(hisType **d, hisType *h)
// d is destination
// h is master history list (source)
// Will do a deep copy of h into d. d should be
// head of copied list.
{
    hisType *temp, *newh = NULL;

    (*d) = NULL;

    if (h)
    {
	// Start with the head.
	(*d) = (hisType*) malloc(sizeof(hisType));
	(*d)->line = (char*) malloc(MAX_BUFFER);
	memset((*d)->line, 0, MAX_BUFFER); // Necessary because strcpy won't init all.
	strcpy((*d)->line, h->line);
	(*d)->len = h->len;
	(*d)->prev = NULL;

	temp = (*d);

	// Copy all other nodes.
	while (h->prev)
	{
	    h = h->prev;
	    newh = (hisType*) malloc(sizeof(hisType));
	    newh->line = (char*) malloc(MAX_BUFFER);
	    memset(newh->line, 0, MAX_BUFFER);
	    strcpy(newh->line, h->line);
	    newh->len = h->len;
	    newh->prev = NULL;
	    temp->prev = newh;
	    temp = newh;
	}
    }
}

hisType * getHis(hisType *h, int l)
// h is the head of the history list.
// l is the line we're looking for (0 is head).
{
    while (l > 0 && h) // Can do l-- here but naw.
    {
	h = h->prev;
	l--;
    }

    if (l > 0) // Cannot find.
	return NULL;
    else
	return h;
}
