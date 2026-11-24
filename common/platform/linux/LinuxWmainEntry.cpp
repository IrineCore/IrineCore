// =============================================================================
// IrineCore — common/platform/linux/LinuxWmainEntry.cpp
// Bridges POSIX main(int argc, char* argv[]) to legacy Win32 _tmain/wmain
// entry points on Linux servers. Placed in its own object file inside
// libirine_platform.a so executables defining main() (tests, tools) never
// pull this object, while all 7 IrineCore servers defining _tmain() link
// cleanly with automatic UTF-8 -> UTF-16LE argv conversion.
// =============================================================================
#if !defined(__linux__)
#error "LinuxWmainEntry.cpp must only be compiled on Linux targets."
#endif

#include "../Platform.h"
#include <signal.h>
#include <string>
#include <vector>

extern int _tmain(int argc, WCHAR* argv[]);

int main(int argc, char* argv[])
{
    ::signal(SIGPIPE, SIG_IGN);
    Irine::Platform::InstallCrashHandler(argc > 0 && argv[0] ? argv[0] : "IrineCore");

    std::vector<std::wstring> wideArgs;
    wideArgs.reserve(static_cast<size_t>(argc > 0 ? argc : 0));
    std::vector<WCHAR*> wargv;
    wargv.reserve(static_cast<size_t>((argc > 0 ? argc : 0) + 1));

    for (int i = 0; i < argc; ++i)
    {
        wideArgs.push_back(Irine::Platform::Utf8ToWide(argv[i] ? argv[i] : ""));
        wargv.push_back(&wideArgs.back()[0]);
    }
    wargv.push_back(nullptr);

    return _tmain(argc, wargv.data());
}
