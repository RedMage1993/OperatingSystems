//****************************************
// Author: Fritz Ammon
// Date: 11 October 2014
// Program: CS 370 Project 3
// Description: A scheduler class that
// contains common members and functions.
//
// May be improved with virtual functions
// for a more specific child scheduler
// with templates.
//****************************************

#ifndef SCHEDTYPE_H
#define SCHEDTYPE_H

#include <cstdio>
#include "linQueueType.h"

using namespace std;

class schedType
{
public:
    schedType(); // Constructor -- initialize private members.
    ~schedType(); // Deconstructor -- free memory and cleanup.

    // Inserts the process into the start up queue.
    void submit(procType *proc);

    // Return processes in all queue's. (At the end this should be from Finished queue only.)
    procType * enumProcs();
    
    // Run scheduler. Scheduler done when each process has no more CPU bursts.
    void run();

	void endStats();

private:
    // We could have a parent queue class of which 3 child schedule types,
    // startup, active/expired, io, and finished may be derived. The parent
    // would have a pure virtual insertion algorithm to create an ordered list.
    // However, because these are the only real differences, it is easier to
    // create a way to differentiate between the 3 insertion styles at runtime.
    linQueueType startup; // Processes start time > clock.
    linQueueType *active, *expired; // Waiting to use the CPU until finished.
    linQueueType inout; // Processes waiting for IO operation.
    linQueueType finished; // When process got all required CPU time.

    int calcPriority(uint nice); // Calculate original priority of proc.
    int calcBonus(uint cpuTime, uint ioTime); // Need proc to hold total CPU and IO time.
    int calcTimeSlice(uint pri); // Calculate timeslice to remove from CPU burst.

    procType * checkForSwitch(int pri, uint pid); // Checks if higher pri process in active queue.

    procType *cpu;
};

#endif
