// =============================================================================
// IrineCore — modules/Hero-Ally/src/HeroAllyModule.cpp
// Hero Ally Accompaniment Module (Geraint, Argenta, Velskud on Easy/Normal).
// =============================================================================
#include "HeroAllyModule.h"
#include <string>

namespace Irine {
namespace Modules {

bool HeroAllyModule::OnLoadConfig(const WCHAR* moduleConfigDirPath)
{
    const std::wstring path = ResolveModuleConfigPath(moduleConfigDirPath, L"Hero-Ally", L"Hero-Ally.ini");

    m_config.enabled              = (::GetPrivateProfileIntW(L"HeroAlly", L"Enabled", 1, path.c_str()) != 0);
    m_config.triggerChancePercent = static_cast<int>(::GetPrivateProfileIntW(L"HeroAlly", L"TriggerChancePercent", 100, path.c_str()));
    m_config.spawnAtStageStart    = (::GetPrivateProfileIntW(L"HeroAlly", L"SpawnAtStageStart", 1, path.c_str()) != 0);
    m_config.followAcrossAreas    = (::GetPrivateProfileIntW(L"HeroAlly", L"FollowAcrossAreas", 1, path.c_str()) != 0);
    m_config.despawnOnStageClear  = (::GetPrivateProfileIntW(L"HeroAlly", L"DespawnOnStageClear", 1, path.c_str()) != 0);
    m_config.geraintMonsterId     = static_cast<int>(::GetPrivateProfileIntW(L"HeroAlly", L"GeraintMonsterID", 236005, path.c_str()));
    m_config.argentaMonsterId     = static_cast<int>(::GetPrivateProfileIntW(L"HeroAlly", L"ArgentaMonsterID", 236003, path.c_str()));
    m_config.velskudMonsterId     = static_cast<int>(::GetPrivateProfileIntW(L"HeroAlly", L"VelskudMonsterID", 236011, path.c_str()));
    return true;
}

bool HeroAllyModule::ShouldSpawnHeroAlly(int difficulty) const noexcept
{
    if (!m_config.enabled)
        return false;
    // Hero Ally spawns only on Easy (0) and Normal (1) difficulty stages
    return (difficulty == 0 || difficulty == 1);
}

int HeroAllyModule::SelectHeroAllyMonsterId(uint32_t seed) const noexcept
{
    const int heroes[3] = {
        m_config.geraintMonsterId,
        m_config.argentaMonsterId,
        m_config.velskudMonsterId
    };
    return heroes[seed % 3u];
}

void Addmod_Hero_AllyScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new HeroAllyModule()));
}

} // namespace Modules
} // namespace Irine
