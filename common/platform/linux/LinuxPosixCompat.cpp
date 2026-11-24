// =============================================================================
// IrineCore — common/platform/linux/LinuxPosixCompat.cpp
// Complete Linux/POSIX implementations for all declared shims in
// PlatformSync.h, PlatformThread.h, PlatformFileSystem.h, PlatformTime.h,
// PlatformIni.h, WideStringCompat.h, and compat/windows.h, plus global
// 2-byte wchar_t std::locale initialization.
// =============================================================================
#if !defined(__linux__)
#error "common/platform/linux/LinuxPosixCompat.cpp must only be compiled on Linux targets."
#endif

#include "compat/windows.h"
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/select.h>
#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <libgen.h>
#include <termios.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cctype>
#include <fstream>
#include <locale>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// 1. Global 2-byte wchar_t std::locale facet installation (runs before main())
// =============================================================================
namespace
{

class ShortWcharCtype : public std::ctype<wchar_t>
{
public:
    explicit ShortWcharCtype(size_t refs = 0) : std::ctype<wchar_t>(refs) {}
protected:
    static mask classify(wchar_t c)
    {
        if (static_cast<uint16_t>(c) > 0x7F)
            return static_cast<mask>(alpha | print);
        unsigned char uc = static_cast<unsigned char>(c);
        mask m = 0;
        if (std::isspace(uc))  m |= space;
        if (std::isprint(uc))  m |= print;
        if (std::iscntrl(uc))  m |= cntrl;
        if (std::isupper(uc))  m |= upper;
        if (std::islower(uc))  m |= lower;
        if (std::isalpha(uc))  m |= alpha;
        if (std::isdigit(uc))  m |= digit;
        if (std::ispunct(uc))  m |= punct;
        if (std::isxdigit(uc)) m |= xdigit;
        if (uc == ' ' || uc == '\t') m |= blank;
        // Never OR compound masks (graph = alpha|digit|punct, alnum = alpha|digit);
        // atomic bits above already match graph/alnum queries via bitwise AND.
        return m;
    }
    bool do_is(mask m, wchar_t c) const override { return (classify(c) & m) != 0; }
    const wchar_t* do_is(const wchar_t* lo, const wchar_t* hi, mask* vec) const override
    {
        for (; lo != hi; ++lo, ++vec) *vec = classify(*lo);
        return hi;
    }
    const wchar_t* do_scan_is(mask m, const wchar_t* lo, const wchar_t* hi) const override
    {
        for (; lo != hi; ++lo) if (classify(*lo) & m) return lo;
        return hi;
    }
    const wchar_t* do_scan_not(mask m, const wchar_t* lo, const wchar_t* hi) const override
    {
        for (; lo != hi; ++lo) if (!(classify(*lo) & m)) return lo;
        return hi;
    }
    wchar_t do_toupper(wchar_t c) const override { return (c >= L'a' && c <= L'z') ? static_cast<wchar_t>(c - L'a' + L'A') : c; }
    const wchar_t* do_toupper(wchar_t* lo, const wchar_t* hi) const override
    {
        for (; lo != hi; ++lo) *lo = do_toupper(*lo);
        return hi;
    }
    wchar_t do_tolower(wchar_t c) const override { return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c - L'A' + L'a') : c; }
    const wchar_t* do_tolower(wchar_t* lo, const wchar_t* hi) const override
    {
        for (; lo != hi; ++lo) *lo = do_tolower(*lo);
        return hi;
    }
    wchar_t do_widen(char c) const override { return static_cast<wchar_t>(static_cast<unsigned char>(c)); }
    const char* do_widen(const char* lo, const char* hi, wchar_t* to) const override
    {
        for (; lo != hi; ++lo, ++to) *to = static_cast<wchar_t>(static_cast<unsigned char>(*lo));
        return hi;
    }
    char do_narrow(wchar_t c, char dfault) const override
    {
        return (static_cast<uint16_t>(c) <= 0x7F) ? static_cast<char>(c) : dfault;
    }
    const wchar_t* do_narrow(const wchar_t* lo, const wchar_t* hi, char dfault, char* to) const override
    {
        for (; lo != hi; ++lo, ++to) *to = do_narrow(*lo, dfault);
        return hi;
    }
};

__attribute__((constructor))
static void InitializeShortWcharGlobalLocale()
{
    try
    {
        // Note: Do NOT subclass std::numpunct<wchar_t> here — libstdc++.so's
        // _M_install_facet / _M_initialize_numpunct expects 4-byte wchar_t in
        // do_truename()/do_falsename(). Creating an unnamed locale with our
        // 2-byte ShortWcharCtype + freshly instantiated 2-byte num_get/num_put
        // gives zero-initialized _M_caches so __numpunct_cache<wchar_t> is built
        // cleanly using 2-byte wchar_t layout.
        std::locale loc(std::locale::classic(), new ShortWcharCtype());
        loc = std::locale(loc, new std::num_get<wchar_t>());
        loc = std::locale(loc, new std::num_put<wchar_t>());
        std::locale::global(loc);
    }
    catch (...)
    {
    }
}

// =============================================================================
// 2. Unified Win32 Handle Table (Event / Thread / Mutex / Semaphore / File / Map)
// =============================================================================
enum class HandleKind : uint32_t
{
    Event = 1,
    Thread,
    Mutex,
    Semaphore,
    File,
    FileMapping,
    Iocp,
};

struct WinHandleObj
{
    static constexpr uint32_t kMagic = 0x49524E48u; // 'IRNH'
    uint32_t   magic = kMagic;
    HandleKind kind  = HandleKind::Event;
    std::atomic<int> refCount{ 1 };

    // Synchronization primitives
    std::mutex              mtx;
    std::condition_variable cv;
    bool manualReset = false;
    bool signaled    = false;
    LONG semCount    = 0;
    LONG semMax      = 1;

    // Thread data
    pthread_t threadId{};
    bool      threadJoined = false;
    bool      threadDone   = false;
    DWORD     exitCode     = 0;

    // File / FileMapping data
    int    fd       = -1;
    size_t mapSize  = 0;
    bool   mapWrite = false;

    // IOCP poller
    std::unique_ptr<Irine::Platform::IPlatformPoller> poller;
};

// Keep handle registry immortal across process shutdown so static destructors
// in server translation units (e.g. ~CThreadManager) can safely call
// WaitForSingleObject / CloseHandle after libirine_platform globals unwind.
static std::mutex& HandleRegistryMtx()
{
    static auto* mtx = new std::mutex();
    return *mtx;
}

static std::unordered_map<HANDLE, WinHandleObj*>& HandleRegistry()
{
    static auto* reg = new std::unordered_map<HANDLE, WinHandleObj*>();
    return *reg;
}

static WinHandleObj* LookupHandle(HANDLE h)
{
    if (!h || h == INVALID_HANDLE_VALUE) return nullptr;
    std::lock_guard<std::mutex> lk(HandleRegistryMtx());
    auto& reg = HandleRegistry();
    auto it = reg.find(h);
    if (it == reg.end()) return nullptr;
    WinHandleObj* obj = it->second;
    if (obj && obj->magic == WinHandleObj::kMagic)
    {
        obj->refCount.fetch_add(1, std::memory_order_relaxed);
        return obj;
    }
    return nullptr;
}

static void ReleaseHandleRef(WinHandleObj* obj)
{
    if (!obj) return;
    if (obj->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        if (obj->kind == HandleKind::File && obj->fd >= 0)
            ::close(obj->fd);
        else if (obj->kind == HandleKind::Thread && !obj->threadJoined)
            ::pthread_detach(obj->threadId);
        obj->magic = 0;
        delete obj;
    }
}

static HANDLE RegisterHandle(WinHandleObj* obj)
{
    HANDLE h = reinterpret_cast<HANDLE>(obj);
    std::lock_guard<std::mutex> lk(HandleRegistryMtx());
    HandleRegistry()[h] = obj;
    return h;
}

thread_local DWORD g_tlsLastError = 0;

struct ThreadStartCtx
{
    WinHandleObj* obj = nullptr;
    _beginthreadex_proc_type crtProc = nullptr;
    LPTHREAD_START_ROUTINE   winProc = nullptr;
    void* arg = nullptr;
    bool  suspended = false;
};

static void* PosixThreadEntry(void* raw)
{
    std::unique_ptr<ThreadStartCtx> ctx(static_cast<ThreadStartCtx*>(raw));
    WinHandleObj* obj = ctx->obj;

    if (ctx->suspended)
    {
        std::unique_lock<std::mutex> lk(obj->mtx);
        obj->cv.wait(lk, [&]() { return obj->manualReset; });
    }

    DWORD ret = 0;
    if (ctx->crtProc)
        ret = static_cast<DWORD>(ctx->crtProc(ctx->arg));
    else if (ctx->winProc)
        ret = ctx->winProc(ctx->arg);

    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        obj->exitCode   = ret;
        obj->threadDone = true;
        obj->signaled   = true;
    }
    obj->cv.notify_all();
    ReleaseHandleRef(obj);
    return nullptr;
}

struct FindHandleState
{
    DIR*        dir = nullptr;
    std::string dirPath;
    std::string pattern;
};

static std::string NormalizeLinuxPath(const char* p, bool forWrite = false)
{
    if (!p) return {};
    return Irine::Platform::ResolveCaseInsensitivePath(p, forWrite);
}

static std::string NormalizeLinuxPathW(const WCHAR* p, bool forWrite = false)
{
    if (!p) return {};
    return Irine::Platform::ResolveCaseInsensitivePath(Irine::Platform::WideToUtf8(p), forWrite);
}

static void UnixTimeToFileTime(time_t t, FILETIME* ft)
{
    if (!ft) return;
    uint64_t ticks = (static_cast<uint64_t>(t) + 11644473600ULL) * 10000000ULL;
    ft->dwLowDateTime  = static_cast<DWORD>(ticks & 0xFFFFFFFFu);
    ft->dwHighDateTime = static_cast<DWORD>(ticks >> 32);
}

static bool FillFindData(FindHandleState* st, WIN32_FIND_DATAA* outA, WIN32_FIND_DATAW* outW)
{
    if (!st || !st->dir) return false;
    while (struct dirent* ent = ::readdir(st->dir))
    {
        if (::fnmatch(st->pattern.c_str(), ent->d_name, FNM_CASEFOLD) != 0)
            continue;
        std::string full = st->dirPath.empty() ? std::string(ent->d_name) : (st->dirPath + "/" + ent->d_name);
        struct stat sb{};
        ::stat(full.c_str(), &sb);

        DWORD attrs = S_ISDIR(sb.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
        uint64_t sz = S_ISDIR(sb.st_mode) ? 0ULL : static_cast<uint64_t>(sb.st_size);
        FILETIME ftM{};
        UnixTimeToFileTime(sb.st_mtime, &ftM);

        if (outA)
        {
            std::memset(outA, 0, sizeof(*outA));
            outA->dwFileAttributes = attrs;
            outA->ftCreationTime   = ftM;
            outA->ftLastAccessTime = ftM;
            outA->ftLastWriteTime  = ftM;
            outA->nFileSizeLow     = static_cast<DWORD>(sz & 0xFFFFFFFFu);
            outA->nFileSizeHigh    = static_cast<DWORD>(sz >> 32);
            strncpy_s(outA->cFileName, MAX_PATH, ent->d_name, _TRUNCATE);
        }
        if (outW)
        {
            std::memset(outW, 0, sizeof(*outW));
            outW->dwFileAttributes = attrs;
            outW->ftCreationTime   = ftM;
            outW->ftLastAccessTime = ftM;
            outW->ftLastWriteTime  = ftM;
            outW->nFileSizeLow     = static_cast<DWORD>(sz & 0xFFFFFFFFu);
            outW->nFileSizeHigh    = static_cast<DWORD>(sz >> 32);
            std::wstring wn = Irine::Platform::Utf8ToWide(ent->d_name);
            wcsncpy_s(outW->cFileName, MAX_PATH, wn.c_str(), _TRUNCATE);
        }
        return true;
    }
    return false;
}

} // namespace

// =============================================================================
// 3. Error / System / Time
// =============================================================================
DWORD GetLastError() noexcept { return g_tlsLastError; }
void  SetLastError(DWORD dwErrCode) noexcept { g_tlsLastError = dwErrCode; }

void GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) noexcept
{
    if (!lpSystemInfo) return;
    std::memset(lpSystemInfo, 0, sizeof(*lpSystemInfo));
    lpSystemInfo->dwPageSize           = static_cast<DWORD>(::sysconf(_SC_PAGESIZE));
    lpSystemInfo->dwNumberOfProcessors = Irine::Platform::GetProcessorCount();
    lpSystemInfo->dwAllocationGranularity = lpSystemInfo->dwPageSize;
}

void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) noexcept
{
    if (!lpBuffer) return;
    struct sysinfo si{};
    ::sysinfo(&si);
    uint64_t unit = si.mem_unit ? si.mem_unit : 1;
    lpBuffer->dwLength        = sizeof(*lpBuffer);
    lpBuffer->dwTotalPhys     = static_cast<SIZE_T>(si.totalram * unit);
    lpBuffer->dwAvailPhys     = static_cast<SIZE_T>(si.freeram * unit);
    lpBuffer->dwTotalPageFile = static_cast<SIZE_T>((si.totalram + si.totalswap) * unit);
    lpBuffer->dwAvailPageFile = static_cast<SIZE_T>((si.freeram + si.freeswap) * unit);
    lpBuffer->dwTotalVirtual  = lpBuffer->dwTotalPhys;
    lpBuffer->dwAvailVirtual  = lpBuffer->dwAvailPhys;
    lpBuffer->dwMemoryLoad    = (si.totalram > 0) ? static_cast<DWORD>(((si.totalram - si.freeram) * 100ULL) / si.totalram) : 0;
}

BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer) noexcept
{
    if (!lpBuffer) return FALSE;
    struct sysinfo si{};
    ::sysinfo(&si);
    uint64_t unit = si.mem_unit ? si.mem_unit : 1;
    lpBuffer->dwLength        = sizeof(*lpBuffer);
    lpBuffer->ullTotalPhys    = static_cast<DWORD64>(si.totalram * unit);
    lpBuffer->ullAvailPhys    = static_cast<DWORD64>(si.freeram * unit);
    lpBuffer->ullTotalPageFile = static_cast<DWORD64>((si.totalram + si.totalswap) * unit);
    lpBuffer->ullAvailPageFile = static_cast<DWORD64>((si.freeram + si.freeswap) * unit);
    lpBuffer->ullTotalVirtual  = lpBuffer->ullTotalPhys;
    lpBuffer->ullAvailVirtual  = lpBuffer->ullAvailPhys;
    lpBuffer->ullAvailExtendedVirtual = 0;
    lpBuffer->dwMemoryLoad     = (si.totalram > 0) ? static_cast<DWORD>(((si.totalram - si.freeram) * 100ULL) / si.totalram) : 0;
    return TRUE;
}

BOOL SystemTimeToFileTime(const SYSTEMTIME* st, LPFILETIME ft) noexcept
{
    if (!st || !ft) return FALSE;
    struct tm t{};
    t.tm_year = st->wYear - 1900;
    t.tm_mon  = st->wMonth - 1;
    t.tm_mday = st->wDay;
    t.tm_hour = st->wHour;
    t.tm_min  = st->wMinute;
    t.tm_sec  = st->wSecond;
    time_t sec = ::timegm(&t);
    uint64_t ticks = (static_cast<uint64_t>(sec) + 11644473600ULL) * 10000000ULL + static_cast<uint64_t>(st->wMilliseconds) * 10000ULL;
    ft->dwLowDateTime  = static_cast<DWORD>(ticks & 0xFFFFFFFFu);
    ft->dwHighDateTime = static_cast<DWORD>(ticks >> 32);
    return TRUE;
}

BOOL FileTimeToSystemTime(const FILETIME* ft, LPSYSTEMTIME st) noexcept
{
    if (!ft || !st) return FALSE;
    uint64_t ticks = (static_cast<uint64_t>(ft->dwHighDateTime) << 32) | ft->dwLowDateTime;
    uint64_t ms    = (ticks / 10000ULL) % 1000ULL;
    int64_t sec    = static_cast<int64_t>(ticks / 10000000ULL) - 11644473600LL;
    time_t tt      = static_cast<time_t>(sec);
    struct tm t{};
    ::gmtime_r(&tt, &t);
    st->wYear         = static_cast<WORD>(t.tm_year + 1900);
    st->wMonth        = static_cast<WORD>(t.tm_mon + 1);
    st->wDayOfWeek    = static_cast<WORD>(t.tm_wday);
    st->wDay          = static_cast<WORD>(t.tm_mday);
    st->wHour         = static_cast<WORD>(t.tm_hour);
    st->wMinute       = static_cast<WORD>(t.tm_min);
    st->wSecond       = static_cast<WORD>(t.tm_sec);
    st->wMilliseconds = static_cast<WORD>(ms);
    return TRUE;
}

// =============================================================================
// 4. Events, Mutexes, Semaphores, Threads & Handles
// =============================================================================
HANDLE CreateEventA(void*, BOOL bManualReset, BOOL bInitialState, LPCSTR) noexcept
{
    auto* obj = new WinHandleObj();
    obj->kind        = HandleKind::Event;
    obj->manualReset = (bManualReset != FALSE);
    obj->signaled    = (bInitialState != FALSE);
    return RegisterHandle(obj);
}

HANDLE CreateEventW(void* sa, BOOL bManualReset, BOOL bInitialState, LPCWSTR) noexcept
{
    return CreateEventA(sa, bManualReset, bInitialState, nullptr);
}

BOOL SetEvent(HANDLE hEvent) noexcept
{
    WinHandleObj* obj = LookupHandle(hEvent);
    if (!obj) return FALSE;
    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        obj->signaled = true;
    }
    if (obj->manualReset) obj->cv.notify_all();
    else obj->cv.notify_one();
    ReleaseHandleRef(obj);
    return TRUE;
}

BOOL ResetEvent(HANDLE hEvent) noexcept
{
    WinHandleObj* obj = LookupHandle(hEvent);
    if (!obj) return FALSE;
    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        obj->signaled = false;
    }
    ReleaseHandleRef(obj);
    return TRUE;
}

HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES, BOOL bInitialOwner, LPCSTR) noexcept
{
    auto* obj = new WinHandleObj();
    obj->kind     = HandleKind::Mutex;
    obj->signaled = (bInitialOwner == FALSE);
    return RegisterHandle(obj);
}

HANDLE CreateMutexW(LPSECURITY_ATTRIBUTES sa, BOOL bInitialOwner, LPCWSTR) noexcept
{
    return CreateMutexA(sa, bInitialOwner, nullptr);
}

BOOL ReleaseMutex(HANDLE hMutex) noexcept
{
    WinHandleObj* obj = LookupHandle(hMutex);
    if (!obj) return FALSE;
    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        obj->signaled = true;
    }
    obj->cv.notify_one();
    ReleaseHandleRef(obj);
    return TRUE;
}

HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES, LONG lInitialCount, LONG lMaximumCount, LPCSTR) noexcept
{
    auto* obj = new WinHandleObj();
    obj->kind     = HandleKind::Semaphore;
    obj->semCount = lInitialCount;
    obj->semMax   = lMaximumCount;
    return RegisterHandle(obj);
}

HANDLE CreateSemaphoreW(LPSECURITY_ATTRIBUTES sa, LONG lInitialCount, LONG lMaximumCount, LPCWSTR) noexcept
{
    return CreateSemaphoreA(sa, lInitialCount, lMaximumCount, nullptr);
}

BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount) noexcept
{
    WinHandleObj* obj = LookupHandle(hSemaphore);
    if (!obj) return FALSE;
    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        if (lpPreviousCount) *lpPreviousCount = obj->semCount;
        obj->semCount = std::min(obj->semMax, obj->semCount + lReleaseCount);
    }
    obj->cv.notify_all();
    ReleaseHandleRef(obj);
    return TRUE;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) noexcept
{
    WinHandleObj* obj = LookupHandle(hHandle);
    if (!obj) return WAIT_FAILED;

    DWORD result = WAIT_OBJECT_0;
    std::unique_lock<std::mutex> lk(obj->mtx);

    auto isReady = [&]() -> bool {
        if (obj->kind == HandleKind::Semaphore) return obj->semCount > 0;
        if (obj->kind == HandleKind::Thread)    return obj->threadDone;
        return obj->signaled;
    };

    if (!isReady())
    {
        if (dwMilliseconds == 0)
            result = WAIT_TIMEOUT;
        else if (dwMilliseconds == INFINITE)
            obj->cv.wait(lk, isReady);
        else if (!obj->cv.wait_for(lk, std::chrono::milliseconds(dwMilliseconds), isReady))
            result = WAIT_TIMEOUT;
    }

    if (result == WAIT_OBJECT_0)
    {
        if (obj->kind == HandleKind::Event && !obj->manualReset)
            obj->signaled = false;
        else if (obj->kind == HandleKind::Mutex)
            obj->signaled = false;
        else if (obj->kind == HandleKind::Semaphore)
            obj->semCount--;
    }
    lk.unlock();
    ReleaseHandleRef(obj);
    return result;
}

DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds) noexcept
{
    if (nCount == 0 || !lpHandles) return WAIT_FAILED;
    if (bWaitAll)
    {
        for (DWORD i = 0; i < nCount; ++i)
        {
            DWORD r = WaitForSingleObject(lpHandles[i], dwMilliseconds);
            if (r != WAIT_OBJECT_0) return r;
        }
        return WAIT_OBJECT_0;
    }
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(dwMilliseconds);
    for (;;)
    {
        for (DWORD i = 0; i < nCount; ++i)
        {
            if (WaitForSingleObject(lpHandles[i], 0) == WAIT_OBJECT_0)
                return WAIT_OBJECT_0 + i;
        }
        if (dwMilliseconds == 0) return WAIT_TIMEOUT;
        if (dwMilliseconds != INFINITE && std::chrono::steady_clock::now() >= deadline)
            return WAIT_TIMEOUT;
        ::usleep(1000);
    }
}

BOOL CloseHandle(HANDLE hObject) noexcept
{
    if (!hObject || hObject == INVALID_HANDLE_VALUE) return FALSE;
    WinHandleObj* obj = nullptr;
    {
        std::lock_guard<std::mutex> lk(HandleRegistryMtx());
        auto& reg = HandleRegistry();
        auto it = reg.find(hObject);
        if (it != reg.end())
        {
            obj = it->second;
            reg.erase(it);
        }
    }
    if (!obj) return FALSE;
    ReleaseHandleRef(obj);
    return TRUE;
}

uintptr_t _beginthreadex(void*, unsigned stack_size, _beginthreadex_proc_type start_address, void* arglist, unsigned initflag, unsigned* thrdaddr) noexcept
{
    if (!start_address) return 0;
    auto* obj = new WinHandleObj();
    obj->kind        = HandleKind::Thread;
    obj->manualReset = ((initflag & CREATE_SUSPENDED) == 0);
    obj->refCount.fetch_add(1, std::memory_order_relaxed); // thread owns a ref

    auto* ctx = new ThreadStartCtx();
    ctx->obj       = obj;
    ctx->crtProc   = start_address;
    ctx->arg       = arglist;
    ctx->suspended = ((initflag & CREATE_SUSPENDED) != 0);

    pthread_attr_t attr;
    ::pthread_attr_init(&attr);
    if (stack_size >= 65536) ::pthread_attr_setstacksize(&attr, stack_size);

    int rc = ::pthread_create(&obj->threadId, &attr, PosixThreadEntry, ctx);
    ::pthread_attr_destroy(&attr);
    if (rc != 0)
    {
        delete ctx;
        delete obj;
        g_tlsLastError = static_cast<DWORD>(rc);
        return 0;
    }
    if (thrdaddr) *thrdaddr = static_cast<unsigned>(static_cast<uintptr_t>(obj->threadId) & 0xFFFFFFFFu);
    return reinterpret_cast<uintptr_t>(RegisterHandle(obj));
}

void _endthreadex(unsigned retval) noexcept
{
    ::pthread_exit(reinterpret_cast<void*>(static_cast<uintptr_t>(retval)));
}

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) noexcept
{
    if (!lpStartAddress) return nullptr;
    auto* obj = new WinHandleObj();
    obj->kind        = HandleKind::Thread;
    obj->manualReset = ((dwCreationFlags & CREATE_SUSPENDED) == 0);
    obj->refCount.fetch_add(1, std::memory_order_relaxed);

    auto* ctx = new ThreadStartCtx();
    ctx->obj       = obj;
    ctx->winProc   = lpStartAddress;
    ctx->arg       = lpParameter;
    ctx->suspended = ((dwCreationFlags & CREATE_SUSPENDED) != 0);

    pthread_attr_t attr;
    ::pthread_attr_init(&attr);
    if (dwStackSize >= 65536) ::pthread_attr_setstacksize(&attr, dwStackSize);

    int rc = ::pthread_create(&obj->threadId, &attr, PosixThreadEntry, ctx);
    ::pthread_attr_destroy(&attr);
    if (rc != 0)
    {
        delete ctx;
        delete obj;
        g_tlsLastError = static_cast<DWORD>(rc);
        return nullptr;
    }
    if (lpThreadId) *lpThreadId = static_cast<DWORD>(static_cast<uintptr_t>(obj->threadId) & 0xFFFFFFFFu);
    return RegisterHandle(obj);
}

HANDLE GetCurrentThread() noexcept { return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(::pthread_self())); }
BOOL   SetThreadPriority(HANDLE, int) noexcept { return TRUE; }
DWORD  SuspendThread(HANDLE) noexcept { return 0; }

DWORD ResumeThread(HANDLE hThread) noexcept
{
    WinHandleObj* obj = LookupHandle(hThread);
    if (!obj) return static_cast<DWORD>(-1);
    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        obj->manualReset = true;
    }
    obj->cv.notify_all();
    ReleaseHandleRef(obj);
    return 1;
}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode) noexcept
{
    WinHandleObj* obj = LookupHandle(hThread);
    if (!obj) return FALSE;
    obj->exitCode = dwExitCode;
    ::pthread_cancel(obj->threadId);
    ReleaseHandleRef(obj);
    return TRUE;
}

BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) noexcept
{
    WinHandleObj* obj = LookupHandle(hThread);
    if (!obj) return FALSE;
    if (lpExitCode)
    {
        std::lock_guard<std::mutex> lk(obj->mtx);
        *lpExitCode = obj->threadDone ? obj->exitCode : STILL_ACTIVE;
    }
    ReleaseHandleRef(obj);
    return TRUE;
}

DWORD_PTR SetThreadAffinityMask(HANDLE, DWORD_PTR dwThreadAffinityMask) noexcept
{
    return dwThreadAffinityMask;
}

// =============================================================================
// 5. Filesystem, Directories, FindFirstFile, File Handles & Memory Mapping
// =============================================================================
BOOL CreateDirectoryA(LPCSTR lpPathName, void*) noexcept
{
    if (!lpPathName) return FALSE;
    std::string p = NormalizeLinuxPath(lpPathName, true);
    if (::mkdir(p.c_str(), 0755) == 0) return TRUE;
    g_tlsLastError = (errno == EEXIST) ? ERROR_ALREADY_EXISTS : static_cast<DWORD>(errno);
    return FALSE;
}

BOOL CreateDirectoryW(LPCWSTR lpPathName, void* sa) noexcept
{
    return lpPathName ? CreateDirectoryA(Irine::Platform::WideToUtf8(lpPathName).c_str(), sa) : FALSE;
}

BOOL RemoveDirectoryA(LPCSTR lpPathName) noexcept
{
    if (!lpPathName) return FALSE;
    return ::rmdir(NormalizeLinuxPath(lpPathName).c_str()) == 0 ? TRUE : FALSE;
}

BOOL RemoveDirectoryW(LPCWSTR lpPathName) noexcept
{
    return lpPathName ? RemoveDirectoryA(Irine::Platform::WideToUtf8(lpPathName).c_str()) : FALSE;
}

DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) noexcept
{
    char tmp[PATH_MAX] = { 0 };
    if (!::getcwd(tmp, sizeof(tmp))) return 0;
    size_t len = std::strlen(tmp);
    if (!lpBuffer || nBufferLength == 0) return static_cast<DWORD>(len + 1);
    if (len >= nBufferLength) return static_cast<DWORD>(len + 1);
    std::memcpy(lpBuffer, tmp, len + 1);
    return static_cast<DWORD>(len);
}

DWORD GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer) noexcept
{
    char tmp[PATH_MAX] = { 0 };
    if (!::getcwd(tmp, sizeof(tmp))) return 0;
    std::wstring w = Irine::Platform::Utf8ToWide(tmp);
    if (!lpBuffer || nBufferLength == 0) return static_cast<DWORD>(w.size() + 1);
    if (w.size() >= nBufferLength) return static_cast<DWORD>(w.size() + 1);
    std::memcpy(lpBuffer, w.data(), (w.size() + 1) * sizeof(WCHAR));
    return static_cast<DWORD>(w.size());
}

BOOL SetCurrentDirectoryA(LPCSTR lpPathName) noexcept
{
    if (!lpPathName) return FALSE;
    return ::chdir(NormalizeLinuxPath(lpPathName).c_str()) == 0 ? TRUE : FALSE;
}

BOOL SetCurrentDirectoryW(LPCWSTR lpPathName) noexcept
{
    return lpPathName ? SetCurrentDirectoryA(Irine::Platform::WideToUtf8(lpPathName).c_str()) : FALSE;
}

DWORD GetModuleFileNameA(HMODULE, LPSTR lpFilename, DWORD nSize) noexcept
{
    if (!lpFilename || nSize == 0) return 0;
    char tmp[PATH_MAX] = { 0 };
    ssize_t len = ::readlink("/proc/self/exe", tmp, sizeof(tmp) - 1);
    if (len <= 0) return 0;
    tmp[len] = '\0';
    size_t copy = std::min(static_cast<size_t>(len), static_cast<size_t>(nSize - 1));
    std::memcpy(lpFilename, tmp, copy);
    lpFilename[copy] = '\0';
    return static_cast<DWORD>(copy);
}

DWORD GetModuleFileNameW(HMODULE hMod, LPWSTR lpFilename, DWORD nSize) noexcept
{
    if (!lpFilename || nSize == 0) return 0;
    char tmp[PATH_MAX] = { 0 };
    DWORD len = GetModuleFileNameA(hMod, tmp, sizeof(tmp));
    if (len == 0) return 0;
    std::wstring w = Irine::Platform::Utf8ToWide(tmp, static_cast<int>(len));
    size_t copy = std::min(w.size(), static_cast<size_t>(nSize - 1));
    std::memcpy(lpFilename, w.data(), copy * sizeof(WCHAR));
    lpFilename[copy] = 0;
    return static_cast<DWORD>(copy);
}

BOOL DeleteFileA(LPCSTR lpFileName) noexcept
{
    if (!lpFileName) return FALSE;
    return ::unlink(NormalizeLinuxPath(lpFileName).c_str()) == 0 ? TRUE : FALSE;
}

BOOL DeleteFileW(LPCWSTR lpFileName) noexcept
{
    return lpFileName ? DeleteFileA(Irine::Platform::WideToUtf8(lpFileName).c_str()) : FALSE;
}

BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) noexcept
{
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    return ::rename(NormalizeLinuxPath(lpExistingFileName, false).c_str(), NormalizeLinuxPath(lpNewFileName, true).c_str()) == 0 ? TRUE : FALSE;
}

BOOL MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) noexcept
{
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    return MoveFileA(Irine::Platform::WideToUtf8(lpExistingFileName).c_str(), Irine::Platform::WideToUtf8(lpNewFileName).c_str());
}

BOOL CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists) noexcept
{
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    std::string src = NormalizeLinuxPath(lpExistingFileName, false);
    std::string dst = NormalizeLinuxPath(lpNewFileName, true);
    if (bFailIfExists && ::access(dst.c_str(), F_OK) == 0) return FALSE;
    std::ifstream ifs(src, std::ios::binary);
    if (!ifs) return FALSE;
    std::ofstream ofs(dst, std::ios::binary | std::ios::trunc);
    if (!ofs) return FALSE;
    ofs << ifs.rdbuf();
    return ofs.good() ? TRUE : FALSE;
}

BOOL CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) noexcept
{
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    return CopyFileA(Irine::Platform::WideToUtf8(lpExistingFileName).c_str(), Irine::Platform::WideToUtf8(lpNewFileName).c_str(), bFailIfExists);
}

int _access(const char* path, int mode) noexcept
{
    if (!path) return -1;
    int amode = F_OK;
    if (mode & 4) amode |= R_OK;
    if (mode & 2) amode |= W_OK;
    return ::access(NormalizeLinuxPath(path).c_str(), amode);
}

int _waccess(const WCHAR* path, int mode) noexcept
{
    return path ? _access(Irine::Platform::WideToUtf8(path).c_str(), mode) : -1;
}

int _mkdir(const char* path) noexcept
{
    return path ? ::mkdir(NormalizeLinuxPath(path, true).c_str(), 0755) : -1;
}

int _wmkdir(const WCHAR* path) noexcept
{
    return path ? _mkdir(Irine::Platform::WideToUtf8(path).c_str()) : -1;
}

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD, LPSECURITY_ATTRIBUTES, DWORD dwCreationDisposition, DWORD, HANDLE) noexcept
{
    if (!lpFileName) return INVALID_HANDLE_VALUE;
    bool forWrite = ((dwDesiredAccess & GENERIC_WRITE) != 0) ||
                    (dwCreationDisposition == CREATE_NEW) ||
                    (dwCreationDisposition == CREATE_ALWAYS) ||
                    (dwCreationDisposition == OPEN_ALWAYS);
    std::string p = NormalizeLinuxPath(lpFileName, forWrite);
    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE)) flags |= O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE) flags |= O_WRONLY;
    else flags |= O_RDONLY;

    switch (dwCreationDisposition)
    {
    case CREATE_NEW:        flags |= O_CREAT | O_EXCL;  break;
    case CREATE_ALWAYS:     flags |= O_CREAT | O_TRUNC; break;
    case OPEN_ALWAYS:       flags |= O_CREAT;           break;
    case TRUNCATE_EXISTING: flags |= O_TRUNC;           break;
    case OPEN_EXISTING:
    default:                break;
    }

    int fd = ::open(p.c_str(), flags, 0644);
    if (fd < 0)
    {
        g_tlsLastError = (errno == ENOENT) ? ERROR_FILE_NOT_FOUND : static_cast<DWORD>(errno);
        return INVALID_HANDLE_VALUE;
    }
    auto* obj = new WinHandleObj();
    obj->kind = HandleKind::File;
    obj->fd   = fd;
    return RegisterHandle(obj);
}

HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES sa, DWORD dwCreationDisposition, DWORD dwFlags, HANDLE hTpl) noexcept
{
    if (!lpFileName) return INVALID_HANDLE_VALUE;
    return CreateFileA(Irine::Platform::WideToUtf8(lpFileName).c_str(), dwDesiredAccess, dwShareMode, sa, dwCreationDisposition, dwFlags, hTpl);
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File) { ReleaseHandleRef(obj); return FALSE; }
    ssize_t n = ::read(obj->fd, lpBuffer, nNumberOfBytesToRead);
    ReleaseHandleRef(obj);
    if (n < 0) { g_tlsLastError = static_cast<DWORD>(errno); return FALSE; }
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = static_cast<DWORD>(n);
    return TRUE;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File) { ReleaseHandleRef(obj); return FALSE; }
    ssize_t n = ::write(obj->fd, lpBuffer, nNumberOfBytesToWrite);
    ReleaseHandleRef(obj);
    if (n < 0) { g_tlsLastError = static_cast<DWORD>(errno); return FALSE; }
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = static_cast<DWORD>(n);
    return TRUE;
}

DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File) { ReleaseHandleRef(obj); return INVALID_FILE_SIZE; }
    struct stat sb{};
    int rc = ::fstat(obj->fd, &sb);
    ReleaseHandleRef(obj);
    if (rc != 0) return INVALID_FILE_SIZE;
    uint64_t sz = static_cast<uint64_t>(sb.st_size);
    if (lpFileSizeHigh) *lpFileSizeHigh = static_cast<DWORD>(sz >> 32);
    return static_cast<DWORD>(sz & 0xFFFFFFFFu);
}

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) noexcept
{
    if (!lpFileSize) return FALSE;
    DWORD hi = 0;
    DWORD lo = GetFileSize(hFile, &hi);
    if (lo == INVALID_FILE_SIZE) return FALSE;
    lpFileSize->QuadPart = (static_cast<LONGLONG>(hi) << 32) | lo;
    return TRUE;
}

DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File) { ReleaseHandleRef(obj); return INVALID_SET_FILE_POINTER; }
    int whence = (dwMoveMethod == FILE_CURRENT) ? SEEK_CUR : ((dwMoveMethod == FILE_END) ? SEEK_END : SEEK_SET);
    off_t offset = lDistanceToMove;
    if (lpDistanceToMoveHigh)
        offset = (static_cast<off_t>(*lpDistanceToMoveHigh) << 32) | static_cast<uint32_t>(lDistanceToMove);
    off_t res = ::lseek(obj->fd, offset, whence);
    ReleaseHandleRef(obj);
    if (res == (off_t)-1) return INVALID_SET_FILE_POINTER;
    if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = static_cast<LONG>(res >> 32);
    return static_cast<DWORD>(res & 0xFFFFFFFFu);
}

BOOL FlushFileBuffers(HANDLE hFile) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File) { ReleaseHandleRef(obj); return FALSE; }
    int rc = ::fsync(obj->fd);
    ReleaseHandleRef(obj);
    return rc == 0 ? TRUE : FALSE;
}

BOOL SetEndOfFile(HANDLE hFile) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File) { ReleaseHandleRef(obj); return FALSE; }
    off_t cur = ::lseek(obj->fd, 0, SEEK_CUR);
    int rc = (cur >= 0) ? ::ftruncate(obj->fd, cur) : -1;
    ReleaseHandleRef(obj);
    return rc == 0 ? TRUE : FALSE;
}

DWORD GetFileAttributesA(LPCSTR lpFileName) noexcept
{
    if (!lpFileName) return INVALID_FILE_ATTRIBUTES;
    struct stat sb{};
    if (::stat(NormalizeLinuxPath(lpFileName).c_str(), &sb) != 0)
        return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(sb.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}

DWORD GetFileAttributesW(LPCWSTR lpFileName) noexcept
{
    return lpFileName ? GetFileAttributesA(Irine::Platform::WideToUtf8(lpFileName).c_str()) : INVALID_FILE_ATTRIBUTES;
}

BOOL SetFileAttributesA(LPCSTR, DWORD) noexcept { return TRUE; }
BOOL SetFileAttributesW(LPCWSTR, DWORD) noexcept { return TRUE; }

HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) noexcept
{
    if (!lpFileName || !lpFindFileData) return INVALID_HANDLE_VALUE;
    std::string norm = Irine::Platform::NormalizePath(lpFileName);
    size_t slash = norm.find_last_of('/');
    std::string rawDir  = (slash == std::string::npos) ? "." : (slash == 0 ? "/" : norm.substr(0, slash));
    std::string dirPath = Irine::Platform::ResolveCaseInsensitivePath(rawDir, false);
    std::string pattern = (slash == std::string::npos) ? norm : norm.substr(slash + 1);
    if (pattern == "*.*") pattern = "*";

    DIR* d = ::opendir(dirPath.c_str());
    if (!d)
    {
        g_tlsLastError = ERROR_FILE_NOT_FOUND;
        return INVALID_HANDLE_VALUE;
    }
    auto* st = new FindHandleState{ d, dirPath, pattern };
    if (!FillFindData(st, lpFindFileData, nullptr))
    {
        ::closedir(d);
        delete st;
        g_tlsLastError = ERROR_FILE_NOT_FOUND;
        return INVALID_HANDLE_VALUE;
    }
    return reinterpret_cast<HANDLE>(st);
}

HANDLE FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData) noexcept
{
    if (!lpFileName || !lpFindFileData) return INVALID_HANDLE_VALUE;
    std::string norm = Irine::Platform::NormalizePath(Irine::Platform::WideToUtf8(lpFileName));
    size_t slash = norm.find_last_of('/');
    std::string rawDir  = (slash == std::string::npos) ? "." : (slash == 0 ? "/" : norm.substr(0, slash));
    std::string dirPath = Irine::Platform::ResolveCaseInsensitivePath(rawDir, false);
    std::string pattern = (slash == std::string::npos) ? norm : norm.substr(slash + 1);
    if (pattern == "*.*") pattern = "*";

    DIR* d = ::opendir(dirPath.c_str());
    if (!d)
    {
        g_tlsLastError = ERROR_FILE_NOT_FOUND;
        return INVALID_HANDLE_VALUE;
    }
    auto* st = new FindHandleState{ d, dirPath, pattern };
    if (!FillFindData(st, nullptr, lpFindFileData))
    {
        ::closedir(d);
        delete st;
        g_tlsLastError = ERROR_FILE_NOT_FOUND;
        return INVALID_HANDLE_VALUE;
    }
    return reinterpret_cast<HANDLE>(st);
}

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) noexcept
{
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE || !lpFindFileData) return FALSE;
    return FillFindData(reinterpret_cast<FindHandleState*>(hFindFile), lpFindFileData, nullptr) ? TRUE : FALSE;
}

BOOL FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData) noexcept
{
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE || !lpFindFileData) return FALSE;
    return FillFindData(reinterpret_cast<FindHandleState*>(hFindFile), nullptr, lpFindFileData) ? TRUE : FALSE;
}

BOOL FindClose(HANDLE hFindFile) noexcept
{
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE) return FALSE;
    auto* st = reinterpret_cast<FindHandleState*>(hFindFile);
    if (st->dir) ::closedir(st->dir);
    delete st;
    return TRUE;
}

HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES, DWORD flProtect, DWORD dwMaxHi, DWORD dwMaxLo, LPCSTR) noexcept
{
    size_t sz = (static_cast<size_t>(dwMaxHi) << 32) | dwMaxLo;
    int dupFd = -1;
    if (hFile && hFile != INVALID_HANDLE_VALUE)
    {
        WinHandleObj* fobj = LookupHandle(hFile);
        if (!fobj || fobj->kind != HandleKind::File) { ReleaseHandleRef(fobj); return nullptr; }
        dupFd = ::dup(fobj->fd);
        if (sz == 0)
        {
            struct stat sb{};
            if (::fstat(dupFd, &sb) == 0) sz = static_cast<size_t>(sb.st_size);
        }
        ReleaseHandleRef(fobj);
    }
    auto* obj = new WinHandleObj();
    obj->kind     = HandleKind::FileMapping;
    obj->fd       = dupFd;
    obj->mapSize  = sz;
    obj->mapWrite = (flProtect == PAGE_READWRITE);
    return RegisterHandle(obj);
}

HANDLE CreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES sa, DWORD flProtect, DWORD dwMaxHi, DWORD dwMaxLo, LPCWSTR) noexcept
{
    return CreateFileMappingA(hFile, sa, flProtect, dwMaxHi, dwMaxLo, nullptr);
}

static std::mutex& MmapMtx()
{
    static auto* mtx = new std::mutex();
    return *mtx;
}

static std::unordered_map<const void*, size_t>& MmapSizes()
{
    static auto* m = new std::unordered_map<const void*, size_t>();
    return *m;
}

LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwOffHi, DWORD dwOffLo, SIZE_T dwNumberOfBytesToMap) noexcept
{
    WinHandleObj* obj = LookupHandle(hFileMappingObject);
    if (!obj || obj->kind != HandleKind::FileMapping) { ReleaseHandleRef(obj); return nullptr; }
    size_t len = dwNumberOfBytesToMap ? dwNumberOfBytesToMap : obj->mapSize;
    if (len == 0) { ReleaseHandleRef(obj); return nullptr; }
    off_t off = (static_cast<off_t>(dwOffHi) << 32) | dwOffLo;
    int prot = PROT_READ | ((dwDesiredAccess & FILE_MAP_WRITE) ? PROT_WRITE : 0);
    int flags = (obj->fd >= 0) ? MAP_SHARED : (MAP_PRIVATE | MAP_ANONYMOUS);
    void* ptr = ::mmap(nullptr, len, prot, flags, obj->fd, off);
    ReleaseHandleRef(obj);
    if (ptr == MAP_FAILED) return nullptr;
    {
        std::lock_guard<std::mutex> lk(MmapMtx());
        MmapSizes()[ptr] = len;
    }
    return ptr;
}

BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) noexcept
{
    if (!lpBaseAddress) return FALSE;
    size_t len = 0;
    {
        std::lock_guard<std::mutex> lk(MmapMtx());
        auto& m = MmapSizes();
        auto it = m.find(lpBaseAddress);
        if (it == m.end()) return FALSE;
        len = it->second;
        m.erase(it);
    }
    return ::munmap(const_cast<void*>(lpBaseAddress), len) == 0 ? TRUE : FALSE;
}

// =============================================================================
// 6. INI ANSI wrappers & Dynamic Loading / Diagnostics / Console
// =============================================================================
DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName) noexcept
{
    if (!lpReturnedString || nSize == 0) return 0;
    std::wstring wApp = lpAppName ? Irine::Platform::Utf8ToWide(lpAppName) : std::wstring();
    std::wstring wKey = lpKeyName ? Irine::Platform::Utf8ToWide(lpKeyName) : std::wstring();
    std::wstring wDef = lpDefault ? Irine::Platform::Utf8ToWide(lpDefault) : std::wstring();
    std::wstring wFile = lpFileName ? Irine::Platform::Utf8ToWide(lpFileName) : std::wstring();
    std::vector<WCHAR> wOut(nSize + 1, 0);
    DWORD n = GetPrivateProfileStringW(
        lpAppName ? wApp.c_str() : nullptr,
        lpKeyName ? wKey.c_str() : nullptr,
        lpDefault ? wDef.c_str() : nullptr,
        wOut.data(), nSize,
        lpFileName ? wFile.c_str() : nullptr);
    std::string u8 = Irine::Platform::WideToUtf8(wOut.data(), static_cast<int>(n));
    size_t copy = std::min(u8.size(), static_cast<size_t>(nSize - 1));
    std::memcpy(lpReturnedString, u8.data(), copy);
    lpReturnedString[copy] = '\0';
    return static_cast<DWORD>(copy);
}

UINT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName) noexcept
{
    char buf[64] = { 0 };
    DWORD n = GetPrivateProfileStringA(lpAppName, lpKeyName, "", buf, sizeof(buf), lpFileName);
    if (n == 0 || buf[0] == '\0') return static_cast<UINT>(nDefault);
    return static_cast<UINT>(std::atoi(buf));
}

BOOL WritePrivateProfileStringA(LPCSTR, LPCSTR, LPCSTR, LPCSTR) noexcept { return TRUE; }
BOOL WritePrivateProfileStringW(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR) noexcept { return TRUE; }

HMODULE LoadLibraryA(LPCSTR lpLibFileName) noexcept { return lpLibFileName ? ::dlopen(lpLibFileName, RTLD_LAZY) : nullptr; }
HMODULE LoadLibraryW(LPCWSTR lpLibFileName) noexcept { return lpLibFileName ? LoadLibraryA(Irine::Platform::WideToUtf8(lpLibFileName).c_str()) : nullptr; }
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) noexcept { return (hModule && lpProcName) ? ::dlsym(hModule, lpProcName) : nullptr; }
BOOL    FreeLibrary(HMODULE hLibModule) noexcept { return hLibModule ? (::dlclose(hLibModule) == 0 ? TRUE : FALSE) : FALSE; }
HMODULE GetModuleHandleA(LPCSTR lpModuleName) noexcept { return lpModuleName ? ::dlopen(lpModuleName, RTLD_NOLOAD | RTLD_LAZY) : ::dlopen(nullptr, RTLD_LAZY); }
HMODULE GetModuleHandleW(LPCWSTR lpModuleName) noexcept { return lpModuleName ? GetModuleHandleA(Irine::Platform::WideToUtf8(lpModuleName).c_str()) : GetModuleHandleA(nullptr); }

void OutputDebugStringA(LPCSTR lpOutputString) noexcept { if (lpOutputString) std::fputs(lpOutputString, stderr); }
void OutputDebugStringW(LPCWSTR lpOutputString) noexcept { if (lpOutputString) std::fputs(Irine::Platform::WideToUtf8(lpOutputString).c_str(), stderr); }
int  MessageBoxA(HWND, LPCSTR lpText, LPCSTR lpCaption, UINT) noexcept { std::fprintf(stderr, "[%s] %s\n", lpCaption ? lpCaption : "IrineCore", lpText ? lpText : ""); return IDOK; }
int  MessageBoxW(HWND h, LPCWSTR lpText, LPCWSTR lpCaption, UINT u) noexcept { return MessageBoxA(h, lpText ? Irine::Platform::WideToUtf8(lpText).c_str() : "", lpCaption ? Irine::Platform::WideToUtf8(lpCaption).c_str() : "IrineCore", u); }

DWORD FormatMessageA(DWORD, LPCVOID, DWORD dwMessageId, DWORD, LPSTR lpBuffer, DWORD nSize, va_list*) noexcept
{
    if (!lpBuffer || nSize == 0) return 0;
    const char* msg = ::strerror(static_cast<int>(dwMessageId));
    if (!msg) msg = "Unknown error";
    size_t len = std::min(std::strlen(msg), static_cast<size_t>(nSize - 1));
    std::memcpy(lpBuffer, msg, len);
    lpBuffer[len] = '\0';
    return static_cast<DWORD>(len);
}

DWORD FormatMessageW(DWORD flags, LPCVOID src, DWORD dwMessageId, DWORD lang, LPWSTR lpBuffer, DWORD nSize, va_list* args) noexcept
{
    if (!lpBuffer || nSize == 0) return 0;
    char tmp[512] = { 0 };
    DWORD n = FormatMessageA(flags, src, dwMessageId, lang, tmp, sizeof(tmp), args);
    std::wstring w = Irine::Platform::Utf8ToWide(tmp, static_cast<int>(n));
    size_t len = std::min(w.size(), static_cast<size_t>(nSize - 1));
    std::memcpy(lpBuffer, w.data(), len * sizeof(WCHAR));
    lpBuffer[len] = 0;
    return static_cast<DWORD>(len);
}

BOOL GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize) noexcept
{
    if (!lpBuffer || !nSize || *nSize == 0) return FALSE;
    if (::gethostname(lpBuffer, *nSize) != 0) return FALSE;
    lpBuffer[*nSize - 1] = '\0';
    *nSize = static_cast<DWORD>(std::strlen(lpBuffer));
    return TRUE;
}

BOOL GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize) noexcept
{
    if (!lpBuffer || !nSize || *nSize == 0) return FALSE;
    char tmp[256] = { 0 };
    DWORD sz = sizeof(tmp);
    if (!GetComputerNameA(tmp, &sz)) return FALSE;
    std::wstring w = Irine::Platform::Utf8ToWide(tmp, static_cast<int>(sz));
    size_t len = std::min(w.size(), static_cast<size_t>(*nSize - 1));
    std::memcpy(lpBuffer, w.data(), len * sizeof(WCHAR));
    lpBuffer[len] = 0;
    *nSize = static_cast<DWORD>(len);
    return TRUE;
}

BOOL   SetConsoleTitleA(LPCSTR) noexcept { return TRUE; }
BOOL   SetConsoleTitleW(LPCWSTR) noexcept { return TRUE; }
HANDLE GetStdHandle(DWORD nStdHandle) noexcept { return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(nStdHandle)); }
BOOL   SetConsoleTextAttribute(HANDLE, WORD) noexcept { return TRUE; }
BOOL   GetConsoleScreenBufferInfo(HANDLE, CONSOLE_SCREEN_BUFFER_INFO* info) noexcept { if (info) std::memset(info, 0, sizeof(*info)); return TRUE; }
BOOL   IsDebuggerPresent() noexcept { return FALSE; }
void   DebugBreak() noexcept {}
HANDLE GetCurrentProcess() noexcept { return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(::getpid())); }
BOOL   GetVersionExA(OSVERSIONINFOA* v) noexcept { if (v) { v->dwMajorVersion = 6; v->dwMinorVersion = 2; } return TRUE; }
BOOL   GetVersionExW(OSVERSIONINFOW* v) noexcept { if (v) { v->dwMajorVersion = 6; v->dwMinorVersion = 2; } return TRUE; }
HLOCAL LocalFree(HLOCAL hMem) noexcept { std::free(hMem); return nullptr; }
LPVOID VirtualAlloc(LPVOID, SIZE_T dwSize, DWORD, DWORD) noexcept { return std::calloc(1, dwSize); }
BOOL   VirtualFree(LPVOID lpAddress, SIZE_T, DWORD) noexcept { std::free(lpAddress); return TRUE; }

// =============================================================================
// 7. WinSock / IOCP Shims
// =============================================================================
int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) noexcept
{
    if (lpWSAData)
    {
        std::memset(lpWSAData, 0, sizeof(*lpWSAData));
        lpWSAData->wVersion     = wVersionRequested;
        lpWSAData->wHighVersion = MAKEWORD(2, 2);
    }
    return 0;
}

int WSACleanup() noexcept { return 0; }

int ioctlsocket(SOCKET s, long cmd, unsigned long* argp) noexcept
{
    if (cmd == FIONBIO && argp)
    {
        int flags = ::fcntl(s, F_GETFL, 0);
        if (flags < 0) return SOCKET_ERROR;
        if (*argp) flags |= O_NONBLOCK;
        else flags &= ~O_NONBLOCK;
        return ::fcntl(s, F_SETFL, flags) == 0 ? 0 : SOCKET_ERROR;
    }
    return ::ioctl(s, static_cast<unsigned long>(cmd), argp);
}

SOCKET WSASocketA(int af, int type, int protocol, void*, unsigned int, DWORD) noexcept
{
    return ::socket(af, type, protocol);
}

SOCKET WSASocketW(int af, int type, int protocol, void* pi, unsigned int g, DWORD flags) noexcept
{
    return WSASocketA(af, type, protocol, pi, g, flags);
}

int WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD, LPWSAOVERLAPPED, void*) noexcept
{
    if (!lpBuffers || dwBufferCount == 0) return SOCKET_ERROR;
    ssize_t total = 0;
    for (DWORD i = 0; i < dwBufferCount; ++i)
    {
        ssize_t n = ::send(s, lpBuffers[i].buf, lpBuffers[i].len, MSG_NOSIGNAL);
        if (n < 0) return SOCKET_ERROR;
        total += n;
    }
    if (lpNumberOfBytesSent) *lpNumberOfBytesSent = static_cast<DWORD>(total);
    return 0;
}

int WSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD, LPWSAOVERLAPPED, void*) noexcept
{
    if (!lpBuffers || dwBufferCount == 0) return SOCKET_ERROR;
    ssize_t n = ::recv(s, lpBuffers[0].buf, lpBuffers[0].len, 0);
    if (n < 0) return SOCKET_ERROR;
    if (lpNumberOfBytesRecvd) *lpNumberOfBytesRecvd = static_cast<DWORD>(n);
    return 0;
}

HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads) noexcept
{
    if (!ExistingCompletionPort)
    {
        auto* obj = new WinHandleObj();
        obj->kind   = HandleKind::Iocp;
        obj->poller = Irine::Platform::CreatePlatformPoller();
        if (!obj->poller || !obj->poller->Initialize(NumberOfConcurrentThreads))
        {
            delete obj;
            return nullptr;
        }
        return RegisterHandle(obj);
    }
    WinHandleObj* obj = LookupHandle(ExistingCompletionPort);
    if (!obj || obj->kind != HandleKind::Iocp || !obj->poller) { ReleaseHandleRef(obj); return nullptr; }
    SOCKET sock = static_cast<SOCKET>(reinterpret_cast<intptr_t>(FileHandle));
    bool ok = obj->poller->AssociateSocket(sock, static_cast<uintptr_t>(CompletionKey));
    ReleaseHandleRef(obj);
    return ok ? ExistingCompletionPort : nullptr;
}

BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytesTransferred, ULONG_PTR* lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds) noexcept
{
    WinHandleObj* obj = LookupHandle(CompletionPort);
    if (!obj || obj->kind != HandleKind::Iocp || !obj->poller) { ReleaseHandleRef(obj); return FALSE; }
    Irine::Platform::PollEvent ev{};
    bool ok = obj->poller->WaitCompletion(ev, dwMilliseconds);
    ReleaseHandleRef(obj);
    if (!ok) return FALSE;
    if (lpNumberOfBytesTransferred) *lpNumberOfBytesTransferred = ev.bytesTransferred;
    if (lpCompletionKey)            *lpCompletionKey            = static_cast<ULONG_PTR>(ev.completionKey);
    if (lpOverlapped)               *lpOverlapped               = reinterpret_cast<LPOVERLAPPED>(ev.overlapped);
    return ev.success ? TRUE : FALSE;
}

BOOL PostQueuedCompletionStatus(HANDLE CompletionPort, DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped) noexcept
{
    WinHandleObj* obj = LookupHandle(CompletionPort);
    if (!obj || obj->kind != HandleKind::Iocp || !obj->poller) { ReleaseHandleRef(obj); return FALSE; }
    bool ok = obj->poller->PostCompletion(static_cast<uintptr_t>(dwCompletionKey), dwNumberOfBytesTransferred, lpOverlapped);
    ReleaseHandleRef(obj);
    return ok ? TRUE : FALSE;
}

// =============================================================================
// 8. CRT String / Format / Numeric / File / Path Shims
// =============================================================================
static char* IntToStrSigned(int64_t val, char* buf, int radix)
{
    if (!buf || radix < 2 || radix > 36) return buf;
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char tmp[72];
    int i = 0;
    bool neg = (val < 0 && radix == 10);
    uint64_t uval = neg ? static_cast<uint64_t>(-val) : static_cast<uint64_t>(val);
    if (uval == 0) tmp[i++] = '0';
    else while (uval > 0) { tmp[i++] = digits[uval % static_cast<uint64_t>(radix)]; uval /= static_cast<uint64_t>(radix); }
    int p = 0;
    if (neg) buf[p++] = '-';
    while (i > 0) buf[p++] = tmp[--i];
    buf[p] = '\0';
    return buf;
}

static char* IntToStrUnsigned(uint64_t uval, char* buf, int radix)
{
    if (!buf || radix < 2 || radix > 36) return buf;
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char tmp[72];
    int i = 0;
    if (uval == 0) tmp[i++] = '0';
    else while (uval > 0) { tmp[i++] = digits[uval % static_cast<uint64_t>(radix)]; uval /= static_cast<uint64_t>(radix); }
    int p = 0;
    while (i > 0) buf[p++] = tmp[--i];
    buf[p] = '\0';
    return buf;
}

char* _itoa(int val, char* buf, int radix)         { return IntToStrSigned(val, buf, radix); }
char* _i64toa(int64_t val, char* buf, int radix) noexcept { return IntToStrSigned(val, buf, radix); }
char* _ui64toa(uint64_t val, char* buf, int radix) noexcept { return IntToStrUnsigned(val, buf, radix); }

static WCHAR* AsciiToWBuf(const char* src, WCHAR* dst)
{
    if (!dst) return nullptr;
    size_t i = 0;
    for (; src[i] != '\0'; ++i) dst[i] = static_cast<WCHAR>(static_cast<unsigned char>(src[i]));
    dst[i] = 0;
    return dst;
}

WCHAR* _itow(int val, WCHAR* buf, int radix)           { char t[72]; IntToStrSigned(val, t, radix); return AsciiToWBuf(t, buf); }
WCHAR* _ltow(long val, WCHAR* buf, int radix)          { char t[72]; IntToStrSigned(val, t, radix); return AsciiToWBuf(t, buf); }
WCHAR* _ultow(unsigned long val, WCHAR* buf, int radix){ char t[72]; IntToStrUnsigned(val, t, radix); return AsciiToWBuf(t, buf); }
WCHAR* _i64tow(int64_t val, WCHAR* buf, int radix)     { char t[72]; IntToStrSigned(val, t, radix); return AsciiToWBuf(t, buf); }
WCHAR* _ui64tow(uint64_t val, WCHAR* buf, int radix)   { char t[72]; IntToStrUnsigned(val, t, radix); return AsciiToWBuf(t, buf); }

errno_t _itoa_s(int val, char* buf, size_t size, int radix) noexcept { char t[72]; IntToStrSigned(val, t, radix); return strcpy_s(buf, size, t); }
errno_t _itow_s(int val, WCHAR* buf, size_t size, int radix) noexcept { WCHAR t[72]; _itow(val, t, radix); return wcscpy_s(buf, size, t); }
errno_t _i64toa_s(int64_t val, char* buf, size_t size, int radix) noexcept { char t[72]; IntToStrSigned(val, t, radix); return strcpy_s(buf, size, t); }
errno_t _i64tow_s(int64_t val, WCHAR* buf, size_t size, int radix) noexcept { WCHAR t[72]; _i64tow(val, t, radix); return wcscpy_s(buf, size, t); }
errno_t _ui64toa_s(uint64_t val, char* buf, size_t size, int radix) noexcept { char t[72]; IntToStrUnsigned(val, t, radix); return strcpy_s(buf, size, t); }
errno_t _ui64tow_s(uint64_t val, WCHAR* buf, size_t size, int radix) noexcept { WCHAR t[72]; _ui64tow(val, t, radix); return wcscpy_s(buf, size, t); }

char*  _strupr(char* str)  { if (str) for (char* p = str; *p; ++p) *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p))); return str; }
char*  _strlwr(char* str)  { if (str) for (char* p = str; *p; ++p) *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p))); return str; }
WCHAR* _wcsupr(WCHAR* str) { if (str) for (WCHAR* p = str; *p; ++p) if (*p >= L'a' && *p <= L'z') *p = static_cast<WCHAR>(*p - L'a' + L'A'); return str; }
WCHAR* _wcslwr(WCHAR* str) { if (str) for (WCHAR* p = str; *p; ++p) if (*p >= L'A' && *p <= L'Z') *p = static_cast<WCHAR>(*p - L'A' + L'a'); return str; }
errno_t _strlwr_s(char* str, size_t) noexcept { if (!str) return EINVAL; _strlwr(str); return 0; }
errno_t _strupr_s(char* str, size_t) noexcept { if (!str) return EINVAL; _strupr(str); return 0; }
errno_t _wcslwr_s(WCHAR* str, size_t) noexcept { if (!str) return EINVAL; _wcslwr(str); return 0; }
errno_t _wcsupr_s(WCHAR* str, size_t) noexcept { if (!str) return EINVAL; _wcsupr(str); return 0; }

FILE* _wfopen(const WCHAR* filename, const WCHAR* mode) noexcept
{
    if (!filename || !mode) return nullptr;
    std::string m = Irine::Platform::WideToUtf8(mode);
    // Strip Windows-specific fopen flags like 't' or ', ccs=...'
    std::string cleanMode;
    for (char c : m)
    {
        if (c == ',' || c == ' ') break;
        if (c == 'r' || c == 'w' || c == 'a' || c == '+' || c == 'b') cleanMode.push_back(c);
    }
    if (cleanMode.empty()) cleanMode = "r";
    bool forWrite = (cleanMode.find('w') != std::string::npos ||
                     cleanMode.find('a') != std::string::npos ||
                     cleanMode.find('+') != std::string::npos);
    std::string p = NormalizeLinuxPathW(filename, forWrite);
    return ::fopen(p.c_str(), cleanMode.c_str());
}

errno_t fopen_s(FILE** pFile, const char* filename, const char* mode)
{
    if (!pFile || !filename || !mode) return EINVAL;
    std::string cleanMode;
    for (const char* c = mode; *c && *c != ',' && *c != ' '; ++c)
    {
        if (*c == 'r' || *c == 'w' || *c == 'a' || *c == '+' || *c == 'b') cleanMode.push_back(*c);
    }
    if (cleanMode.empty()) cleanMode = "r";
    bool forWrite = (cleanMode.find('w') != std::string::npos ||
                     cleanMode.find('a') != std::string::npos ||
                     cleanMode.find('+') != std::string::npos);
    std::string p = NormalizeLinuxPath(filename, forWrite);
    *pFile = ::fopen(p.c_str(), cleanMode.c_str());
    return (*pFile != nullptr) ? 0 : (errno ? errno : ENOENT);
}

errno_t _wfopen_s(FILE** pFile, const WCHAR* filename, const WCHAR* mode)
{
    if (!pFile || !filename || !mode) return EINVAL;
    *pFile = _wfopen(filename, mode);
    return (*pFile != nullptr) ? 0 : (errno ? errno : ENOENT);
}

errno_t localtime_s(struct tm* _Tm, const time_t* _Time)
{
    if (!_Tm || !_Time) return EINVAL;
    return ::localtime_r(_Time, _Tm) ? 0 : EINVAL;
}

int _vsnprintf(char* buffer, size_t count, const char* format, va_list argptr) noexcept
{
    return ::vsnprintf(buffer, count, format, argptr);
}

int _snprintf(char* buffer, size_t count, const char* format, ...) noexcept
{
    va_list args;
    va_start(args, format);
    int r = ::vsnprintf(buffer, count, format, args);
    va_end(args);
    return r;
}

int _vsnprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, va_list argptr)
{
    if (!buffer || sizeOfBuffer == 0 || !format) return -1;
    size_t maxWrite = (count == _TRUNCATE) ? sizeOfBuffer : std::min(sizeOfBuffer, count + 1);
    int r = ::vsnprintf(buffer, maxWrite, format, argptr);
    buffer[sizeOfBuffer - 1] = '\0';
    return r;
}

int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnprintf_s(buffer, sizeOfBuffer, count, format, args);
    va_end(args);
    return r;
}

int sprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, args);
    va_end(args);
    return r;
}

int _vsnwprintf(WCHAR* buffer, size_t count, const WCHAR* format, va_list argptr)
{
    return ::vswprintf(buffer, count, format, argptr);
}

int _vsnwprintf_s(WCHAR* buffer, size_t sizeOfBuffer, size_t count, const WCHAR* format, va_list argptr)
{
    if (!buffer || sizeOfBuffer == 0 || !format) return -1;
    size_t maxWrite = (count == _TRUNCATE) ? sizeOfBuffer : std::min(sizeOfBuffer, count + 1);
    int r = ::vswprintf(buffer, maxWrite, format, argptr);
    buffer[sizeOfBuffer - 1] = 0;
    return r;
}

int _snwprintf(WCHAR* buffer, size_t count, const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf(buffer, count, format, args);
    va_end(args);
    return r;
}

int _snwprintf_s(WCHAR* buffer, size_t sizeOfBuffer, size_t count, const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf_s(buffer, sizeOfBuffer, count, format, args);
    va_end(args);
    return r;
}

int swprintf_s(WCHAR* buffer, size_t sizeOfBuffer, const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, args);
    va_end(args);
    return r;
}

int _vscprintf(const char* format, va_list argptr) noexcept
{
    va_list cp;
    va_copy(cp, argptr);
    int n = ::vsnprintf(nullptr, 0, format, cp);
    va_end(cp);
    return n;
}

int _vscwprintf(const WCHAR* format, va_list argptr) noexcept
{
    va_list cp;
    va_copy(cp, argptr);
    int n = ::vswprintf(nullptr, 0, format, cp);
    va_end(cp);
    return n;
}

int _scprintf(const char* format, ...) noexcept
{
    va_list args;
    va_start(args, format);
    int n = _vscprintf(format, args);
    va_end(args);
    return n;
}

int _scwprintf(const WCHAR* format, ...) noexcept
{
    va_list args;
    va_start(args, format);
    int n = _vscwprintf(format, args);
    va_end(args);
    return n;
}

errno_t _splitpath_s(const char* path, char* drive, size_t driveLen, char* dir, size_t dirLen, char* fname, size_t fnameLen, char* ext, size_t extLen) noexcept
{
    if (drive && driveLen) drive[0] = '\0';
    if (dir && dirLen) dir[0] = '\0';
    if (fname && fnameLen) fname[0] = '\0';
    if (ext && extLen) ext[0] = '\0';
    if (!path) return EINVAL;

    std::string p = NormalizeLinuxPath(path);
    size_t pos = 0;
    if (p.size() >= 2 && p[1] == ':')
    {
        if (drive && driveLen) strncpy_s(drive, driveLen, p.substr(0, 2).c_str(), _TRUNCATE);
        pos = 2;
    }
    size_t slash = p.find_last_of('/');
    std::string dirStr, baseStr;
    if (slash != std::string::npos && slash >= pos)
    {
        dirStr  = p.substr(pos, slash - pos + 1);
        baseStr = p.substr(slash + 1);
    }
    else
    {
        baseStr = p.substr(pos);
    }
    if (dir && dirLen) strncpy_s(dir, dirLen, dirStr.c_str(), _TRUNCATE);

    size_t dot = baseStr.find_last_of('.');
    if (dot != std::string::npos)
    {
        if (fname && fnameLen) strncpy_s(fname, fnameLen, baseStr.substr(0, dot).c_str(), _TRUNCATE);
        if (ext && extLen) strncpy_s(ext, extLen, baseStr.substr(dot).c_str(), _TRUNCATE);
    }
    else
    {
        if (fname && fnameLen) strncpy_s(fname, fnameLen, baseStr.c_str(), _TRUNCATE);
    }
    return 0;
}

int _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext) noexcept
{
    return _splitpath_s(path, drive, drive ? _MAX_DRIVE : 0, dir, dir ? _MAX_DIR : 0, fname, fname ? _MAX_FNAME : 0, ext, ext ? _MAX_EXT : 0);
}

errno_t _wsplitpath_s(const WCHAR* path, WCHAR* drive, size_t driveLen, WCHAR* dir, size_t dirLen, WCHAR* fname, size_t fnameLen, WCHAR* ext, size_t extLen) noexcept
{
    char d[_MAX_DRIVE]{}, dr[_MAX_DIR]{}, fn[_MAX_FNAME]{}, ex[_MAX_EXT]{};
    errno_t rc = _splitpath_s(path ? Irine::Platform::WideToUtf8(path).c_str() : nullptr, d, sizeof(d), dr, sizeof(dr), fn, sizeof(fn), ex, sizeof(ex));
    if (drive && driveLen) wcsncpy_s(drive, driveLen, Irine::Platform::Utf8ToWide(d).c_str(), _TRUNCATE);
    if (dir && dirLen) wcsncpy_s(dir, dirLen, Irine::Platform::Utf8ToWide(dr).c_str(), _TRUNCATE);
    if (fname && fnameLen) wcsncpy_s(fname, fnameLen, Irine::Platform::Utf8ToWide(fn).c_str(), _TRUNCATE);
    if (ext && extLen) wcsncpy_s(ext, extLen, Irine::Platform::Utf8ToWide(ex).c_str(), _TRUNCATE);
    return rc;
}

void _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext) noexcept
{
    if (!path) return;
    std::string out;
    if (drive && drive[0]) out += drive;
    if (dir && dir[0])
    {
        out += dir;
        if (out.back() != '/' && out.back() != '\\') out += '/';
    }
    if (fname && fname[0]) out += fname;
    if (ext && ext[0])
    {
        if (ext[0] != '.') out += '.';
        out += ext;
    }
    std::strcpy(path, out.c_str());
}

char* _fullpath(char* absPath, const char* relPath, size_t maxLength) noexcept
{
    if (!relPath) return nullptr;
    std::string norm = NormalizeLinuxPath(relPath);
    char resolved[PATH_MAX] = { 0 };
    const char* src = ::realpath(norm.c_str(), resolved) ? resolved : norm.c_str();
    if (!absPath) return ::strdup(src);
    if (maxLength == 0) return nullptr;
    strncpy_s(absPath, maxLength, src, _TRUNCATE);
    return absPath;
}

errno_t rand_s(unsigned int* randomValue) noexcept
{
    if (!randomValue) return EINVAL;
    static thread_local std::mt19937 rng(std::random_device{}());
    *randomValue = rng();
    return 0;
}

int      _strtoi64(const char* str, char** endptr, int base) noexcept    { return static_cast<int>(std::strtoll(str, endptr, base)); }
int64_t  _strtoi64_ll(const char* str, char** endptr, int base) noexcept { return std::strtoll(str, endptr, base); }
uint64_t _strtoui64(const char* str, char** endptr, int base) noexcept   { return std::strtoull(str, endptr, base); }
int64_t  _wcstoi64(const WCHAR* str, WCHAR** endptr, int base) noexcept  { return ::wcstoll(str, endptr, base); }
uint64_t _wcstoui64(const WCHAR* str, WCHAR** endptr, int base) noexcept { return ::wcstoull(str, endptr, base); }

int _filelength(int fd) noexcept
{
    struct stat sb{};
    return (::fstat(fd, &sb) == 0) ? static_cast<int>(sb.st_size) : -1;
}

int64_t _filelengthi64(int fd) noexcept
{
    struct stat sb{};
    return (::fstat(fd, &sb) == 0) ? static_cast<int64_t>(sb.st_size) : -1;
}

int _fileno(FILE* stream) noexcept { return stream ? ::fileno(stream) : -1; }
int _chsize(int fd, long size) noexcept { return ::ftruncate(fd, size); }
int _stat64(const char* path, struct stat* buffer) noexcept { return path ? ::stat(NormalizeLinuxPath(path).c_str(), buffer) : -1; }
int _wstat(const WCHAR* path, struct stat* buffer) noexcept { return path ? ::stat(NormalizeLinuxPathW(path).c_str(), buffer) : -1; }

int _kbhit() noexcept
{
    struct timeval tv{ 0, 0 };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return ::select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0 ? 1 : 0;
}

int _getch() noexcept { return ::getchar(); }
int _putch(int c) noexcept { return ::putchar(c); }

errno_t strncat_s(char* dest, size_t destSize, const char* src, size_t count)
{
    if (!dest || destSize == 0 || !src) return EINVAL;
    size_t dlen = std::strlen(dest);
    if (dlen >= destSize) return ERANGE;
    size_t rem = destSize - dlen - 1;
    size_t slen = std::strlen(src);
    size_t copy = (count == _TRUNCATE) ? std::min(slen, rem) : std::min(std::min(slen, count), rem);
    std::memcpy(dest + dlen, src, copy);
    dest[dlen + copy] = '\0';
    return 0;
}

errno_t wcsncat_s(WCHAR* dest, size_t destElements, const WCHAR* src, size_t count)
{
    if (!dest || destElements == 0 || !src) return EINVAL;
    size_t dlen = ::wcslen(dest);
    if (dlen >= destElements) return ERANGE;
    size_t rem = destElements - dlen - 1;
    size_t slen = ::wcslen(src);
    size_t copy = (count == _TRUNCATE) ? std::min(slen, rem) : std::min(std::min(slen, count), rem);
    std::memcpy(dest + dlen, src, copy * sizeof(WCHAR));
    dest[dlen + copy] = 0;
    return 0;
}

DWORD GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart) noexcept
{
    if (!lpFileName) return 0;
    std::string norm = NormalizeLinuxPath(lpFileName);
    char resolved[PATH_MAX] = { 0 };
    std::string full;
    if (::realpath(norm.c_str(), resolved))
    {
        full = resolved;
    }
    else if (!norm.empty() && norm[0] == '/')
    {
        full = norm;
    }
    else
    {
        char cwd[PATH_MAX] = { 0 };
        if (::getcwd(cwd, sizeof(cwd)))
            full = std::string(cwd) + "/" + norm;
        else
            full = norm;
    }
    if (!lpBuffer || nBufferLength == 0) return static_cast<DWORD>(full.size() + 1);
    size_t copy = std::min(full.size(), static_cast<size_t>(nBufferLength - 1));
    std::memcpy(lpBuffer, full.data(), copy);
    lpBuffer[copy] = '\0';
    if (lpFilePart)
    {
        char* slash = std::strrchr(lpBuffer, '/');
        *lpFilePart = slash ? (slash + 1) : lpBuffer;
    }
    return static_cast<DWORD>(copy);
}

DWORD GetFullPathNameW(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR* lpFilePart) noexcept
{
    if (!lpFileName) return 0;
    char tmp[PATH_MAX] = { 0 };
    DWORD n = GetFullPathNameA(Irine::Platform::WideToUtf8(lpFileName).c_str(), sizeof(tmp), tmp, nullptr);
    if (n == 0) return 0;
    std::wstring w = Irine::Platform::Utf8ToWide(tmp, static_cast<int>(n));
    if (!lpBuffer || nBufferLength == 0) return static_cast<DWORD>(w.size() + 1);
    size_t copy = std::min(w.size(), static_cast<size_t>(nBufferLength - 1));
    std::memcpy(lpBuffer, w.data(), copy * sizeof(WCHAR));
    lpBuffer[copy] = 0;
    if (lpFilePart)
    {
        WCHAR* slash = const_cast<WCHAR*>(::wcsrchr(lpBuffer, L'/'));
        *lpFilePart = slash ? (slash + 1) : lpBuffer;
    }
    return static_cast<DWORD>(copy);
}

BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime) noexcept
{
    if (!lpFileTime || !lpLocalFileTime) return FALSE;
    uint64_t ticks = (static_cast<uint64_t>(lpFileTime->dwHighDateTime) << 32) | lpFileTime->dwLowDateTime;
    int64_t sec = static_cast<int64_t>(ticks / 10000000ULL) - 11644473600LL;
    time_t tt = static_cast<time_t>(sec);
    struct tm localTm{};
    ::localtime_r(&tt, &localTm);
    int64_t offsetSec = static_cast<int64_t>(localTm.tm_gmtoff);
    int64_t localTicks = static_cast<int64_t>(ticks) + offsetSec * 10000000LL;
    if (localTicks < 0) localTicks = 0;
    lpLocalFileTime->dwLowDateTime  = static_cast<DWORD>(static_cast<uint64_t>(localTicks) & 0xFFFFFFFFu);
    lpLocalFileTime->dwHighDateTime = static_cast<DWORD>(static_cast<uint64_t>(localTicks) >> 32);
    return TRUE;
}

LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2) noexcept
{
    if (!lpFileTime1 || !lpFileTime2) return 0;
    uint64_t t1 = (static_cast<uint64_t>(lpFileTime1->dwHighDateTime) << 32) | lpFileTime1->dwLowDateTime;
    uint64_t t2 = (static_cast<uint64_t>(lpFileTime2->dwHighDateTime) << 32) | lpFileTime2->dwLowDateTime;
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    return 0;
}

int wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...) noexcept
{
    if (!lpOut || !lpFmt) return 0;
    va_list args;
    va_start(args, lpFmt);
    int r = ::vsnprintf(lpOut, 1024, lpFmt, args);
    va_end(args);
    return r;
}

int wsprintfW(LPWSTR lpOut, LPCWSTR lpFmt, ...) noexcept
{
    if (!lpOut || !lpFmt) return 0;
    va_list args;
    va_start(args, lpFmt);
    int r = ::vswprintf(lpOut, 1024, lpFmt, args);
    va_end(args);
    return r;
}

char* _strrev(char* str) noexcept
{
    if (!str) return nullptr;
    size_t len = std::strlen(str);
    for (size_t i = 0; i < len / 2; ++i)
        std::swap(str[i], str[len - 1 - i]);
    return str;
}

int _wsplitpath(const WCHAR* path, WCHAR* drive, WCHAR* dir, WCHAR* fname, WCHAR* ext) noexcept
{
    return _wsplitpath_s(path, drive, drive ? _MAX_DRIVE : 0, dir, dir ? _MAX_DIR : 0, fname, fname ? _MAX_FNAME : 0, ext, ext ? _MAX_EXT : 0);
}

int GetLocaleInfoA(LCID, DWORD LCType, LPSTR lpLCData, int cchData) noexcept
{
    const char* val = (LCType == LOCALE_IDEFAULTANSICODEPAGE) ? "65001" : "ENU";
    int len = static_cast<int>(std::strlen(val)) + 1;
    if (!lpLCData || cchData <= 0) return len;
    strncpy_s(lpLCData, static_cast<size_t>(cchData), val, _TRUNCATE);
    return std::min(len, cchData);
}

int GetLocaleInfoW(LCID, DWORD LCType, LPWSTR lpLCData, int cchData) noexcept
{
    const WCHAR* val = (LCType == LOCALE_IDEFAULTANSICODEPAGE) ? L"65001" : L"ENU";
    int len = static_cast<int>(::wcslen(val)) + 1;
    if (!lpLCData || cchData <= 0) return len;
    wcsncpy_s(lpLCData, static_cast<size_t>(cchData), val, _TRUNCATE);
    return std::min(len, cchData);
}

LONG RegOpenKeyExA(HKEY, LPCSTR, DWORD, REGSAM, PHKEY phkResult) noexcept
{
    if (phkResult) *phkResult = nullptr;
    return ERROR_FILE_NOT_FOUND;
}

LONG RegOpenKeyExW(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY phkResult) noexcept
{
    if (phkResult) *phkResult = nullptr;
    return ERROR_FILE_NOT_FOUND;
}

LONG RegCreateKeyExA(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY phkResult, LPDWORD lpdwDisposition) noexcept
{
    if (phkResult) *phkResult = nullptr;
    if (lpdwDisposition) *lpdwDisposition = REG_CREATED_NEW_KEY;
    return ERROR_ACCESS_DENIED;
}

LONG RegCreateKeyExW(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY phkResult, LPDWORD lpdwDisposition) noexcept
{
    if (phkResult) *phkResult = nullptr;
    if (lpdwDisposition) *lpdwDisposition = REG_CREATED_NEW_KEY;
    return ERROR_ACCESS_DENIED;
}

LONG RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD) noexcept { return ERROR_FILE_NOT_FOUND; }
LONG RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD) noexcept { return ERROR_FILE_NOT_FOUND; }
LONG RegSetValueExA(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD) noexcept { return ERROR_ACCESS_DENIED; }
LONG RegSetValueExW(HKEY, LPCWSTR, DWORD, DWORD, const BYTE*, DWORD) noexcept { return ERROR_ACCESS_DENIED; }
LONG RegEnumKeyExA(HKEY, DWORD, LPSTR, LPDWORD, LPDWORD, LPSTR, LPDWORD, LPFILETIME) noexcept { return ERROR_NO_MORE_FILES; }
LONG RegEnumKeyExW(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPWSTR, LPDWORD, LPFILETIME) noexcept { return ERROR_NO_MORE_FILES; }
LONG RegDeleteKeyA(HKEY, LPCSTR) noexcept { return ERROR_ACCESS_DENIED; }
LONG RegDeleteKeyW(HKEY, LPCWSTR) noexcept { return ERROR_ACCESS_DENIED; }
LONG RegCloseKey(HKEY) noexcept { return ERROR_SUCCESS; }

BOOL CopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, void*, LPVOID, LPBOOL, DWORD dwCopyFlags) noexcept
{
    return CopyFileA(lpExistingFileName, lpNewFileName, (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? TRUE : FALSE);
}

BOOL CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, void*, LPVOID, LPBOOL, DWORD dwCopyFlags) noexcept
{
    return CopyFileW(lpExistingFileName, lpNewFileName, (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? TRUE : FALSE);
}

BOOL LockFile(HANDLE hFile, DWORD, DWORD, DWORD, DWORD) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj) return FALSE;
    ReleaseHandleRef(obj);
    return TRUE;
}

BOOL UnlockFile(HANDLE hFile, DWORD, DWORD, DWORD, DWORD) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj) return FALSE;
    ReleaseHandleRef(obj);
    return TRUE;
}

BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) noexcept
{
    WinHandleObj* obj = LookupHandle(hFile);
    if (!obj || obj->kind != HandleKind::File || obj->fd < 0)
    {
        if (obj) ReleaseHandleRef(obj);
        return FALSE;
    }
    struct stat sb{};
    bool ok = (::fstat(obj->fd, &sb) == 0);
    ReleaseHandleRef(obj);
    if (!ok) return FALSE;
    if (lpCreationTime)   UnixTimeToFileTime(sb.st_ctime, lpCreationTime);
    if (lpLastAccessTime) UnixTimeToFileTime(sb.st_atime, lpLastAccessTime);
    if (lpLastWriteTime)  UnixTimeToFileTime(sb.st_mtime, lpLastWriteTime);
    return TRUE;
}

UINT GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName) noexcept
{
    if (!lpPathName || !lpPrefixString || !lpTempFileName) return 0;
    static std::atomic<UINT> s_seq{ 1 };
    UINT id = uUnique ? uUnique : s_seq.fetch_add(1, std::memory_order_relaxed);
    std::string dir = NormalizeLinuxPath(lpPathName);
    if (!dir.empty() && dir.back() != '/') dir.push_back('/');
    char buf[MAX_PATH] = { 0 };
    std::snprintf(buf, sizeof(buf), "%s%.3s%04X.tmp", dir.c_str(), lpPrefixString, id & 0xFFFFu);
    strncpy_s(lpTempFileName, MAX_PATH, buf, _TRUNCATE);
    if (uUnique == 0)
    {
        int fd = ::open(buf, O_CREAT | O_RDWR, 0644);
        if (fd >= 0) ::close(fd);
    }
    return id;
}

UINT GetTempFileNameW(LPCWSTR lpPathName, LPCWSTR lpPrefixString, UINT uUnique, LPWSTR lpTempFileName) noexcept
{
    if (!lpPathName || !lpPrefixString || !lpTempFileName) return 0;
    char outA[MAX_PATH] = { 0 };
    UINT id = GetTempFileNameA(Irine::Platform::WideToUtf8(lpPathName).c_str(),
                               Irine::Platform::WideToUtf8(lpPrefixString).c_str(),
                               uUnique, outA);
    if (id == 0) return 0;
    wcsncpy_s(lpTempFileName, MAX_PATH, Irine::Platform::Utf8ToWide(outA).c_str(), _TRUNCATE);
    return id;
}

BOOL CreatePipe(HANDLE* hReadPipe, HANDLE* hWritePipe, LPSECURITY_ATTRIBUTES, DWORD) noexcept
{
    if (!hReadPipe || !hWritePipe) return FALSE;
    int fds[2] = { -1, -1 };
    if (::pipe(fds) != 0) return FALSE;
    auto* rObj = new WinHandleObj();
    rObj->kind = HandleKind::File;
    rObj->fd   = fds[0];
    auto* wObj = new WinHandleObj();
    wObj->kind = HandleKind::File;
    wObj->fd   = fds[1];
    *hReadPipe  = RegisterHandle(rObj);
    *hWritePipe = RegisterHandle(wObj);
    return TRUE;
}

BOOL CreateProcessA(LPCSTR, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, STARTUPINFOA* lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) noexcept
{
    if (!lpCommandLine || !lpCommandLine[0]) return FALSE;
    if (lpProcessInformation) std::memset(lpProcessInformation, 0, sizeof(*lpProcessInformation));
    FILE* fp = ::popen(lpCommandLine, "r");
    if (!fp) return FALSE;
    if (lpStartupInfo && (lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) && lpStartupInfo->hStdOutput)
    {
        char buf[4096];
        size_t n = 0;
        while ((n = ::fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            DWORD written = 0;
            WriteFile(lpStartupInfo->hStdOutput, buf, static_cast<DWORD>(n), &written, nullptr);
        }
    }
    ::pclose(fp);
    return TRUE;
}

BOOL CreateProcessW(LPCWSTR, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES pa, LPSECURITY_ATTRIBUTES ta, BOOL inherit, DWORD flags, LPVOID env, LPCWSTR, STARTUPINFOW* si, LPPROCESS_INFORMATION pi) noexcept
{
    if (!lpCommandLine) return FALSE;
    std::string cmd = Irine::Platform::WideToUtf8(lpCommandLine);
    STARTUPINFOA siA{};
    if (si)
    {
        siA.cb         = sizeof(siA);
        siA.dwFlags    = si->dwFlags;
        siA.hStdInput  = si->hStdInput;
        siA.hStdOutput = si->hStdOutput;
        siA.hStdError  = si->hStdError;
    }
    return CreateProcessA(nullptr, &cmd[0], pa, ta, inherit, flags, env, nullptr, si ? &siA : nullptr, pi);
}

BOOL GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters) noexcept
{
    std::string path = (lpRootPathName && lpRootPathName[0]) ? NormalizeLinuxPath(lpRootPathName) : ".";
    struct statvfs st{};
    if (::statvfs(path.c_str(), &st) != 0) return FALSE;
    DWORD blockSize = st.f_frsize ? static_cast<DWORD>(st.f_frsize) : 4096u;
    if (lpSectorsPerCluster)     *lpSectorsPerCluster     = 1;
    if (lpBytesPerSector)        *lpBytesPerSector        = blockSize;
    if (lpNumberOfFreeClusters)  *lpNumberOfFreeClusters  = static_cast<DWORD>(std::min<uint64_t>(st.f_bavail, 0xFFFFFFFFu));
    if (lpTotalNumberOfClusters) *lpTotalNumberOfClusters = static_cast<DWORD>(std::min<uint64_t>(st.f_blocks, 0xFFFFFFFFu));
    return TRUE;
}

BOOL GetDiskFreeSpaceW(LPCWSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters) noexcept
{
    std::string path = (lpRootPathName && lpRootPathName[0]) ? NormalizeLinuxPathW(lpRootPathName) : ".";
    return GetDiskFreeSpaceA(path.c_str(), lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters);
}
