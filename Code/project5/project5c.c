//**********************************************
// Author: Fritz Ammon
// Date: 26 November 2014
// Program: CS 370 Project 5
// PLEASE READ:
// I tried the best I could to follow the PDF
// but there are inconsistencies about the left
// and right neighbors. In text, a process P_i
// gets the state value from the left neighbor
// P_i-1. But in the picture you show P_i
// getting data from P_i+1. Look at P_1.
// Please watch my explanation:
// http://youtu.be/OcAk9aiQXCw
//
// Dijkstra's K-State Mutual Exclusion
// Description: We implement Dijkstra's K-State
// solution for stabilization. This must use a
// UNIdirectional ring.
// Machine i gets lefthand machine state L from
// i + 1. In other words, the left of 1 is 2 and
// we view the ring from the outside.
//**********************************************

//#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_NAME 255

typedef enum {LEFT, RIGHT} side;
typedef enum {FALSE, TRUE} BOOL;

typedef struct
{
  int descriptor;
  struct sockaddr_un address;
} sock;

void printNotice();
BOOL sockMake(int machineIndex, side neighbor, sock *s);
BOOL sockClient(sock *s);
int sockServer(sock *s);

const char SOCKET_NAME[] = "project5a";

int main()
{
    int noMachines = 0;
    int machineIndex = 0;
    int machineState = 0;
    sock server; // Server for machine to obtain lefthand machine state.
    int lefthandDesc = 0; // Descriptor of client socket of lefthand machine.
    int lefthandState = 0;
    sock client; // Client to transmit machine state to lefthand machine.
    sock signal; // Sockets used by 0 and n - 1 for the initial signal for flow of execution.
    int sigDesc = 0; // Desciptor of n - 1.
    BOOL done = FALSE;

    //srand(time(0)); // Otherwise all programs begin with the same random state.

    // Get this machine's index in the ring.
    // Get the total number of machines in the ring.
    scanf("%d%d", &machineIndex, &noMachines);

    machineState = rand() % noMachines; // Pick a state value between 0 and noMachines - 1, inclusive.

    // Understand that the following up to the while loop is only run once by each machine.

    if (machineIndex == 0) // For bottom machine.
	// Here we must create a server for n - 1 (our righthand machine).
    {
	if (!sockMake(0, RIGHT, &signal)) // A neignbor of RIGHT means server expects data from its right.
	    return 0;

	sigDesc = sockServer(&signal);
	if (sigDesc == -1)
	    return 0;
       
	// The only purpose of this connection above was to know when to continue executing.
	// Thus, we can now close the connection as it is no longer needed.
	// We will also close sigDesc from the address space of top machine although it would
	// automatically be closed due its server closing down first (from here).
	close(signal.descriptor);

	if (!sockMake(noMachines - 1, LEFT, &client)) // A neighbor of LEFT means the client is
	    return 0;                                                  // connecting to a server which expects its
	                                                               // data from the left.

	// We loop when we attempt to connect for the moment that 0 is released from waiting for
	// n - 1 to signal and 0 may attempt to connect before n - 1 has made the server.
	while (!sockClient(&client));
    }

    if (machineIndex == noMachines - 1) // For top machine.
	// Here it is essential we connect to 0 before we make server for it. This tells machine 0 that
	// it may accept the connection and begin attempting to connect to the top machine.
    {
	if (!sockMake(0, RIGHT, &signal)) // A neighbor of RIGHT means the client is connecting to a server
	    return 0;                     // which expects its data from the right.

	if (!sockClient(&signal))
	    return 0;

	// As mentioned above, we only wanted to connect to 0 as a signal. We now close it.
	close(signal.descriptor);
    }

    // For all machines.
    // This is the normal flow of execution, where we make a server for our lefthand machine (i + 1) and
    // a client send our state to the righthand machine (i - 1).
    if (!sockMake(machineIndex, LEFT, &server)) // A neighbor of LEFT means the server is expecting data from
	return 0;                               // the left.

    lefthandDesc = sockServer(&server);
    if (lefthandDesc == -1)
	return 0;

    if (machineIndex != 0) // 0 should have already made a client connection to n - 1.
    {
	if (!sockMake((machineIndex - 1) % noMachines, LEFT, &client)) // A neighbor of LEFT means the client is
	    return 0;                                                  // connecting to a server which expects its
	                                                               // data from the left.
	if(!sockClient(&client))
	    return 0;
    }

    while (!done) // This program is actually expected to run forever.
    {
	send(client.descriptor, &machineState, sizeof(int), 0);
	recv(lefthandDesc, &lefthandState, sizeof(int), 0);

	if (machineIndex == 0) // Instructions for bottom machine.
	{
	    if (machineState == lefthandState)
	    {
		// In critical section.
		printNotice();
		//printf("%d got %d\n", 0, lefthandState);
		machineState = (machineState + 1) % noMachines;
	    }
	}
	else // Instructions for other machines.
	{
	    if (machineState != lefthandState)
	    {
		// In critical section.
		printNotice();
		//printf("%d got %d\n", machineIndex, lefthandState);
		machineState = lefthandState;
	    }
	}
    }

    // These will never execute. Only if flag is changed in main while loop.
    close(server.descriptor);
    close(client.descriptor);

    return 0;
}

void printNotice()
// Prints a notification that this machine is in the critical section.
{
    puts("#####################################");
    puts(" I n  C r i t i c a l  S e c t i o n");
    puts("#####################################\n");
}

BOOL sockMake(int machineIndex, side neighbor, sock *s)
// machineIndex is the index of the machine the socket is being made for.
// neighbor for a server is the direction it's expecting data from, for a client it is from the perspective
// of the server, thus it is the same.
// s is where information about the socket will be saved.
// Returns information about the created socket in s.
{
    memset(s, 0, sizeof(sock));
	
    // Generate an address to use for the socket based on the machine index.
    sprintf(s->address.sun_path, "%s_%d%d", SOCKET_NAME, machineIndex, neighbor);

    s->address.sun_family = AF_UNIX;
    s->descriptor = socket(s->address.sun_family, SOCK_STREAM, 0); // Get descriptor for socket.

    if (s->descriptor == -1)
    {
	printf("Error creating socket [%s]\n", s->address.sun_path);
	return FALSE;
    }

    return TRUE;
}

BOOL sockClient(sock *s)
// s holds information about a socket.
// Establishes a connection with the socket associated with the descriptor in s.
{
    if (connect(s->descriptor, (struct sockaddr *) &s->address,
		sizeof(s->address)))
	return FALSE;

    return TRUE;
}

int sockServer(sock *s)
// s holds information about a socket.
// Waits for socket associated with descriptor in s to establish a connection.
{
    int ret = 0;
    int addyLen = 0;

    if (!s)
	return FALSE;

    // Make sure no address bound to descriptor.
    unlink(s->address.sun_path);

    ret = bind(s->descriptor, (struct sockaddr *) &s->address,
	       sizeof(s->address));
    if (ret == -1)
    {
	printf("Error binding socket [%s]\n", s->address.sun_path);
	return -1;
    }

    // Prepare socket associated with descriptor to accept connections.
    ret = listen(s->descriptor, 1);
    if (ret == -1)
    {
	printf("Error listening to socket [%s]\n", s->address.sun_path);
	return -1;
    }

    addyLen = sizeof(s->address); // Function expects a pointer to variable holding size of address.
    return accept(s->descriptor, (struct sockaddr *) &s->address, &addyLen);
}
