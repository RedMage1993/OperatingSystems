//**********************************************
// Author: Fritz Ammon
// Date: 26 November 2014
// Program: CS 370 Project 5
// PLEASE READ:
// I have slowed down the execution and have
// placed a system("clear") to make it easy to
// see who is in the critical section. Thanks.
// Demo/proof: http://youtu.be/L7IEhDo_Zdw
//
// Programs should be started as described:
// Start machine 0, enter 0 n, machine 1, enter
// 1 n, ..., machine n - 1, enter n - 1 n.
//
// Dijkstra's K-State Mutual Exclusion
// Description: We implement Dijkstra's K-State
// solution for stabilization.
// Machine i gets lefthand machine state L from
// i - 1. In other words, the left of 1 is 0.
//
// Updated with while loop on createClient to
// allow 2 nodes.
//**********************************************

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_NAME 255
#define WAIT_TIME_MICROS 500000

typedef enum {FALSE, TRUE} BOOL;

typedef struct
{
  int descriptor;
  struct sockaddr_un address;
} sock;

void printNotice();
BOOL sockMake(int machineIndex, sock *s);
BOOL sockClient(sock *s);
int sockServer(sock *s);
BOOL createClient(int machineIndex,  sock *s);
int createServer(int machineIndex, sock *s);

const char SOCKET_NAME[] = "project5a";

int main()
{
    int noMachines = 0;
    int machineIndex = 0;
    int machineState = 0;
    sock server; // Server for machine to obtain lefthand machine state.
    int lefthandDesc = 0; // Descriptor of client socket of lefthand machine.
    int lefthandState = 0;
    sock client; // Client to transmit machine state to righthand machine.
    BOOL done = FALSE;

    srand(time(0));

    // Get this machine's index in the ring.
    // Get the total number of machines in the ring.
    scanf("%d%d", &machineIndex, &noMachines);

    machineState = rand() % noMachines; // Pick a state value between 0 and noMachines - 1, inclusive.

    if (machineIndex != noMachines - 1)
	// Request connections in this order for machines 0 to noMachines - 2.
    {
        lefthandDesc = createServer(machineIndex, &server);
	if (lefthandDesc == -1)
	    return 0;

	while (!createClient((machineIndex + 1) % noMachines, &client))
	    usleep(WAIT_TIME_MICROS);
    }
    else
	// This is where we need machine noMachines - 1 to offer to connect to machine 0
	// before requesting lefthand machine to connect. Otherwise, we have a deadlock.
    {
        if (!createClient(0, &client))
            return 0;

	lefthandDesc = createServer(machineIndex, &server);
	if (lefthandDesc == -1)
	    return 0;
    }

    while (!done)
    {
	// Trasmit machine state to righthand machine.
	send(client.descriptor, &machineState, sizeof(int), 0);
	// Obtain lefthand machine state.
	recv(lefthandDesc, &lefthandState, sizeof(int), 0);

	if (machineIndex == 0)
	    // These are the actions to take for the bottom machine, machine 0.
	{
	    if (machineState == lefthandState)
	    {
		//printf("0L = %d\n", lefthandState);
		printNotice();
		machineState = (machineState + 1) % noMachines; // Calculate new machine state.
	    }
	}
	else
	    // These are the actions to take for the other machines.
	{
	    if (machineState != lefthandState)
	    {
		//printf("%dL = %d\n", machineIndex, lefthandState);
		printNotice();
		machineState = lefthandState; // Get new machine state.
	    }
	}
    }

    return 0;
}

void printNotice()
// Prints a notification that this machine is in the critical section.
{
    puts("#####################################");
    puts(" I n  C r i t i c a l  S e c t i o n");
    puts("#####################################");

    usleep(WAIT_TIME_MICROS);

    system("clear"); // Make it easy to see who is in critical section.
}

BOOL sockMake(int machineIndex, sock *s)
// machineIndex is the index of the machine the socket is being made for.
// s is where information about the socket will be saved.
// Returns information about the created socket in s.
{
    memset(s, 0, sizeof(sock));
	
    // Generate an address to use for the socket based on the machine index.
    sprintf(s->address.sun_path, "%s_%d", SOCKET_NAME, machineIndex);

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

BOOL createClient(int machineIndex, sock *s)
{
    // Create the client for the righthand machine.
    if (!sockMake(machineIndex, s))
	return FALSE;

    // Establish a connection with the righthand machine.
    // Use the descriptor of this socket to send data to righthand machine.
    if (!sockClient(s))
	return FALSE;
}

int createServer(int machineIndex, sock *s)
{
    // Create the server for this machine.
    // The lefthand machine will connect to this.
    if (!sockMake(machineIndex, s))
	return -1;

    // Let the lefthand machine connect to this machine's server.
    return sockServer(s);
}
