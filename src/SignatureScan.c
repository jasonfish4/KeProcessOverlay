// This function only works in the context of KeStackAttachProcess with the usermode kprocessor mode
#include "SignatureScan.h"

PVOID GetSignatureBase(HANDLE ProcessId, DWORD64 BaseStart, 
		       DWORD64 BaseEnd, Signature ScanSignature, 
		       ULONG PageProtect, SIZE_T RegionSize, ULONG Type)
{
	DWORD64 cmp64;
	MEMORY_BASIC_INFORMATION mem_basic_info;
	POBJECT_TYPE PsProcessType = NULL;
	PEPROCESS peprocess;
	
  	PsLookupProcessByProcessId(ProcessId, &peprocess);

	HANDLE process = NULL;
	OBJECT_ATTRIBUTES obj_attr;
	CLIENT_ID cid;

	cid.UniqueProcess = ProcessId; //PsGetCurrentProcessId();
	cid.UniqueThread = NULL; //(HANDLE)0;
	InitializeObjectAttributes(&obj_attr, NULL, 0, NULL, NULL);
	ZwOpenProcess(&process, PROCESS_ALL_ACCESS, &obj_attr, &cid);
	
	size_t ReturnLength = 0;
  
  // loop til base start value reaches the base end value
	while (BaseStart < BaseEnd)
	{
    // Query the memory region with the specified filters
		ZwQueryVirtualMemory(process, (PVOID)BaseStart, 
				     MemoryBasicInformation, &mem_basic_info, 
				     sizeof(mem_basic_info), &ReturnLength);
		
		if (mem_basic_info.Protect == PageProtect && 
		    mem_basic_info.RegionSize == RegionSize && mem_basic_info.Type == Type)
		{
			for (size_t BaseIterator = (size_t)mem_basic_info.BaseAddress;
			BaseIterator < ((DWORD64)mem_basic_info.BaseAddress + mem_basic_info.RegionSize); BaseIterator += 8)
			{
				
        // copy 64bit value into cmp64 buffer and then compare it with val1, then repeat option offsetted + 8 with val2
				
				RtlCopyMemory(&cmp64, (PVOID)BaseIterator, sizeof(DWORD64));
				if (cmp64 == ScanSignature.val1)
				{
					RtlCopyMemory(&cmp64, (PVOID)(BaseIterator + 8), sizeof(DWORD64));
					if (cmp64 == ScanSignature.val2)
						return (PVOID)BaseIterator; // return the address with the matching signature
				}
				
			}
		}
		BaseStart += mem_basic_info.RegionSize;
	}
	return NULL; // return null because no results
}
