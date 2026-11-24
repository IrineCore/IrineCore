// =============================================================================
// IrineCore — common/platform/linux/LinuxPlatform.cpp
// Linux platform service implementations (POSIX, BSD sockets, epoll, UTF-16LE).
// =============================================================================
#if !defined(__linux__)
#error "common/platform/linux/LinuxPlatform.cpp must only be compiled on Linux targets."
#endif

#include "../Platform.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <dirent.h>
#include <strings.h>
#include <signal.h>
#include <execinfo.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <cctype>

namespace Irine {
namespace Platform {

std::string WideToUtf8(const WCHAR* wstr, int len)
{
    if (!wstr) return {};
    size_t count = 0;
    if (len < 0)
    {
        while (wstr[count] != 0) ++count;
    }
    else
    {
        count = static_cast<size_t>(len);
    }
    std::string out;
    out.reserve(count * 3);
    for (size_t i = 0; i < count; ++i)
    {
        uint32_t cp = static_cast<uint16_t>(wstr[i]);
        if (cp >= 0xD800 && cp <= 0xDBFF && (i + 1) < count)
        {
            uint32_t low = static_cast<uint16_t>(wstr[i + 1]);
            if (low >= 0xDC00 && low <= 0xDFFF)
            {
                cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
                ++i;
            }
        }
        if (cp < 0x80)
        {
            out.push_back(static_cast<char>(cp));
        }
        else if (cp < 0x800)
        {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else if (cp < 0x10000)
        {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else
        {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

std::wstring Utf8ToWide(const char* str, int len)
{
    if (!str) return {};
    size_t count = (len < 0) ? std::strlen(str) : static_cast<size_t>(len);
    std::wstring out;
    out.reserve(count);
    size_t i = 0;
    while (i < count)
    {
        uint8_t c = static_cast<uint8_t>(str[i]);
        uint32_t cp = 0;
        if (c < 0x80) { cp = c; i += 1; }
        else if ((c & 0xE0) == 0xC0 && i + 1 < count)
        {
            cp = ((c & 0x1F) << 6) | (static_cast<uint8_t>(str[i + 1]) & 0x3F);
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0 && i + 2 < count)
        {
            cp = ((c & 0x0F) << 12) | ((static_cast<uint8_t>(str[i + 1]) & 0x3F) << 6) | (static_cast<uint8_t>(str[i + 2]) & 0x3F);
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0 && i + 3 < count)
        {
            cp = ((c & 0x07) << 18) | ((static_cast<uint8_t>(str[i + 1]) & 0x3F) << 12) | ((static_cast<uint8_t>(str[i + 2]) & 0x3F) << 6) | (static_cast<uint8_t>(str[i + 3]) & 0x3F);
            i += 4;
        }
        else { cp = '?'; i += 1; }

        if (cp <= 0xFFFF)
        {
            out.push_back(static_cast<WCHAR>(cp));
        }
        else
        {
            cp -= 0x10000;
            out.push_back(static_cast<WCHAR>(0xD800 + (cp >> 10)));
            out.push_back(static_cast<WCHAR>(0xDC00 + (cp & 0x3FF)));
        }
    }
    return out;
}

uint64_t GetTickCount64Ms() noexcept
{
    static const auto s_start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now - s_start).count());
}

uint32_t GetTickCountMs() noexcept
{
    return static_cast<uint32_t>(GetTickCount64Ms());
}

void GetLocalSystemTime(SYSTEMTIME* outTime) noexcept
{
    if (!outTime) return;
    struct timeval tv{};
    ::gettimeofday(&tv, nullptr);
    struct tm tmLocal{};
    ::localtime_r(&tv.tv_sec, &tmLocal);
    outTime->wYear         = static_cast<WORD>(tmLocal.tm_year + 1900);
    outTime->wMonth        = static_cast<WORD>(tmLocal.tm_mon + 1);
    outTime->wDayOfWeek    = static_cast<WORD>(tmLocal.tm_wday);
    outTime->wDay          = static_cast<WORD>(tmLocal.tm_mday);
    outTime->wHour         = static_cast<WORD>(tmLocal.tm_hour);
    outTime->wMinute       = static_cast<WORD>(tmLocal.tm_min);
    outTime->wSecond       = static_cast<WORD>(tmLocal.tm_sec);
    outTime->wMilliseconds = static_cast<WORD>(tv.tv_usec / 1000);
}

void SleepMs(uint32_t milliseconds) noexcept
{
    ::usleep(static_cast<useconds_t>(milliseconds) * 1000u);
}

uint32_t GetCurrentThreadId() noexcept
{
    return static_cast<uint32_t>(::syscall(SYS_gettid));
}

uint32_t GetCurrentProcessId() noexcept
{
    return static_cast<uint32_t>(::getpid());
}

uint32_t GetProcessorCount() noexcept
{
    long n = ::sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? static_cast<uint32_t>(n) : 1u;
}

std::string NormalizePath(const std::string& path)
{
    std::string out = path;
    std::replace(out.begin(), out.end(), '\\', '/');
    return out;
}

std::wstring NormalizePathW(const std::wstring& path)
{
    std::wstring out = path;
    std::replace(out.begin(), out.end(), L'\\', L'/');
    return out;
}

std::string ResolveCaseInsensitivePath(const std::string& path, bool forWrite)
{
    if (path.empty()) return {};

    std::string norm = NormalizePath(path);
    // Collapse redundant consecutive slashes (e.g. "a//b" -> "a/b")
    std::string collapsed;
    collapsed.reserve(norm.size());
    for (char c : norm)
    {
        if (c == '/' && !collapsed.empty() && collapsed.back() == '/')
            continue;
        collapsed.push_back(c);
    }
    norm.swap(collapsed);

    // O(1) fast path: exact casing already exists on disk
    struct stat sb{};
    if (::lstat(norm.c_str(), &sb) == 0)
        return norm;

    bool isAbs = (!norm.empty() && norm[0] == '/');
    std::vector<std::string> parts;
    size_t pos = isAbs ? 1 : 0;
    while (pos < norm.size())
    {
        size_t next = norm.find('/', pos);
        if (next == std::string::npos) next = norm.size();
        if (next > pos)
            parts.emplace_back(norm.data() + pos, next - pos);
        pos = next + 1;
    }

    if (parts.empty())
        return isAbs ? "/" : norm;

    std::string cur = isAbs ? "/" : ".";
    bool hasLeadingRelDot = (!isAbs && norm.size() >= 2 && norm[0] == '.' && norm[1] == '/');

    auto joinPath = [&](const std::string& base, const std::string& comp) -> std::string {
        if (base == "/") return "/" + comp;
        if (base == ".") return hasLeadingRelDot ? ("./" + comp) : comp;
        return base + "/" + comp;
    };

    for (size_t i = 0; i < parts.size(); ++i)
    {
        const std::string& comp = parts[i];
        if (comp == ".") continue;
        if (comp == "..")
        {
            cur = joinPath(cur, "..");
            continue;
        }

        std::string exact = joinPath(cur, comp);
        if (::lstat(exact.c_str(), &sb) == 0)
        {
            cur.swap(exact);
            continue;
        }

        // Case-insensitive lookup inside current directory
        DIR* dir = ::opendir(cur.c_str());
        std::string matched;
        if (dir)
        {
            while (struct dirent* ent = ::readdir(dir))
            {
                if (::strcasecmp(ent->d_name, comp.c_str()) == 0)
                {
                    matched = ent->d_name;
                    break;
                }
            }
            ::closedir(dir);
        }

        if (!matched.empty())
        {
            cur = joinPath(cur, matched);
            continue;
        }

        // Component does not exist in cur
        if (forWrite && i + 1 == parts.size())
        {
            return joinPath(cur, comp);
        }

        // Append remaining unresolved components so caller receives standard ENOENT
        for (size_t j = i; j < parts.size(); ++j)
            cur = joinPath(cur, parts[j]);
        return cur;
    }

    return cur;
}

bool EnsureDirectoryExists(const std::string& path)
{
    if (path.empty()) return false;
    std::string norm = NormalizePath(path);
    if (::mkdir(norm.c_str(), 0755) == 0) return true;
    return errno == EEXIST;
}

std::string GetExecutableDirectory()
{
    char buf[MAX_PATH] = { 0 };
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) return ".";
    std::string full(buf, static_cast<size_t>(len));
    size_t pos = full.find_last_of('/');
    return (pos != std::string::npos) ? full.substr(0, pos) : ".";
}

static void CrashSignalHandler(int sig)
{
    void* frames[64];
    int count = ::backtrace(frames, 64);
    ::fprintf(stderr, "\n[IrineCore] Fatal signal %d received. Backtrace (%d frames):\n", sig, count);
    ::backtrace_symbols_fd(frames, count, STDERR_FILENO);
    ::_exit(128 + sig);
}

void InstallCrashHandler(const char* /*appName*/) noexcept
{
    ::signal(SIGPIPE, SIG_IGN);
    struct sigaction sa{};
    sa.sa_handler = CrashSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    ::sigaction(SIGSEGV, &sa, nullptr);
    ::sigaction(SIGABRT, &sa, nullptr);
    ::sigaction(SIGFPE,  &sa, nullptr);
    ::sigaction(SIGBUS,  &sa, nullptr);
}

class LinuxEpollPoller final : public IPlatformPoller
{
public:
    LinuxEpollPoller() = default;
    ~LinuxEpollPoller() override { Shutdown(); }

    bool Initialize(uint32_t /*concurrentThreads*/) override
    {
        Shutdown();
        m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
        if (m_epollFd < 0) return false;
        m_wakeFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (m_wakeFd < 0)
        {
            ::close(m_epollFd);
            m_epollFd = -1;
            return false;
        }
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.u64 = 0;
        ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_wakeFd, &ev);
        return true;
    }

    void Shutdown() override
    {
        if (m_wakeFd >= 0)  { ::close(m_wakeFd);  m_wakeFd  = -1; }
        if (m_epollFd >= 0) { ::close(m_epollFd); m_epollFd = -1; }
    }

    bool AssociateSocket(SOCKET sock, uintptr_t completionKey) override
    {
        if (m_epollFd < 0 || sock < 0) return false;
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
        ev.data.u64 = static_cast<uint64_t>(completionKey);
        return ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, sock, &ev) == 0;
    }

    bool PostCompletion(uintptr_t completionKey, uint32_t bytesTransferred, void* overlapped) override
    {
        if (m_epollFd < 0 || m_wakeFd < 0) return false;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            PollEvent ev{};
            ev.completionKey    = completionKey;
            ev.bytesTransferred = bytesTransferred;
            ev.overlapped       = overlapped;
            ev.success          = true;
            m_customQueue.push(ev);
        }
        uint64_t one = 1;
        return ::write(m_wakeFd, &one, sizeof(one)) == sizeof(one);
    }

    bool WaitCompletion(PollEvent& outEvent, uint32_t timeoutMs) override
    {
        if (m_epollFd < 0) return false;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (!m_customQueue.empty())
            {
                outEvent = m_customQueue.front();
                m_customQueue.pop();
                return true;
            }
        }
        struct epoll_event ev{};
        int timeout = (timeoutMs == INFINITE) ? -1 : static_cast<int>(timeoutMs);
        int n = ::epoll_wait(m_epollFd, &ev, 1, timeout);
        if (n <= 0)
        {
            outEvent = PollEvent{};
            outEvent.errorCode = (n == 0) ? ETIMEDOUT : errno;
            return false;
        }
        if (ev.data.u64 == 0)
        {
            uint64_t val = 0;
            (void)::read(m_wakeFd, &val, sizeof(val));
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (!m_customQueue.empty())
            {
                outEvent = m_customQueue.front();
                m_customQueue.pop();
                return true;
            }
            return false;
        }
        outEvent.completionKey    = static_cast<uintptr_t>(ev.data.u64);
        outEvent.overlapped       = nullptr;
        outEvent.bytesTransferred = 0;
        outEvent.success          = ((ev.events & (EPOLLERR | EPOLLHUP)) == 0);
        outEvent.errorCode        = outEvent.success ? 0 : EIO;
        return true;
    }

    HANDLE GetNativeHandle() const override
    {
        return reinterpret_cast<HANDLE>(static_cast<intptr_t>(m_epollFd));
    }

private:
    int m_epollFd = -1;
    int m_wakeFd  = -1;
    std::mutex m_queueMutex;
    std::queue<PollEvent> m_customQueue;
};

std::unique_ptr<IPlatformPoller> CreatePlatformPoller()
{
    return std::unique_ptr<IPlatformPoller>(new LinuxEpollPoller());
}

} // namespace Platform
} // namespace Irine

// =============================================================================
// Global Win32 Compatibility Shims on Linux
// =============================================================================

DWORD     timeGetTime() noexcept     { return Irine::Platform::GetTickCountMs(); }
DWORD     GetTickCount() noexcept    { return Irine::Platform::GetTickCountMs(); }
ULONGLONG GetTickCount64() noexcept  { return Irine::Platform::GetTickCount64Ms(); }
void      GetLocalTime(SYSTEMTIME* lpSystemTime) noexcept  { Irine::Platform::GetLocalSystemTime(lpSystemTime); }
void      GetSystemTime(SYSTEMTIME* lpSystemTime) noexcept { Irine::Platform::GetLocalSystemTime(lpSystemTime); }
void      Sleep(DWORD dwMilliseconds) noexcept             { Irine::Platform::SleepMs(dwMilliseconds); }
DWORD     GetCurrentThreadId() noexcept                    { return Irine::Platform::GetCurrentThreadId(); }
DWORD     GetCurrentProcessId() noexcept                   { return Irine::Platform::GetCurrentProcessId(); }

BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) noexcept
{
    if (!lpPerformanceCount) return FALSE;
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    lpPerformanceCount->QuadPart = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return TRUE;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) noexcept
{
    if (!lpFrequency) return FALSE;
    lpFrequency->QuadPart = 1000000000LL;
    return TRUE;
}

void InitializeCriticalSection(LPCRITICAL_SECTION cs) noexcept
{
    if (!cs) return;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cs->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

BOOL InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION cs, DWORD /*dwSpinCount*/) noexcept
{
    InitializeCriticalSection(cs);
    return TRUE;
}

void DeleteCriticalSection(LPCRITICAL_SECTION cs) noexcept
{
    if (cs) pthread_mutex_destroy(&cs->mutex);
}

void EnterCriticalSection(LPCRITICAL_SECTION cs) noexcept
{
    if (cs) pthread_mutex_lock(&cs->mutex);
}

BOOL TryEnterCriticalSection(LPCRITICAL_SECTION cs) noexcept
{
    return (cs && pthread_mutex_trylock(&cs->mutex) == 0) ? TRUE : FALSE;
}

void LeaveCriticalSection(LPCRITICAL_SECTION cs) noexcept
{
    if (cs) pthread_mutex_unlock(&cs->mutex);
}
