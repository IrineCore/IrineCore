// =============================================================================
// IrineCore — common/platform/interfaces/IPlatformPoller.h
// Abstract asynchronous network I/O event demultiplexer (IOCP vs epoll).
// =============================================================================
#pragma once

#include "PlatformTypes.h"
#include <cstdint>
#include <memory>

namespace Irine {
namespace Platform {

struct PollEvent
{
    uintptr_t completionKey = 0;
    void*     overlapped    = nullptr;
    uint32_t  bytesTransferred = 0;
    int32_t   errorCode     = 0;
    bool      success       = false;
};

class IPlatformPoller
{
public:
    virtual ~IPlatformPoller() = default;

    virtual bool Initialize(uint32_t concurrentThreads = 0) = 0;
    virtual void Shutdown() = 0;

    // Associate a socket handle with a completion key
    virtual bool AssociateSocket(SOCKET sock, uintptr_t completionKey) = 0;

    // Post a custom completion notification to wake a worker thread
    virtual bool PostCompletion(uintptr_t completionKey, uint32_t bytesTransferred, void* overlapped) = 0;

    // Dequeue next completion event (timeoutMs = INFINITE blocks indefinitely)
    virtual bool WaitCompletion(PollEvent& outEvent, uint32_t timeoutMs = INFINITE) = 0;

    virtual HANDLE GetNativeHandle() const = 0;
};

std::unique_ptr<IPlatformPoller> CreatePlatformPoller();

} // namespace Platform
} // namespace Irine
