// =============================================================================
// IrineCore — modules/FPS-PING/src/FpsPingModule.h
// Real-time Server/Client FPS & Ping Telemetry Module.
// =============================================================================
#pragma once

#include "ModuleRegistry.h"

namespace Irine {
namespace Modules {

struct FpsPingConfig
{
    bool enabled              = true;
    int  pingUpdateIntervalMs = 2000;
    bool showFpsOverlay       = true;
    bool showPingOverlay      = true;
};

class FpsPingModule final : public IModule
{
public:
    const char* GetName() const noexcept override { return "FPS-PING"; }
    const char* GetConfigFileName() const noexcept override { return "FPS-PING.ini"; }
    bool        IsEnabled() const noexcept override { return m_config.enabled; }

    bool OnLoadConfig(const WCHAR* moduleConfigDirPath) override;
    const FpsPingConfig& GetConfig() const noexcept { return m_config; }

private:
    FpsPingConfig m_config;
};

} // namespace Modules
} // namespace Irine
