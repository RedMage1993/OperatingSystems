//**********************************************
// Author: Fritz Ammon
// Date: 26 November 2014
// Program: CS 370 Project 5
// Dijkstra's K-State Mutual Exclusion
// Description: We create a K ring network that
// communicates via sockets locally.
//**********************************************

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_NAME 255

typedef enum {RIGHT, LEFT} side;
typedef enum {FALSE, TRUE} BOOL;

typedef struct
{
  int descriptor;
  struct sockaddr_un address;
} sock;

void inCS(int id);
BOOL sockMake(int instancedId, side neighbour, sock *s);
BOOL sockClient(sock *s);
int sockServer(sock *s);

// socket names will be appended to this constant name.
// It is followed by an underscore, and the instanceId.
const char SOCKET_NAME[] = "project5a";

int ringSize = 0; // How many nodes are in the ring.
int state = 0;

int main()
{
  int instanceId = 0; // id assigned to instance of program.
  sock right, left; // The sockets sending/receiving states.
  int connectRight = 0, connectLeft = 0; // The nodes which connected to me.
  BOOL done = FALSE;
  int neighbourState = 0;
	
  scanf("%d%d", &instanceId, &ringSize); // get instanceId and ringSize.

  srand(time(0));

  state = rand() % ringSize;
  printf("%d begins with %d\n", instanceId, state);
  
  if (instanceId == 0) // bottom node (0).
  {
    // Socket intended for node 1 to connect to.
    if (!sockMake(0, RIGHT, &right))
      return 0;

    connectRight = sockServer(&right); // Make connection to 1 first.
    if (connectRight == -1)
      return 0;

    // Socket intended for node n-1 to connect to.
    if (!sockMake(0, LEFT, &left))
      return 0;

    connectLeft = sockServer(&left); // Wait for a ringSize - 1.
    if (connectLeft == -1)
      return 0;

    while (!done)
    {
      recv(connectRight, &neighbourState, sizeof(int), 0);
      if (state == neighbourState)
      {
	inCS(instanceId);
	state = (state + 1) % ringSize;
      }

      recv(connectLeft, &neighbourState, sizeof(int), 0);
      if (state == neighbourState)
      {
	inCS(instanceId);
	state = (state + 1) % ringSize;
      }
    }
  }
  else if (instanceId == ringSize - 1) // top node (n - 1).
  {
    // Use address (name) of left node.
    if (!sockMake(instanceId - 1, RIGHT, &left))
      return 0;

    if (!sockClient(&left))
      return 0;

    // Use address (name) of right node.
    if (!sockMake(0, LEFT, &right))
      return 0;

    if (!sockClient(&right))
      return 0;

    while (!done)
    {
      send(left.descriptor, &state, sizeof(int), 0);
      send(right.descriptor, &state, sizeof(int), 0);
    }
  }
  else // middle nodes
  {
    if (!sockMake(instanceId - 1, RIGHT, &left))
      return 0;

    if (!sockClient(&left))
      return 0;

    if (!sockMake(instanceId, RIGHT, &right))
      return 0;

    connectRight = sockServer(&right);
    if (connectRight == -1)
      return 0;

    while (!done)
    {
      recv(connectRight, &neighbourState, sizeof(int), 0);
      if (state != neighbourState)
      {
	inCS(instanceId);
	state = neighbourState;
      }

      send(left.descriptor, &state, sizeof(int), 0);
    }
  }

  return 0;
}

BOOL sockMake(int instanceId, side neighbour, sock *s)
// instanceId is the id of program/node. It's either a server or a client.
// This generates a name based off instancedId and side.
{
  memset(s, 0, sizeof(sock)); // Initialize sock.
	
  // Generate the name for the socket and save it in sock.
  sprintf(s->address.sun_path, "%s_%d%d", SOCKET_NAME, instanceId, neighbour);

  s->address.sun_family = AF_UNIX;
  s->descriptor = socket(s->address.sun_family, SOCK_STREAM, 0);

  if (s->descriptor == -1)
  {
    printf("Error creating socket [%s]\n", s->address.sun_path);
    return FALSE;
  }

  return TRUE;
}

void inCS(int id)
{
  puts("#####################################");
  printf("[%d] I n C r i t i c a l S e c t i o n\n", id);
  puts("#####################################\n");
}

BOOL sockClient(sock *s)
// s is our socket. It should hold the address of a server.
// We connect to the name/address of the socket.
{
  if (connect(s->descriptor, (struct sockaddr *) &s->address,
	      sizeof(s->address)))
    return FALSE;

  return TRUE;
}

int sockServer(sock *s)
// s is the server socket.
// This function creates a server using s and listens for a connection.
{
  int ret = 0;
  int addyLen = 0;

  if (!s)
    return FALSE;

  unlink(s->address.sun_path); // Make sure no socket associated with address.

  ret = bind(s->descriptor, (struct sockaddr *) &s->address,
	     sizeof(s->address));
  if (ret == -1)
  {
    printf("Error binding socket [%s]\n", s->address.sun_path);
    return -1;
  }

  ret = listen(s->descriptor, 1);
  if (ret == -1)
  {
    printf("Error listening to socket [%s]\n", s->address.sun_path);
    return -1;
  }

  addyLen = sizeof(s->address);
  return accept(s->descriptor, (struct sockaddr *) &s->address, &addyLen);
}


