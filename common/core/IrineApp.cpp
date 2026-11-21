// =============================================================================
// IrineCore — common/core/IrineApp.cpp
// Cross-platform server application lifecycle, banner, and signal management.
// =============================================================================
#include "IrineApp.h"
#include <atomic>
#include <cstdio>

#if !defined(_WIN32)
#include <signal.h>
#endif

namespace Irine {
namespace Core {

namespace {
std::atomic<bool> s_shutdownRequested{ false };
uint64_t          s_startTickMs = 0;
const char*       s_serverName  = "IrineServer";

#if defined(_WIN32)
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        s_shutdownRequested.store(true, std::memory_order_release);
        return TRUE;
    default:
        return FALSE;
    }
}
#else
void PosixShutdownHandler(int /*sig*/)
{
    s_shutdownRequested.store(true, std::memory_order_release);
}
#endif
} // namespace

void Application::InitializeProcess(const char* serverName) noexcept
{
    if (serverName && serverName[0] != '\0')
        s_serverName = serverName;
    s_startTickMs = Irine::Platform::GetTickCount64Ms();
    s_shutdownRequested.store(false, std::memory_order_release);

    Irine::Platform::InstallCrashHandler(s_serverName);

#if defined(_WIN32)
    ::SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#else
    struct sigaction sa{};
    sa.sa_handler = PosixShutdownHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    ::sigaction(SIGINT,  &sa, nullptr);
    ::sigaction(SIGTERM, &sa, nullptr);
#endif
}

void Application::RequestShutdown() noexcept
{
    s_shutdownRequested.store(true, std::memory_order_release);
}

bool Application::IsShutdownRequested() noexcept
{
    return s_shutdownRequested.load(std::memory_order_acquire);
}

uint64_t Application::GetUptimeSeconds() noexcept
{
    uint64_t now = Irine::Platform::GetTickCount64Ms();
    return (now >= s_startTickMs) ? ((now - s_startTickMs) / 1000ULL) : 0ULL;
}

const char* Application::GetServerName() noexcept
{
    return s_serverName;
}

const char* Application::GetFrameworkVersion() noexcept
{
    return "IrineCore 1.0.0 (Cap 60 Cross-Platform Server Framework)";
}

} // namespace Core
} // namespace Irine
