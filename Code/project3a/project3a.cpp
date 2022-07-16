//****************************************
// Author: Fritz Ammon
// Date: 1 October 2014
// Program: CS 370 Project 3
// Description: Simulate small version of
// Linux scheduler.
// I will pretend to be the OS, and get
// information of process and submit to
// a scheduler for processing.
// This is under the assumption that we
// have a list of all the processes and
// don't allow this list to change.
// If it did change then that would mean
// arrival time would not be specified.
//****************************************

#include <iostream>
#include <string>
#include "schedType.h"

using namespace std;

bool getNewProc(string &pInfo);

int main()
{
    string pInfo; // String of process information to be parsed.
    procType *proc; // (process) Holds information about a process.
    schedType sched; // (scheduler) Holds necessary queue's and scheduling functions for processes.

    while (getNewProc(pInfo)) // Get new process information.
    {
		proc = new procType();
		proc->init(pInfo); // Have info parsed into private members.

		sched.submit(proc);
    }

    sched.run();

	sched.endStats();

    // Enumerate through processes and delete them.
    while ((proc = sched.enumProcs()))
		delete proc;

    return 0;
}

bool getNewProc(string &pInfo)
{
    getline(cin, pInfo); // We are assuming input is correct.

    return (pInfo != "***"); // More processes left.
}
