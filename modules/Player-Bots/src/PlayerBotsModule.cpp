// =============================================================================
// IrineCore — modules/Player-Bots/src/PlayerBotsModule.cpp
// Autonomous AI Party Companion Bots Module (excludes Easy/Normal stages).
// =============================================================================
#include "PlayerBotsModule.h"
#include <string>

namespace Irine {
namespace Modules {

bool PlayerBotsModule::OnLoadConfig(const WCHAR* moduleConfigDirPath)
{
    const std::wstring path = ResolveModuleConfigPath(moduleConfigDirPath, L"Player-Bots", L"Player-Bots.ini");

    m_config.enabled              = (::GetPrivateProfileIntW(L"PlayerBots", L"Enabled", 1, path.c_str()) != 0);
    m_config.excludeEasyAndNormal = (::GetPrivateProfileIntW(L"PlayerBots", L"ExcludeEasyAndNormal", 1, path.c_str()) != 0);
    m_config.maxBotsPerParty      = static_cast<int>(::GetPrivateProfileIntW(L"PlayerBots", L"MaxBotsPerParty", 3, path.c_str()));
    m_config.autoSummonInDungeons = (::GetPrivateProfileIntW(L"PlayerBots", L"AutoSummonInDungeons", 1, path.c_str()) != 0);
    m_config.autoSummonInNests    = (::GetPrivateProfileIntW(L"PlayerBots", L"AutoSummonInNests", 1, path.c_str()) != 0);
    m_config.aggroRange           = static_cast<int>(::GetPrivateProfileIntW(L"PlayerBots", L"AggroRange", 1500, path.c_str()));
    m_config.followDistance       = static_cast<int>(::GetPrivateProfileIntW(L"PlayerBots", L"FollowDistance", 300, path.c_str()));
    return true;
}

bool PlayerBotsModule::ShouldAutoSummonBots(int difficulty, bool isNest) const noexcept
{
    if (!m_config.enabled)
        return false;
    // Easy (0) and Normal (1) stages never summon bots when ExcludeEasyAndNormal is active
    if (m_config.excludeEasyAndNormal && (difficulty == 0 || difficulty == 1))
        return false;
    return isNest ? m_config.autoSummonInNests : m_config.autoSummonInDungeons;
}

void Addmod_Player_BotsScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new PlayerBotsModule()));
}

} // namespace Modules
} // namespace Irine
