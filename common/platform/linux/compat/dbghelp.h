#pragma once
#include "windows.h"
enum MINIDUMP_TYPE {
    MiniDumpNormal = 0x00000000,
    MiniDumpWithDataSegs = 0x00000001,
    MiniDumpWithFullMemory = 0x00000002,
    MiniDumpWithHandleData = 0x00000004,
    MiniDumpWithUnloadedModules = 0x00000020,
    MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
    MiniDumpWithProcessThreadData = 0x00000100,
    MiniDumpWithPrivateReadWriteMemory = 0x00000200,
};
using _MINIDUMP_TYPE = MINIDUMP_TYPE;
enum MINIDUMP_STREAM_TYPE {
    ModuleListStream = 4,
    ExceptionStream = 6,
    SystemInfoStream = 7,
    MiscInfoStream = 15,
    LastReservedStream = 0xffff
};
struct MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
};
using PMINIDUMP_EXCEPTION_INFORMATION = MINIDUMP_EXCEPTION_INFORMATION*;
struct MINIDUMP_USER_STREAM { ULONG32 Type; ULONG BufferSize; PVOID Buffer; };
using PMINIDUMP_USER_STREAM = MINIDUMP_USER_STREAM*;
struct MINIDUMP_USER_STREAM_INFORMATION { ULONG UserStreamCount; PMINIDUMP_USER_STREAM UserStreamArray; };
using PMINIDUMP_USER_STREAM_INFORMATION = MINIDUMP_USER_STREAM_INFORMATION*;
struct MINIDUMP_CALLBACK_INFORMATION { PVOID CallbackRoutine; PVOID CallbackParam; };
using PMINIDUMP_CALLBACK_INFORMATION = MINIDUMP_CALLBACK_INFORMATION*;
struct MINIDUMP_CALLBACK_INPUT { ULONG ProcessId; HANDLE ProcessHandle; ULONG CallbackType; };
using PMINIDUMP_CALLBACK_INPUT = MINIDUMP_CALLBACK_INPUT*;
struct MINIDUMP_CALLBACK_OUTPUT { ULONG ModuleWriteFlags; ULONG ThreadWriteFlags; };
using PMINIDUMP_CALLBACK_OUTPUT = MINIDUMP_CALLBACK_OUTPUT*;
struct MINIDUMP_DIRECTORY { ULONG32 StreamType; DWORD DataSize; DWORD Rva; };
using PMINIDUMP_DIRECTORY = MINIDUMP_DIRECTORY*;
struct MINIDUMP_MISC_INFO { ULONG32 SizeOfInfo; ULONG32 Flags1; ULONG32 ProcessId; ULONG32 ProcessCreateTime; ULONG32 ProcessUserTime; ULONG32 ProcessKernelTime; };
struct MINIDUMP_MODULE_LIST { ULONG32 NumberOfModules; };
struct MINIDUMP_SYSTEM_INFO { USHORT ProcessorArchitecture; USHORT ProcessorLevel; USHORT ProcessorRevision; };
struct MINIDUMP_EXCEPTION_STREAM { ULONG32 ThreadId; ULONG32 __alignment; };
struct IMAGEHLP_LINE64 { DWORD SizeOfStruct; PVOID Key; DWORD LineNumber; PCHAR FileName; DWORD64 Address; };
struct STACKFRAME64 { DWORD64 AddrPC; DWORD64 AddrReturn; DWORD64 AddrFrame; DWORD64 AddrStack; };
inline BOOL MiniDumpWriteDump(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PVOID, PVOID) noexcept { return FALSE; }
