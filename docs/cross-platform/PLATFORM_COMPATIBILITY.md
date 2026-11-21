# Cross-Platform Compatibility (Windows & Linux)

IrineCore servers build and run natively on both Windows (MSVC x64) and Linux (GCC/Clang x64) while staying wire-compatible with the Windows game client and SQL Server databases.

## 1. 2-Byte `WCHAR` Wire & ODBC ABI

- Dragon Nest wire packets (`common/Protocol/DNPacket.h`, `src/server/shared/DNServerPacket*.h`) embed fixed-length `WCHAR` arrays inside `#pragma pack(push, 1)` structs.
- Microsoft's `msodbcsql18` ODBC driver on Linux also expects 2-byte UTF-16LE `SQLWCHAR` buffers for `SQLBindParameter`.
- Because GCC/Clang default `wchar_t` to 4 bytes on Linux, `src/cmake/CompilerFlags.cmake` compiles Unix targets with `-fshort-wchar` so `sizeof(WCHAR) == 2` everywhere.
- Linux builds never call glibc's 4-byte `wchar_t` functions on `WCHAR*`; UTF-16 string helpers are implemented in `common/platform/linux/`.

## 2. Networking Backends

- **TCP (`common/networking/IocpManager.*`)**:
  - Windows uses native I/O Completion Ports (`common/platform/windows/WinIocpPoller.cpp`).
  - Linux implements the same poller interface with edge-triggered `epoll` + `eventfd` (`common/platform/linux/LinuxEpollPoller.cpp`).
- **RUDP (`common/networking/RUDPSocketFrame.*`)**:
  - Built on standard UDP datagram sockets (`socket`, `select`, `recvfrom`, `sendto`), shared directly between WinSock2 and POSIX sockets.

## 3. Portable Math (`deps/etmath/`)

Server collision, navigation meshes, and headless animation (`common/EternityEngineSrv`, `common/EtWorldBase`, `common/EngineUtil`) use `deps/etmath/EtMathPortable.h` instead of DirectX 9 SDK headers. Struct layouts (`EtVector2`, `EtVector3`, `EtVector4`, `EtMatrix`, `EtQuat`) match IEEE-754 row-major binary layouts used by `.skn`, `.ani`, `.act`, and `.nav` files.

## 4. Threading, Sync, Time & Config

OS primitives are wrapped under `common/platform/{interfaces,windows,linux}` and `common/`:

- Locks & threads: `CSyncLock` (`CRITICAL_SECTION` / recursive mutex), `CMultiSync` (reader-writer lock), `Thread` (`_beginthreadex` / `std::thread`).
- INI configuration: `common/configuration/IniFile.{h,cpp}` reads UTF-8 and UTF-16LE `.ini` files identically on both platforms without `GetPrivateProfile*`.
- Monotonic timers: `timeGetTime()` / `std::chrono::steady_clock`.
- Fault handling: Windows SEH (`__try`/`__except` + `MiniDumpWriteDump`) on MSVC; standard `try`/`catch` plus POSIX `sigaction` stack trace logging on Linux.

## 5. Case-Sensitive Includes & ODBC

- All `#include` directives must match exact on-disk casing for Linux `ext4`/`xfs` filesystems.
- On Linux, `ODBCINI` points to `env/dist/config/odbc.ini` configured for `ODBC Driver 18 for SQL Server`.
