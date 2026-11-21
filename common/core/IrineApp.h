// =============================================================================
// IrineCore — common/core/IrineApp.h
// Cross-platform server application lifecycle, banner, and signal management.
// =============================================================================
#pragma once

#include "../platform/Platform.h"
#include <cstdint>

namespace Irine {
namespace Core {

class Application
{
public:
    static void        InitializeProcess(const char* serverName) noexcept;
    static void        RequestShutdown() noexcept;
    static bool        IsShutdownRequested() noexcept;
    static uint64_t    GetUptimeSeconds() noexcept;
    static const char* GetServerName() noexcept;
    static const char* GetFrameworkVersion() noexcept;
};

} // namespace Core
} // namespace Irine
