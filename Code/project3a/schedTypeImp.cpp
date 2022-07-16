//*************************************
// Author: Fritz Ammon
// Date: 13 October 2014
// Program: CS 370 Project 3
// Description: An implementation of
// the functions used by the scheduler
// class.
//
// A CPU structure mainly for the proc
// timeslice decrement is not used.
//*************************************

#include "schedType.h"

schedType::schedType()
{
    // Initialize queue's and their styles
	startup.init(STARTUP);

    active = new linQueueType();
    active->init(ACTIVE);

    expired = new linQueueType();
    expired->init(EXPIRED);
    
    inout.init(INOUT);

    finished.init(FINISHED);

    cpu = NULL; // Idle cpu.
}

schedType::~schedType()
{
    delete expired; // Delete in reverse order.
    delete active;
}

void schedType::submit(procType *proc)
{
    startup.add(proc); // The queue should insert node by its style.
}

int schedType::calcPriority(uint nice)
{
    return static_cast<int> (((nice + 20) / 39.0 * 30 + 0.5) + 105);
}

int schedType::calcBonus(uint cpuTime, uint ioTime)
{
    double ratio = 0.0;

    if (cpuTime < ioTime)
		return static_cast<int> (((1 - cpuTime / static_cast<double> (ioTime)) * 5) - 0.5);
    else
		return static_cast<int> (((1 - ioTime / static_cast<double> (cpuTime)) * 5) + 0.5);
}

int schedType::calcTimeSlice(uint pri)
{
    return static_cast<int> (((1 - pri / 150.0) * 395 + 0.5) + 5);
}

procType * schedType::enumProcs()
{
    static linNode *node = finished.first(), *prev = 0;

    prev = node;
    
	if (node)
		node = node->next; // Node eventually null, signal to user of end (then restart).

	if (prev)
		return prev->proc;
	else
		return NULL;
}

procType * schedType::checkForSwitch(int pri, uint pid)
{
    // The processes in active queue should be ordered by priority,
    // the head/first being the highest (the one that should go next).
    // The processes with same priority should be ordered by PID,
    // so first PID should be ahead of the others.

    // This means we can actually just stop once we've found a
    // process with a higher priority (no need to worry about others).

    // If CPU empty, pri should be 99 (highest impossible priority, so
    // that all in active are higher).

    linNode *node = active->first();

    while (node && node->proc->getPri() < pri)
		node = node->next;

	if (!node) // Nobody in active queue.
		return NULL;

    if (node->proc->getPri() > pri ||
		(node->proc->getPri() == pri && node->proc->getPid() < pid))
		return node->proc;

    return NULL;
}

void schedType::run()
{
    uint clock = 0;
    linNode *node = 0;
    procType *proc = 0;
	int bonus = 0;
	bool done = false;
	linQueueType *temp;

    while (!done)
    {
		// Move processes arriving at this tick (clock)
		// to active queue.
		node = startup.first();
		while (node != NULL)
		{
			proc = node->proc;
			if (proc->getArrival() == clock)
			{
				// Need to calculate original priority first.
				proc->setOPri(calcPriority(proc->getNiceness()));
				proc->setPri(proc->getOPri());

				// Next, calculate its first timeslice.
				proc->setTimeSlice(calcTimeSlice(proc->getPri()));

				node = startup.remove(proc); // Update node with new position.
				active->add(proc);

				// Arrival at active event.
				printf("[%d] <%d> Enters ready queue (Priority: %d, TimeSlice: %d)\n",
					clock, proc->getPid(), proc->getPri(), proc->getTimeSlice());
			}
			else
				node = node->next;
		}

		// Check for preemption.
		if (cpu == NULL) // CPU empty/idle.
		{
			cpu = checkForSwitch(99, -1); // -1 uint max.

			if (cpu)
			{
				active->remove(cpu);
				printf("[%d] <%d> Enters the CPU\n",
					clock, cpu->getPid());
			}
		}
		else // Busy.
		{
			proc = checkForSwitch(cpu->getPri(), cpu->getPid());
			if (proc) // Higher priority process found. Preempt.
			{
				active->remove(proc);
				active->add(cpu); // Current proc goes back in active.

				printf("[%d] <%d> Preempts Process %d\n",
					clock, proc->getPid(), cpu->getPid());

				cpu = proc;
			}
		}

		// Perform cpu tick.
		if (cpu)
			cpu->tick(); // Decrements process timeslice.

		// Decrememnt all process' IO ticks in inout queue.
		node = inout.first();
		while (node)
		{
			node->proc->decIO(); // Subtract 1 from first IO burst in process IO burst list.
			node = node->next;
		}

		clock++;

		if (cpu)
		{
			// If cpu burst exhausted...
			if (cpu->currCpuLeft() == 0) // Head burst has become 0.
			{
				// All cpu bursts finished (including after last IO).
				if (cpu->cpuBurstsLeft() == 0)
				{
					printf("[%d] <%d> Finishes and moves to the Finished Queue\n",
						clock, cpu->getPid());

					cpu->setEndTime(clock);

					finished.add(cpu);
					cpu = NULL;
				}
				else
				{
					// Still has an IO burst (send to inout).
					inout.add(cpu);

					printf("[%d] <%d> Moves to the IO Queue\n",
						clock, cpu->getPid());

					cpu = NULL;
				}
			}

			// If cpu timeslice exhausted...
			if (cpu && cpu->getTimeSlice() == 0)
			{
				// Calculate new priority based on bonus and original pri.
				bonus = calcBonus(cpu->cpuTime(), cpu->ioTime());
				cpu->setPri(cpu->getOPri() + bonus);

				// Calculate next timeslice.
				cpu->setTimeSlice(calcTimeSlice(cpu->getPri()));

				expired->add(cpu);

				printf("[%d] <%d> Finishes its time slice and moves to the Expired Queue\
\n\t(Priority: %d, Timeslice: %d)\n", clock, cpu->getPid(), cpu->getPri(), cpu->getTimeSlice());

				cpu = NULL;
			}
		}

		// Check for completed IO bursts.
		node = inout.first();
		while (node != NULL)
		{
			proc = node->proc;
			if (proc->currIoLeft() == 0) // IO burst complete.
			{
				// If timeslice was finished...
				if (proc->getTimeSlice() == 0)
				{
					node = inout.remove(proc); // Update node with new position.

					// Calculate new priority based on bonus and original pri.
					bonus = calcBonus(proc->cpuTime(), proc->ioTime());
					proc->setPri(proc->getOPri() + bonus);

					// Calculate next timeslice.
					proc->setTimeSlice(calcTimeSlice(node->proc->getPri()));

					expired->add(proc);

					printf("[%d] <%d> Finishes IO and moves to the Expired Queue\
\n\t(Priority: %d, TimeSlice: %d)\n", clock, proc->getPid(), proc->getPri(),
						proc->getTimeSlice());
				}
				else
				{
					node = inout.remove(proc);
					active->add(proc);

					printf("[%d] <%d> Finishes IO and moves to the Ready Queue\n",
						clock, proc->getPid());
				}
			}
			else
				node = node->next;
		}

		// Check if all queue's empty (all are in finished).
		if ((startup.size() | active->size() | expired->size() |
			inout.size()) == 0 && cpu == NULL)
		{
			done = true;
			continue;
		}

		// Need to swap expired and active.
		if (active->size() == 0 && cpu == NULL && expired->size() != 0)
		{
			temp = active;
			active = expired;
			expired = temp;

			printf("[%d] *** Queue Swap\n", clock);
		}
    }
}

void schedType::endStats()
{
	linNode *temp = finished.first();
	uint totTAT = 0, totWT = 0;
	double totCUT = 0.0;
	uint tat = 0, wt = 0;
	double cut = 0.0;

	puts("");

	// Get and show statistics for each process.
	while (temp)
	{
		tat = temp->proc->getEndTime() - temp->proc->getArrival();
		totTAT += tat;

		wt = tat - temp->proc->cpuTime() - temp->proc->ioTime();
		totWT += wt;

		cut = temp->proc->cpuTime() / static_cast<double> (tat);
		totCUT += cut;

		printf("<%d> TAT = %d, TCT = %d, WT = %d, CUT = %.1f\n", temp->proc->getPid(),
			tat, temp->proc->cpuTime(), wt, cut * 100);

		temp = temp->next;
	}

	// Overall performance.
	printf("\nAverage Waiting Time: %.3f\n", totWT / static_cast<double> (finished.size()));
	printf("Average Turnaround Time: %.3f\n", totTAT / static_cast<double> (finished.size()));
	printf("Average CPU Utilization: %.3f\n", totCUT / static_cast<double> (finished.size()) * 100);
}