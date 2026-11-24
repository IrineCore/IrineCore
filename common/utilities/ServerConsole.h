// =============================================================================
// IrineCore — common/utilities/ServerConsole.h
//
// Shared interactive console loop helper for all 7 server executables.
//
// Background:
//   Every server main() used to run:
//
//       printf("CMD>");
//       fgets(szCmd, sizeof(szCmd), stdin) ? ... : (szCmd[0] = '\0');
//
//   in an endless loop. When a server runs detached (deploy/linux/start.sh
//   uses `< /dev/null`, systemd, Docker, nohup, Windows services, ...) stdin
//   is at EOF, so fgets() returns immediately and the loop spins at 100% CPU,
//   writing "CMD>" to the redirected stdout log forever (logs/<Server>.log
//   grows to many GB). DNVillageServer instead broke out of the loop on EOF
//   and shut itself down right after startup.
//
// Behaviour of Irine::ServerConsole::ReadCommand():
//   - Prints the "CMD>" prompt only when stdin is an interactive terminal.
//   - Returns true with the next command line (CR/LF stripped).
//   - When stdin is closed / not readable, prints a single notice and then
//     blocks quietly (no CPU spin, no log spam) until shutdown is requested.
//   - On Linux/UNIX, SIGTERM and SIGINT (Ctrl+C) request a graceful shutdown:
//     ReadCommand() returns false so the caller breaks out of its loop and runs
//     its normal cleanup path (ClearApp(), save IDs, module shutdown, ...).
//     This is what deploy/linux/stop.sh and systemd send.
// =============================================================================
#pragma once

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <thread>

#if defined(_WIN32)
#include <io.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

namespace Irine {
namespace ServerConsole {

inline volatile std::sig_atomic_t& ShutdownFlag()
{
    static volatile std::sig_atomic_t s_bShutdown = 0;
    return s_bShutdown;
}

inline void OnShutdownSignal(int /*nSignal*/)
{
    ShutdownFlag() = 1;
}

inline bool IsShutdownRequested()
{
    return ShutdownFlag() != 0;
}

inline bool IsStdinInteractive()
{
#if defined(_WIN32)
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(fileno(stdin)) != 0;
#endif
}

inline void InstallShutdownSignalHandlers()
{
    static bool s_bInstalled = false;
    if (s_bInstalled)
        return;
    s_bInstalled = true;
    (void)ShutdownFlag(); // initialise the static before any signal can arrive

#if !defined(_WIN32)
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &OnShutdownSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // no SA_RESTART: a blocking fgets() must be interrupted
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
#endif
}

// Reads the next console command into szCmd (size nSize, CR/LF stripped).
// Returns false when the server should shut down.
inline bool ReadCommand(char* szCmd, size_t nSize)
{
    static bool s_bStdinClosed = false;

    InstallShutdownSignalHandlers();
    if (szCmd && nSize > 0)
        szCmd[0] = '\0';
    if (!szCmd || nSize < 2)
        return false;

    while (!IsShutdownRequested())
    {
        if (!s_bStdinClosed)
        {
            if (IsStdinInteractive())
            {
                fputs("CMD>", stdout);
                fflush(stdout);
            }

            if (fgets(szCmd, static_cast<int>(nSize), stdin) != nullptr)
            {
                szCmd[strcspn(szCmd, "\r\n")] = '\0';
                return true;
            }

            szCmd[0] = '\0';
            if (IsShutdownRequested())
                break;

            if (ferror(stdin) && errno == EINTR)
            {
                clearerr(stdin); // interrupted by an unrelated signal; retry
                continue;
            }

            s_bStdinClosed = true;
#if defined(_WIN32)
            fputs("[IrineCore] Console input is not available (stdin closed); "
                  "running without interactive commands.\n", stdout);
#else
            fputs("[IrineCore] Console input is not available (stdin closed); "
                  "running in daemon mode. Send SIGTERM or SIGINT to shut down.\n", stdout);
#endif
            fflush(stdout);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    szCmd[0] = '\0';
    return false;
}

} // namespace ServerConsole
} // namespace Irine
