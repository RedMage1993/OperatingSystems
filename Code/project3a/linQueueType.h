//******************************************
// Author: Fritz Ammon
// Date: 14 October 2014
// Program: linQueue class header.
// Description: Declaration of linQueue
// for linux scheduler simulation.
// It is a special linux queue type.
//******************************************

#ifndef LINQUEUETYPE_H
#define LINQUEUETYPE_H

#include "procType.h"

struct linNode
{
	procType *proc;
	linNode *next;
};

class linQueueType
{
public:
	linQueueType(); // Constructor - init members.
	~linQueueType(); // Destructor - release nodes.

	void init(qStyle s); // Determines which type of queue this is.

	void add(procType *proc);
	linNode * remove(procType *proc); // Returns node that comes after proc's node.

	linNode * first(); // Returns pointer to first node (head).

	uint size();

private:
	qStyle style; // How the add function should work.
	linNode *head; // Pointer to first node.
	uint length; // Num of nodes in queue.
};

#endif