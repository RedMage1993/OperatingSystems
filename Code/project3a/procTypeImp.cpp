//***********************************
// Author: Fritz Ammon
// Date: 14 October 2014
// Program: procType implementation.
// Description: Defines funcs.
//***********************************

#include "procType.h"

uint procType::noProcs = 0; // Helps us know pid.

procType::procType()
{
	pid = 0;
	nice = 0;
	arrivalTime = endTime = 0;
	origPri = pri = 0;
	timeSlice = 0;
	totCpu = totIO = 0;
}

procType::~procType()
{
	// Nothing atm.
}

void procType::init(string &pInfo)
{
	char *info, *token;
	uint noCpuBursts;
	uint index;

	// pInfo is a string, basically the line describing the process.
	// So we parse it to fill in basic data members.

	info = new char[pInfo.size() + 1]; // +1 for null.
	strcpy(info, pInfo.c_str());

	pid = noProcs;

	// Start parsing string for info.
	token = strtok(info, " ");
	sscanf(token, "%d", &nice);

	token = strtok(NULL, " ");
	sscanf(token, "%d", &arrivalTime);

	token = strtok(NULL, " ");
	sscanf(token, "%d", &noCpuBursts);

	// Initialize vectors.
	cpuBursts.resize(noCpuBursts, 0);
	ioBursts.resize(noCpuBursts - 1, 0);

	// Save the first cpu burst length.
	index = 0;

	token = strtok(NULL, " ");
	sscanf(token, "%d", &cpuBursts[index]);

	// The following is IO/CPU pairs...
	token = strtok(NULL, " ");
	while (token != NULL)
	{
		sscanf(token, "%d", &ioBursts[index]);

		token = strtok(NULL, " "); // If an IO was there, then there is CPU.
		sscanf(token, "%d", &cpuBursts[++index]);

		token = strtok(NULL, " ");
	}

	noProcs++; // Increment process count.
}

uint procType::getPid()
{
	return pid;
}

uint procType::getArrival()
{
	return arrivalTime;
}

uint procType::getEndTime()
{
	return endTime;
}

void procType::setEndTime(uint fin)
{
	endTime = fin;
}

int procType::getOPri()
{
	return origPri;
}

void procType::setOPri(int oPri)
{
	origPri = oPri;
}

int procType::getPri()
{
	return pri;
}

void procType::setPri(int p)
{
	pri = p;
}

int procType::getNiceness()
{
	return nice;
}

uint procType::getTimeSlice()
{
	return timeSlice;
}

void procType::setTimeSlice(uint ts)
{
	timeSlice = ts;
}

void procType::tick()
{
	timeSlice--;
	cpuBursts[0]--;
	totCpu++; // Count total cpu.
}

void procType::decIO()
{
	ioBursts[0]--;
	totIO++; // Count total io.
}

uint procType::currCpuLeft()
{
	uint timeLeft = -1;

	if (cpuBursts.size() > 0 && (timeLeft = cpuBursts[0]) == 0)
	{
		// Remove the burst from the list.
		cpuBursts.erase(cpuBursts.begin());
		return timeLeft;
	}
	else
		return timeLeft;
}

uint procType::currIoLeft()
{
	uint timeLeft = -1;

	if (ioBursts.size() > 0 && (timeLeft = ioBursts[0]) == 0)
	{
		// Remove the burst from the list.
		ioBursts.erase(ioBursts.begin());
		return timeLeft;
	}
	else
		return timeLeft;
}

uint procType::cpuBurstsLeft()
{
	return cpuBursts.size(); // NOT the capacity.
}

uint procType::cpuTime()
{
	return totCpu;
}

uint procType::ioTime()
{
	return totIO;
}