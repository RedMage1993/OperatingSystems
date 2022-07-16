//********************************************
// Author: Fritz Ammon
// Date: 28 October 2014
// Program: Project 4 | CS 370
// Description: Demonstrates knowledge of
// semaphores as solution for race condition,
// synchronization, and the implementation of
// threads for multiprogramming.
//********************************************

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

typedef enum {FALSE, TRUE} BOOL;

typedef unsigned int uint;

typedef struct node
{
	// Assuming first node:
	uint uId; // tempUId.
	struct node *next; // oneHopUId, twoHopUId, etc.
} uIdNode;

typedef struct
{
	BOOL active; // False => relay.
	uint uId, tempUId;
	uIdNode *channel; // Queue with uID's.
	sem_t sync, crit;
} procNode;

typedef struct
{
	procNode *proc;
	procNode *nextProc;
} threadArg;

int pError(const char *szError);
uint init(procNode **procs);
void release(procNode **procs, uint *size);
uint read(uIdNode **channel, sem_t *sync, sem_t *crit);
void write(uIdNode **channel, sem_t *sync, sem_t *crit, uint uId);
void * phase(void *arg);

int main()
{
	procNode *procs = NULL;
	uint size = 0; // Number of nodes.
	uint iterator;
	pthread_t *threadIds = NULL;
	threadArg *arg = NULL;

	size = init(&procs); // Load process info from input.
	if (size == 0)
		return pError("No nodes added");

	// Space to hold thread Id's.
	threadIds = (pthread_t*) malloc(sizeof(pthread_t) * size);
	if (!threadIds)
		return pError("Bad malloc from main");

	// Initialize semaphores for each procNode first.
	for (iterator = 0; iterator < size; iterator++)
	{
		if (sem_init(&procs[iterator].sync, 0, 0)) // 0 because data not initially ready.
			return pError("Bad sem_init from main");
		if (sem_init(&procs[iterator].crit, 0, 1)) // 1 because code seg initially ready.
			return pError("Bad sem_init from main");
	}

	// Create threads for each procNode.
	for (iterator = 0; iterator < size; iterator++)
	{
		// Allocate arguments here. Delete after join returns.
		arg = (threadArg*) malloc(sizeof(threadArg));
		arg->proc = &procs[iterator];
		arg->nextProc = &(procs[(iterator + 1) % size]); // Mod to cycle.

		if (pthread_create(&threadIds[iterator], NULL,
				(void *) phase, (void *) arg)) // We pass a procNode ptr.
			return pError("Bad pthread_create from main");
	}

	// Wait for all of threads to complete.
	for (iterator = 0; iterator < size; iterator++)
	{
		if (pthread_join(threadIds[iterator], (void **) &arg))
			return pError("Bad pthread_join from main");

		free(arg); // Delete thread argument.
	}

	// Destroy the semaphores.
	for (iterator = 0; iterator < size; iterator++)
	{
		if (sem_destroy(&procs[iterator].crit))
			return pError("Bad sem_destroy from main");
		if (sem_destroy(&procs[iterator].sync))
			return pError("Bad sem_destroy from main");
	}

	free(threadIds);

	release(&procs, &size); // Release procs and channel queue's.

	return 0;
}

int pError(const char *szError)
// Used to conveniently print error message and return exit code.
{
	printf("%s", szError);
	return 0;
}

uint init(procNode **procs)
// Initializes each node/proc with starting values (from input).
//
// procs is an array of procNode.
//
// Return the number of procs in the array.
{
	int ret;
	uint size = 0; // Number of procs in the array.
	uint p; // Iterator.

	ret = scanf("%u", &size);
	if (ret == EOF || ret < 1) // End-of-file or scan failed to fill size.
		return (uint) pError("Bad scanf from init");

	*procs = (procNode*) malloc(sizeof(procNode) * size); // Create array with size procNodes.
	if (!*procs)
		return (uint) pError("Bad malloc from init");

	for (p = 0; p < size; p++)
	{
		ret = scanf("%u", &(*procs)[p].uId); // Read in the uId for proc[p].
		if (ret == EOF || ret < 1)
			return (uint) pError("Bad scanf from init");
		
		(*procs)[p].active = TRUE;
		(*procs)[p].tempUId = (*procs)[p].uId;
		(*procs)[p].channel = NULL;
	}

	return size;
}

void release(procNode **procs, uint *size)
// Frees allocated memory of channel and then the procNode array.
//
// procs is an array of pointers to procNodes.
{
	uint n = *size;
	uint p; // Iterator.

	if (*procs)
	{
		for (p = 0; p < n; p++)
		{
			if ((*procs)[p].channel)
			{
				free((*procs)[p].channel);
				(*procs)[p].channel = NULL;
			}
		}

		free(*procs);
		procs = NULL;

		*size = 0;
	}
}

uint read(uIdNode **channel, sem_t *sync, sem_t *crit)
{
	uint uId;
	uIdNode *next;

	// A sem_wait decrements/locks the semaphore.
	// If semaphore is at 0, then block occurs until increment/unlock by sem_post.
	if (sem_wait(sync))
		return (uint) pError("Bad sem_wait from read");
	if (sem_wait(crit))
		return (uint) pError("Bad sem_wait from read");

	// Critical section begin.
	uId = (*channel)->uId;

	next = (*channel)->next; // Backup channel's next before deleting.

	free(*channel);

	*channel = next; // Update queue's head.
	// Critical section end.

	if (sem_post(crit))
		return (uint) pError("Bad sem_post from read");

	return uId;
}

void write(uIdNode **channel, sem_t *sync, sem_t *crit, uint uId)
{
	uIdNode *new, *iterator;

	if (sem_wait(crit))
	{
			pError("Bad sem_wait from write");
			return;
	}

	// Critical section begin.
	new = (uIdNode*) malloc(sizeof(uIdNode));

	if (!new)
	{
		pError("Bad malloc from write");
		return;
	}

	new->uId = uId;
	new->next = NULL;

	if (!(*channel))
		*channel = new; // Has no head (at least tempUId has been read).
	else
	{
		iterator = *channel;

		while (iterator->next)
			iterator = iterator->next; // Iterate to last node.

		iterator->next = new; // Make last node point to new last node.
	}
	// Critical section end.

	if (sem_post(crit))
	{
		pError("Bad sem_post from write");
		return;
	}

	if (sem_post(sync)) // Increment sync sem (so read can now proceed).
		pError("Bad sem_post from write");
}

void * phase(void *arg)
{
	threadArg *tArg = (threadArg *) arg; // Cast to proper definition.
	uint oneHop, twoHop; // Values read in from queue.
	BOOL done = FALSE;
	uint phaseNo = 1;

	while (!done)
	{
		// We need to differentiate between active or relay phase.
		if (tArg->proc->active)
		{
			printf("[%u][%u][%u]\n", phaseNo++, tArg->proc->uId, tArg->proc->tempUId);

			// Perform active phase.
			write(&tArg->nextProc->channel, &(tArg->nextProc->sync), &(tArg->nextProc->crit), tArg->proc->tempUId);
			oneHop = read(&tArg->proc->channel, &(tArg->proc->sync), &(tArg->proc->crit));
			write(&tArg->nextProc->channel, &(tArg->nextProc->sync), &(tArg->nextProc->crit), oneHop);

			twoHop = read(&tArg->proc->channel, &(tArg->proc->sync), &(tArg->proc->crit));
			if (oneHop == tArg->proc->tempUId)
			{
				// Report this node is the leader and finish.
				printf("leader: %u\n", tArg->proc->uId);
				done = TRUE;
			}
			else if ((oneHop > twoHop) && (oneHop > tArg->proc->tempUId))
				tArg->proc->tempUId = oneHop; // Update tempUId and remain active.
			else
				tArg->proc->active = FALSE; // Become relay node.
		}
		else
		{
			// Perform relay phase.
			oneHop = read(&tArg->proc->channel, &(tArg->proc->sync), &(tArg->proc->crit));
			write(&tArg->nextProc->channel, &(tArg->nextProc->sync), &(tArg->nextProc->crit), oneHop);
			twoHop = read(&tArg->proc->channel, &(tArg->proc->sync), &(tArg->proc->crit));
			write(&tArg->nextProc->channel, &(tArg->nextProc->sync), &(tArg->nextProc->crit), twoHop);

			if (oneHop == twoHop)
				done = TRUE; // This thread may end; leader has been found.
		}
	}

	return arg; // Preferable over pthread_exit for unwound stack.
}
