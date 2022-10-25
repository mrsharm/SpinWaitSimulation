// Adopted from https://github.com/umezawatakeshi/GetLogicalProcessorInformationEx/blob/master/GetLogicalProcessorInformationEx.cc
#include <Windows.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int GetProcessorCount(void)
{
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* pBuffer;
	DWORD cbBuffer;
	char dummy[256] = {};

	pBuffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)dummy;
	cbBuffer = 1;
	if (GetLogicalProcessorInformationEx(RelationAll, pBuffer, &cbBuffer))
	{
		//printf("GetLogicalProcessorInformationEx returned nothing successfully.\n");
		return 0;
	}
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		//printf("GetLogicalProcessorInformationEx returned error (1). GetLastError() = %u\n", GetLastError());
		return 1;
	}

	pBuffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(cbBuffer);
	if (!GetLogicalProcessorInformationEx(RelationAll, pBuffer, &cbBuffer))
	{
		//printf("GetLogicalProcessorInformationEx returned error (2). GetLastError() = %u\n", GetLastError());
		return 2;
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
			int activeProcessorCount = 0;
			for (int i = 0; i < activeGroupCount; i++)
			{
				PROCESSOR_GROUP_INFO& ginfo = info.GroupInfo[i];
				activeProcessorCount += ginfo.ActiveProcessorCount;
			}
			return activeProcessorCount;
		}
	}
}
