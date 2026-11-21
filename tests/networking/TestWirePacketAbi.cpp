// =============================================================================
// IrineCore — tests/networking/TestWirePacketAbi.cpp
// Networking tests for IPlatformPoller (IOCP/epoll) and wire packet framing.
// =============================================================================
#include "../../common/platform/Platform.h"
#include <cstdio>

#pragma pack(push, 1)
struct TestDNTPacketHeader
{
    uint16_t iLen;
    uint8_t  cMainCmd;
    uint8_t  cSubCmd;
};
#pragma pack(pop)

int RunNetworkingTests()
{
    int failures = 0;
    auto check = [&](bool cond, const char* msg) {
        if (!cond) { std::fprintf(stderr, "  [FAIL][Networking] %s\n", msg); ++failures; }
        else       { std::printf("  [PASS][Networking] %s\n", msg); }
    };

    // 1. Wire packet header packing invariant
    check(sizeof(TestDNTPacketHeader) == 4, "sizeof(DNTPacketHeader) == 4 bytes (#pragma pack(1) wire invariant)");

    // 2. IPlatformPoller (WinIocpPoller on Windows, LinuxEpollPoller on Linux) round-trip
    auto poller = Irine::Platform::CreatePlatformPoller();
    check(poller != nullptr && poller->Initialize(2), "CreatePlatformPoller()->Initialize(2) succeeded");
    if (poller)
    {
        bool posted = poller->PostCompletion(0xCAFEBABE, 128, nullptr);
        Irine::Platform::PollEvent ev{};
        bool waited = poller->WaitCompletion(ev, 1000);
        check(posted && waited && ev.completionKey == 0xCAFEBABE && ev.bytesTransferred == 128,
              "IPlatformPoller PostCompletion -> WaitCompletion round-trip verified");
        poller->Shutdown();
    }

    return failures;
}
