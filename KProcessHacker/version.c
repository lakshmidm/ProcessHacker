/*
 * Process Hacker Driver - 
 *   Windows version-specific data
 * 
 * Copyright (C) 2009 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _VERSION_PRIVATE
#include "include/version.h"
#include "include/debug.h"

/*
 * mov      edi, edi
 * push     ebp
 * mov      ebp, esp
 */
static char StandardPrologue[] = { 0x8b, 0xff, 0x55, 0x8b, 0xec };

/* PsTerminateProcess/PspTerminateProcess */
static char PspTerminateProcess51[] =
{
    0x8b, 0xff, 0x55, 0x8b, 0xec, 0x56, 0x64, 0xa1,
    0x24, 0x01, 0x00, 0x00, 0x8b, 0x75, 0x08, 0x3b
};
static char PsTerminateProcess60[] =
{
    0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x56, 0x57,
    0x33, 0xd2, 0x6a, 0x08, 0x42, 0x5e, 0x8d, 0xb9
};
static char PsTerminateProcess61[] =
{
    0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x56, 0x57,
    0x33, 0xd2, 0x6a, 0x08, 0x42, 0x5e, 0x8d, 0xb9
}; /* same as 6.0 */

/* PspTerminateThreadByPointer */
static char PspTerminateThreadByPointer51[] =
{
    0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xec, 0x0c,
    0x83, 0x4d, 0xf8, 0xff, 0x56, 0x57, 0x8b, 0x7d
};
static char PspTerminateThreadByPointer60[] =
{
    0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8,
    0x51, 0x53, 0x56, 0x8b, 0x75, 0x08, 0x57, 0x8d,
    0xbe, 0x60, 0x02, 0x00, 0x00, 0xf6, 0x07, 0x40
};
static char PspTerminateThreadByPointer61[] =
{
    0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8,
    0x51, 0x53, 0x56, 0x8b, 0x75, 0x08, 0x57, 0x8d,
    0xbe, 0x60, 0x02, 0x00, 0x00, 0xf6, 0x07, 0x40
}; /* same as 6.0 */

/* The following offsets took me a long time to work out, so 
   please do not steal them. If you want to use them, please 
   license your project under the GNU GPL (although you are 
   not legally required to).
 */
NTSTATUS KvInit()
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG majorVersion, minorVersion, servicePack, buildNumber;
    
    RtlWindowsVersion.dwOSVersionInfoSize = sizeof(RtlWindowsVersion);
    status = RtlGetVersion((PRTL_OSVERSIONINFOW)&RtlWindowsVersion);
    
    if (!NT_SUCCESS(status))
        return status;
    
    majorVersion = RtlWindowsVersion.dwMajorVersion;
    minorVersion = RtlWindowsVersion.dwMinorVersion;
    servicePack = RtlWindowsVersion.wServicePackMajor;
    buildNumber = RtlWindowsVersion.dwBuildNumber;
    dprintf("Windows %d.%d, SP%d.%d, build %d\n",
        majorVersion, minorVersion, servicePack,
        RtlWindowsVersion.wServicePackMinor, buildNumber
        );
    
    __NtClose = GetSystemRoutineAddress(L"NtClose");
    
    /* NtClose is used as a reference point for most addresses 
       dependent on where the kernel is loaded, so if we don't 
       have it, we can't proceed.
     */
    if (!__NtClose)
        return STATUS_NOT_SUPPORTED;
    
    /* Windows XP */
    if (majorVersion == 5 && minorVersion == 1)
    {
        ULONG_PTR searchOffset = (ULONG_PTR)__NtClose;
        
        WindowsVersion = WINDOWS_XP;
        ProcessAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xfff;
        ThreadAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3ff;
        
        OffEtClientId = 0x1ec;
        OffEtStartAddress = 0x224;
        OffEtWin32StartAddress = 0x228;
        OffEpJob = 0x134;
        OffEpObjectTable = 0xc4;
        OffEpProtectedProcessOff = 0;
        OffEpProtectedProcessBit = 0;
        OffEpRundownProtect = 0x80;
        OffOtiGenericMapping = 0x60 + 0x8;
        OffOhBody = 0x18;
        
        /* We are scanning for PspTerminateProcess which has 
           the same signature as PsTerminateProcess because 
           PsTerminateProcess is simply a wrapper on XP.
         */
        INIT_SCAN(
            PsTerminateProcessScan,
            PspTerminateProcess51,
            16, searchOffset, SCAN_LENGTH, 0
            );
        INIT_SCAN(
            PspTerminateThreadByPointerScan,
            PspTerminateThreadByPointer51,
            16, searchOffset, SCAN_LENGTH, 0
            );
        
        /* Windows XP SP0 and 1 are not supported */
        if (servicePack == 0)
        {
            return STATUS_NOT_SUPPORTED;
        }
        else if (servicePack == 1)
        {
            return STATUS_NOT_SUPPORTED;
        }
        else if (servicePack == 2)
        {
        }
        else if (servicePack == 3)
        {
        }
        else
        {
            return STATUS_NOT_SUPPORTED;
        }
        
        dprintf("Initialized version-specific data for Windows XP SP%d\n", servicePack);
    }
    /* Windows Server 2003 */
    else if (majorVersion == 5 && minorVersion == 2)
    {
        WindowsVersion = WINDOWS_SERVER_2003;
        
        /* Not supported yet */
        return STATUS_NOT_SUPPORTED;
    }
    /* Windows Vista, Windows Server 2008 */
    else if (majorVersion == 6 && minorVersion == 0)
    {
        ULONG_PTR searchOffset = (ULONG_PTR)__NtClose;
        
        WindowsVersion = WINDOWS_VISTA;
        ProcessAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xffff;
        ThreadAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xffff;
        OffEtClientId = 0x20c;
        OffEtStartAddress = 0x1f8;
        OffEtWin32StartAddress = 0x240;
        OffEpJob = 0x10c;
        OffEpObjectTable = 0xdc;
        OffEpProtectedProcessOff = 0x224;
        OffEpProtectedProcessBit = 0xb;
        OffEpRundownProtect = 0x98;
        OffOhBody = 0x18;
        
        INIT_SCAN(
            PsTerminateProcessScan,
            PsTerminateProcess60,
            16, searchOffset, SCAN_LENGTH, 0
            );
        INIT_SCAN(
            PspTerminateThreadByPointerScan,
            PspTerminateThreadByPointer60,
            24, searchOffset - 0x50000, SCAN_LENGTH, 0
            );
        
        /* SP0 */
        if (servicePack == 0)
        {
            OffOtiGenericMapping = 0x60 + 0xc;
        }
        /* SP1 */
        else if (servicePack == 1)
        {
            OffOtiGenericMapping = 0x28 + 0xc; /* They got rid of the Mutex (an ERESOURCE) */
        }
        else
        {
            return STATUS_NOT_SUPPORTED;
        }
        
        dprintf("Initialized version-specific data for Windows Vista SP%d/Windows Server 2008\n", servicePack);
    }
    /* Windows 7 */
    else if (majorVersion == 6 && minorVersion == 1)
    {
        ULONG_PTR psSearchOffset = (ULONG_PTR)GetSystemRoutineAddress(L"PsSetCreateProcessNotifyRoutine");
        ULONG psScanLength = 0x200000;
        
        if (!psSearchOffset)
            return STATUS_NOT_SUPPORTED;
        
        WindowsVersion = WINDOWS_7;
        ProcessAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xffff;
        ThreadAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xffff;
        OffEtClientId = 0x22c;
        OffEtStartAddress = 0x218;
        OffEtWin32StartAddress = 0x260;
        OffEpJob = 0x124;
        OffEpObjectTable = 0xf4;
        OffEpProtectedProcessOff = 0x25c;
        OffEpProtectedProcessBit = 0xb;
        OffEpRundownProtect = 0xb0;
        OffOtiGenericMapping = 0x28 + 0xc;
        OffOhBody = 0x18;
        
        INIT_SCAN(
            PsTerminateProcessScan,
            PsTerminateProcess61,
            16, psSearchOffset, psScanLength, 0
            );
        INIT_SCAN(
            PspTerminateThreadByPointerScan,
            PspTerminateThreadByPointer61,
            24, psSearchOffset, psScanLength, 0
            );
        
        /* SP0 */
        if (servicePack == 0)
        {
        }
        else
        {
            return STATUS_NOT_SUPPORTED;
        }
        
        dprintf("Initialized version-specific data for Windows 7 SP%d\n", servicePack);
    }
    else
    {
        return STATUS_NOT_SUPPORTED;
    }
    
    return status;
}

PVOID KvVerifyPrologue(
    ULONG_PTR Offset
    )
{
    PVOID address = (PVOID)((ULONG_PTR)__NtClose + Offset);
    
    if (memcmp(address, StandardPrologue, 5) == 0)
        return address;
    else
        return NULL;
}

PVOID KvScanProc(
    PKV_SCANPROC ScanProc
    )
{
    PCHAR bytes = ScanProc->Bytes;
    ULONG length = ScanProc->Length;
    ULONG_PTR endAddress = ScanProc->StartAddress + ScanProc->ScanLength;
    ULONG_PTR i;
    
    for (i = ScanProc->StartAddress; i < endAddress; i++)
    {
        if (memcmp((PVOID)i, bytes, length) == 0)
            return (PVOID)(i + ScanProc->Displacement);
    }
    
    return NULL;
}
