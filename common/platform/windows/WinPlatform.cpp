// =============================================================================
// IrineCore — common/platform/windows/WinPlatform.cpp
// Windows platform service implementations (Win32 API, WinSock2, IOCP).
// =============================================================================
#if !defined(_WIN32)
#error "common/platform/windows/WinPlatform.cpp must only be compiled on Windows targets."
#endif

#include "../Platform.h"
#include <mmsystem.h>
#include <direct.h>
#include <algorithm>

namespace Irine {
namespace Platform {

std::string WideToUtf8(const WCHAR* wstr, int len)
{
    if (!wstr) return {};
    int srcLen = (len < 0) ? static_cast<int>(wcslen(wstr)) : len;
    if (srcLen == 0) return {};
    int needed = ::WideCharToMultiByte(CP_UTF8, 0, wstr, srcLen, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return {};
    std::string out(static_cast<size_t>(needed), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wstr, srcLen, &out[0], needed, nullptr, nullptr);
    return out;
}

std::wstring Utf8ToWide(const char* str, int len)
{
    if (!str) return {};
    int srcLen = (len < 0) ? static_cast<int>(strlen(str)) : len;
    if (srcLen == 0) return {};
    int needed = ::MultiByteToWideChar(CP_UTF8, 0, str, srcLen, nullptr, 0);
    if (needed <= 0) return {};
    std::wstring out(static_cast<size_t>(needed), L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, str, srcLen, &out[0], needed);
    return out;
}

uint32_t GetTickCountMs() noexcept
{
    return static_cast<uint32_t>(::timeGetTime());
}

uint64_t GetTickCount64Ms() noexcept
{
    return static_cast<uint64_t>(::GetTickCount64());
}

void GetLocalSystemTime(SYSTEMTIME* outTime) noexcept
{
    if (outTime)
        ::GetLocalTime(outTime);
}

void SleepMs(uint32_t milliseconds) noexcept
{
    ::Sleep(static_cast<DWORD>(milliseconds));
}

uint32_t GetCurrentThreadId() noexcept
{
    return static_cast<uint32_t>(::GetCurrentThreadId());
}

uint32_t GetCurrentProcessId() noexcept
{
    return static_cast<uint32_t>(::GetCurrentProcessId());
}

uint32_t GetProcessorCount() noexcept
{
    SYSTEM_INFO sysInfo{};
    ::GetSystemInfo(&sysInfo);
    return (sysInfo.dwNumberOfProcessors > 0) ? static_cast<uint32_t>(sysInfo.dwNumberOfProcessors) : 1u;
}

std::string NormalizePath(const std::string& path)
{
    std::string out = path;
    std::replace(out.begin(), out.end(), '/', '\\');
    return out;
}

std::wstring NormalizePathW(const std::wstring& path)
{
    std::wstring out = path;
    std::replace(out.begin(), out.end(), L'/', L'\\');
    return out;
}

bool EnsureDirectoryExists(const std::string& path)
{
    if (path.empty()) return false;
    std::string norm = NormalizePath(path);
    if (::CreateDirectoryA(norm.c_str(), nullptr) != 0)
        return true;
    return ::GetLastError() == ERROR_ALREADY_EXISTS;
}

std::string GetExecutableDirectory()
{
    char buf[MAX_PATH] = { 0 };
    DWORD len = ::GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len == 0) return ".";
    std::string full(buf, len);
    size_t pos = full.find_last_of("\\/");
    return (pos != std::string::npos) ? full.substr(0, pos) : ".";
}

void InstallCrashHandler(const char* /*appName*/) noexcept
{
    // Windows servers use CMiniDumpSet / SetUnhandledExceptionFilter
}

class WinIocpPoller final : public IPlatformPoller
{
public:
    WinIocpPoller() = default;
    ~WinIocpPoller() override { Shutdown(); }

    bool Initialize(uint32_t concurrentThreads) override
    {
        Shutdown();
        m_hIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, static_cast<DWORD>(concurrentThreads));
        return m_hIocp != nullptr;
    }

    void Shutdown() override
    {
        if (m_hIocp)
        {
            ::CloseHandle(m_hIocp);
            m_hIocp = nullptr;
        }
    }

    bool AssociateSocket(SOCKET sock, uintptr_t completionKey) override
    {
        if (!m_hIocp || sock == INVALID_SOCKET) return false;
        HANDLE h = ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), m_hIocp, static_cast<ULONG_PTR>(completionKey), 0);
        return h == m_hIocp;
    }

    bool PostCompletion(uintptr_t completionKey, uint32_t bytesTransferred, void* overlapped) override
    {
        if (!m_hIocp) return false;
        return ::PostQueuedCompletionStatus(
            m_hIocp,
            static_cast<DWORD>(bytesTransferred),
            static_cast<ULONG_PTR>(completionKey),
            reinterpret_cast<LPOVERLAPPED>(overlapped)) != FALSE;
    }

    bool WaitCompletion(PollEvent& outEvent, uint32_t timeoutMs) override
    {
        if (!m_hIocp) return false;
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED ov = nullptr;
        BOOL ok = ::GetQueuedCompletionStatus(m_hIocp, &bytes, &key, &ov, static_cast<DWORD>(timeoutMs));
        outEvent.completionKey    = static_cast<uintptr_t>(key);
        outEvent.overlapped       = ov;
        outEvent.bytesTransferred = static_cast<uint32_t>(bytes);
        outEvent.success          = (ok != FALSE);
        outEvent.errorCode        = ok ? 0 : static_cast<int32_t>(::GetLastError());
        return ok != FALSE || ov != nullptr;
    }

    HANDLE GetNativeHandle() const override { return m_hIocp; }

private:
    HANDLE m_hIocp = nullptr;
};

std::unique_ptr<IPlatformPoller> CreatePlatformPoller()
{
    return std::unique_ptr<IPlatformPoller>(new WinIocpPoller());
}

} // namespace Platform
} // namespace Irine
