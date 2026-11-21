// =============================================================================
// IrineCore — modules/FPS-PING/src/FpsPingModule.cpp
// Real-time Server/Client FPS & Ping Telemetry Module.
// =============================================================================
#include "FpsPingModule.h"
#include <string>

namespace Irine {
namespace Modules {

bool FpsPingModule::OnLoadConfig(const WCHAR* moduleConfigDirPath)
{
    const std::wstring path = ResolveModuleConfigPath(moduleConfigDirPath, L"FPS-PING", L"FPS-PING.ini");

    m_config.enabled              = (::GetPrivateProfileIntW(L"FpsPing", L"Enabled", 1, path.c_str()) != 0);
    m_config.pingUpdateIntervalMs = static_cast<int>(::GetPrivateProfileIntW(L"FpsPing", L"PingUpdateIntervalMs", 2000, path.c_str()));
    m_config.showFpsOverlay       = (::GetPrivateProfileIntW(L"FpsPing", L"ShowFpsOverlay", 1, path.c_str()) != 0);
    m_config.showPingOverlay      = (::GetPrivateProfileIntW(L"FpsPing", L"ShowPingOverlay", 1, path.c_str()) != 0);
    return true;
}

void Addmod_FPS_PINGScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new FpsPingModule()));
}

} // namespace Modules
} // namespace Irine
