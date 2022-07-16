//**********************************
// Author: Fritz Ammon
// Date: 14 October 2014
// Program: linQueue implementation
// Description: Definitions for
// linQueue class functions.
//**********************************

#include "linQueueType.h"

linQueueType::linQueueType()
{
	head = NULL;
	length = 0;
}

linQueueType::~linQueueType()
{
	linNode *temp = head;

	// Delete all of the nodes that were allocated.
	while (temp)
	{
		head = head->next;
		delete temp;
		temp = head;
	}
}

void linQueueType::init(qStyle s)
{
	style = s;
}

void linQueueType::add(procType *proc)
{
	linNode *newNode, *temp = head, *prev = head;
	bool done = false;

	newNode = new linNode;
	newNode->proc = proc;

	while (temp && !done)
	{
		switch (style)
		{
		case STARTUP: // Need temp to have greater start time.
			if (temp->proc->getArrival() > proc->getArrival())
			{
				done = true;
				continue;
			}

			break;

		case ACTIVE: // Need temp to have lower priority (to go after).
		case EXPIRED: // By lower priority, we mean it has a smaller pri value.
			if (temp->proc->getPri() < proc->getPri())
			{
				done = true;
				continue;
			}

			break;

		case INOUT: // Need temp to have greater IO burst remaining.
			if (temp->proc->currIoLeft() > proc->currIoLeft())
			{
				done = true;
				continue;
			}

			break;

		case FINISHED: // Need temp to have greater/later finish time.
			if (temp->proc->getEndTime() > proc->getEndTime())
			{
				done = true;
				continue;
			}

			break;
		}

		prev = temp;
		temp = temp->next;
	}

	if (temp == head) // If head is null.
	{
		newNode->next = head;
		head = newNode;
	}
	else
	{
		// If anything is to be pushed back 1 spot,
		// or if it shall become last (temp will have become NULL, done false).
		// 
		newNode->next = temp;
		prev->next = newNode;
	}

	length++;
}

linNode * linQueueType::remove(procType *proc)
{
	linNode *temp = head, *prev = head;
	linNode *after = NULL;

	// Find the right node.
	while (temp)
	{
		if (temp->proc == proc)
			break;

		prev = temp;
		temp = temp->next;
	}

	if (temp == NULL)
		return NULL;
	else
		after = temp->next;

	// Remove as necessary.
	if (temp == head)
	{
		head = temp->next;
		delete temp;
	}
	else
	{
		prev->next = temp->next;
		delete temp;
	}

	length--;

	return after;
}

linNode * linQueueType::first()
{
	return head;
}

uint linQueueType::size()
{
	return length;
}