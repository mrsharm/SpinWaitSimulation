// Adopted from https://github.com/umezawatakeshi/GetLogicalProcessorInformationEx/blob/master/GetLogicalProcessorInformationEx.cc
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"

class GroupProcNo
{
	uint16_t m_groupProc;

public:

	static const uint16_t NoGroup = 0;

	GroupProcNo(uint16_t groupProc) : m_groupProc(groupProc)
	{
	}

	GroupProcNo(uint16_t group, uint16_t procIndex) : m_groupProc((group << 6) | procIndex)
	{
		assert(group <= 0x3ff);
		assert(procIndex <= 0x3f);
	}

	uint16_t GetGroup() { return m_groupProc >> 6; }
	uint16_t GetProcIndex() { return m_groupProc & 0x3f; }
	uint16_t GetCombinedValue() { return m_groupProc; }
};


void GetProcessorInfo(int* processorCount, int* processorGroupCount)
{
	*processorGroupCount = 1;
	*processorCount = 1;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* pBuffer;
	DWORD cbBuffer;
	char dummy[256] = {};

	pBuffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)dummy;
	cbBuffer = 1;
	if (GetLogicalProcessorInformationEx(RelationAll, pBuffer, &cbBuffer))
	{
		//printf("GetLogicalProcessorInformationEx returned nothing successfully.\n");
		*processorCount = 0;
		return;
	}
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("GetLogicalProcessorInformationEx returned error (1). GetLastError() = %u\n", GetLastError());
		*processorCount = 1;
		return;
	}

	pBuffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(cbBuffer);
	if (!GetLogicalProcessorInformationEx(RelationAll, pBuffer, &cbBuffer))
	{
		printf("GetLogicalProcessorInformationEx returned error (2). GetLastError() = %u\n", GetLastError());
		*processorCount = 2;
		return;
	}

	//printf("GetLogicalProcessorInformationEx returned %u byte data.\n", cbBuffer);

	char* pCur = (char*)pBuffer;
	char* pEnd = pCur + cbBuffer;
	for (int idx = 0; pCur < pEnd; pCur += ((SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)pCur)->Size, ++idx)
	{
		pBuffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)pCur;
		if (pBuffer->Relationship == RelationGroup)
		{
			GROUP_RELATIONSHIP& info = pBuffer->Group;
			int activeGroupCount = info.ActiveGroupCount;
			*processorGroupCount = activeGroupCount;
			int activeProcessorCount = 0;
			for (int i = 0; i < activeGroupCount; i++)
			{
				PROCESSOR_GROUP_INFO& ginfo = info.GroupInfo[i];
				activeProcessorCount += ginfo.ActiveProcessorCount;
			}
			*processorCount = activeProcessorCount;
			return;
		}
	}
}

// Helper function to count set bits in the processor mask.
DWORD CountSetBits(ULONG_PTR bitMask)
{
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i)
	{
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}

	return bitSetCount;
}

/// <summary>
/// Credit: https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformation
/// </summary>
/// <returns></returns>
bool IsHyperThreadingEnabled()
{
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD cbBuffer;

	cbBuffer = 0;
	BOOL done = FALSE;

	while (!done)
	{
		BOOL ret = GetLogicalProcessorInformation(buffer, &cbBuffer);
		if (ret == FALSE)
		{
			DWORD lastError = GetLastError();
			if (lastError == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer)
				{
					free(buffer);
				}

				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(cbBuffer);

				if (NULL == buffer)
				{
					printf("\nError: Allocation failure\n");
					return false;
				}
			}
			else
			{
				printf("\nError %d\n", lastError);
				return false;
			}
		}
		else
		{
			done = TRUE;
		}
	}

	ptr = buffer;
	DWORD byteOffset = 0;
	int processorCoreCount = 0, logicalProcessorCount = 0;
	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= cbBuffer)
	{
		if (ptr->Relationship == RelationProcessorCore)
		{
			processorCoreCount++;
			logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}
	return (processorCoreCount != logicalProcessorCount);
}

/// <summary>
/// Hard affinitize the threads to the processors. If hyper threading is ON, it
/// will affinitize every other core.
/// </summary>
/// <param name="processorCount"></param>
/// <param name="isMultiCpuGroup"></param>
/// <param name="threadHandles"></param>
/// <param name="affinitizeEveryOtherProc"></param>
void SetThreadAffinity(int processorCount, 
					   int numGroups, 
	                   std::vector<HANDLE>& threadHandles, 
	                   bool affinitizeEveryOtherProc)
{
	assert(threadHandles.size() == processorCount);

	bool isMultiCpuGroup = (numGroups > 1);

	for (int procNo = 0; procNo < processorCount; procNo++)
	{
		int adjustedProcNo = (affinitizeEveryOtherProc ? (procNo * 2) : procNo);

		//GroupProcNo groupProcNo(numGroups, adjustedProcNo);

		if (isMultiCpuGroup)
		{
			GROUP_AFFINITY ga;
			//ga.Group = (WORD)groupProcNo.GetGroup();
			ga.Group = (WORD)(adjustedProcNo >> 6);
			ga.Reserved[0] = 0; // reserve must be filled with zero
			ga.Reserved[1] = 0; // otherwise call may fail
			ga.Reserved[2] = 0;
			//ga.Mask = (size_t)1 << groupProcNo.GetProcIndex();
			ga.Mask = (size_t)1 << (adjustedProcNo % 64);
			BOOL result = SetThreadGroupAffinity(threadHandles[procNo], &ga, nullptr);
			if (result == 0)
			{
				PRINT_VERBOSE("SetThreadGroupAffinity returned 0 for processor %d. GetLastError() = %u\n", procNo, GetLastError());
				return;
			}
			else
			{
				PRINT_VERBOSE("affinitizing thread %d with group %d, index %d\n", procNo, ga.Group, (adjustedProcNo % 64));
			}
		}
		else
		{
			//DWORD_PTR result = SetThreadAffinityMask(threadHandles[procNo], (DWORD_PTR)1 << groupProcNo.GetProcIndex());
			DWORD_PTR result = SetThreadAffinityMask(threadHandles[procNo], (DWORD_PTR)1 << adjustedProcNo);
			if (result == 0)
			{
				PRINT_VERBOSE("SetThreadGroupAffinity returned 0 for processor %d. GetLastError() = %u\n", procNo, GetLastError());
				return;
			}
			else
			{
				//printf("affinitizing thread %d with index %d\n", procNo, groupProcNo.GetProcIndex());
				PRINT_VERBOSE("affinitizing thread %d with index %d\n", procNo, adjustedProcNo);
			}
		}
	}
}