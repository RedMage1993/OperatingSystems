//**************************************
// Author: Fritz Ammon
// Date: 14 October 2014
// Program: procType class header.
// Description: Class to hold information
// about a process and basic functions.
// *************************************

#ifndef PROCTYPE_H
#define PROCTYPE_H

#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "dataType.h"

using namespace std;

class procType
{
public:
	procType(); // Constructor -- initializes members to NULL.
	~procType(); // Destructor -- frees any space if necessary.

	void init(string &pInfo); // Fills members with process specific info.

	uint getPid();
	uint getArrival();

	uint getEndTime();
	void setEndTime(uint fin);

	int getOPri();
	void setOPri(int oPri);

	int getPri();
	void setPri(int p);

	int getNiceness(); // Used for calculation of oPri.

	uint getTimeSlice();
	void setTimeSlice(uint ts);

	void tick(); // Decrement timeslice.
	void decIO(); // Decrement current IO burst.

	uint currCpuLeft(); // Time left for current CPU burst.
	uint cpuBurstsLeft(); // The size of cpuBursts.

	uint currIoLeft(); // Time left for current IO burst.

	// Used in calculating bonus for priority.
	uint cpuTime();
	uint ioTime();

private:
	static uint noProcs;

	uint pid;
	int nice;
	uint arrivalTime, endTime;
	int origPri, pri;
	uint timeSlice;
	uint totCpu, totIO; // Total amount of cpu and io time currently.
	
	vector<uint> cpuBursts;
	vector<uint> ioBursts; // 1 less than cpuBursts.
};

#endif
