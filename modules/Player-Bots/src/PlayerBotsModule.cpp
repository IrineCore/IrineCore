// =============================================================================
// IrineCore — modules/Player-Bots/src/PlayerBotsModule.cpp
// Autonomous AI Party Companion Bots Module with modular strategy engine,
// player-synced level/gear/skill scaling, and full INI configuration.
// =============================================================================
#include "PlayerBotsModule.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <string>

namespace Irine {
namespace Modules {

namespace {

int ClampInt(int v, int lo, int hi) noexcept
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int ReadIniInt(const WCHAR* section, const WCHAR* key, int defaultVal, const std::wstring& path)
{
    return static_cast<int>(::GetPrivateProfileIntW(section, key, defaultVal, path.c_str()));
}

int ReadIniSignedInt(const WCHAR* section, const WCHAR* key, int defaultVal, const std::wstring& path)
{
    WCHAR buf[64] = { 0 };
    WCHAR defBuf[32] = { 0 };
    swprintf_s(defBuf, L"%d", defaultVal);
    ::GetPrivateProfileStringW(section, key, defBuf, buf, _countof(buf), path.c_str());
    return _wtoi(buf);
}

std::string ReadIniStringA(const WCHAR* section, const WCHAR* key, const char* defaultVal, const std::wstring& path)
{
    WCHAR defWide[512] = { 0 };
    if (defaultVal)
    {
        for (size_t i = 0; defaultVal[i] && i + 1 < _countof(defWide); ++i)
            defWide[i] = static_cast<WCHAR>(static_cast<unsigned char>(defaultVal[i]));
    }
    WCHAR buf[512] = { 0 };
    ::GetPrivateProfileStringW(section, key, defWide, buf, _countof(buf), path.c_str());

    std::string out;
    for (size_t i = 0; buf[i] != L'\0'; ++i)
        out.push_back(static_cast<char>(buf[i] & 0x7F));
    return out;
}

std::string TrimLower(const std::string& in)
{
    size_t start = 0;
    while (start < in.size() && std::isspace(static_cast<unsigned char>(in[start])))
        ++start;
    size_t end = in.size();
    while (end > start && std::isspace(static_cast<unsigned char>(in[end - 1])))
        --end;

    std::string out;
    out.reserve(end - start);
    for (size_t i = start; i < end; ++i)
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(in[i]))));
    return out;
}

struct StrategyNamedFlag
{
    const char* name;
    uint32_t    flag;
};

static const StrategyNamedFlag kCombatStrategyTable[] = {
    { "tank",         CS_TANK },
    { "healer",       CS_HEALER },
    { "heal",         CS_HEALER },
    { "dps",          CS_DPS },
    { "icestack",     CS_ICESTACK },
    { "ice",          CS_ICESTACK },
    { "support",      CS_SUPPORT },
    { "aoe",          CS_AOE },
    { "burst",        CS_BURST },
    { "threat",       CS_THREAT },
    { "avoid_aoe",    CS_AVOID_AOE },
    { "avoid",        CS_AVOID_AOE },
    { "dodge",        CS_AVOID_AOE },
    { "defensive",    CS_DEFENSIVE },
    { "def",          CS_DEFENSIVE },
    { "assist",       CS_ASSIST },
    { "focus_boss",   CS_FOCUS_BOSS },
    { "boss",         CS_FOCUS_BOSS },
    { "clear_adds",   CS_CLEAR_ADDS },
    { "adds",         CS_CLEAR_ADDS },
    { "save_mana",    CS_SAVE_MANA },
    { "mana",         CS_SAVE_MANA },
    { "break_props",  CS_BREAK_PROPS },
    { "props",        CS_BREAK_PROPS },
    { "dodge_cancel", CS_DODGE_CANCEL },
    { "cancel",       CS_DODGE_CANCEL },
};

static const StrategyNamedFlag kCombatCanonicalNames[] = {
    { "tank",         CS_TANK },
    { "healer",       CS_HEALER },
    { "dps",          CS_DPS },
    { "icestack",     CS_ICESTACK },
    { "support",      CS_SUPPORT },
    { "aoe",          CS_AOE },
    { "burst",        CS_BURST },
    { "threat",       CS_THREAT },
    { "avoid_aoe",    CS_AVOID_AOE },
    { "defensive",    CS_DEFENSIVE },
    { "assist",       CS_ASSIST },
    { "focus_boss",   CS_FOCUS_BOSS },
    { "clear_adds",   CS_CLEAR_ADDS },
    { "save_mana",    CS_SAVE_MANA },
    { "break_props",  CS_BREAK_PROPS },
    { "dodge_cancel", CS_DODGE_CANCEL },
};

static const StrategyNamedFlag kNonCombatStrategyTable[] = {
    { "follow",      NCS_FOLLOW },
    { "stay",        NCS_STAY },
    { "hold",        NCS_STAY },
    { "buff",        NCS_BUFF },
    { "gather_gate", NCS_GATHER_GATE },
    { "gate",        NCS_GATHER_GATE },
    { "passive",     NCS_PASSIVE },
};

static const StrategyNamedFlag kNonCombatCanonicalNames[] = {
    { "follow",      NCS_FOLLOW },
    { "stay",        NCS_STAY },
    { "buff",        NCS_BUFF },
    { "gather_gate", NCS_GATHER_GATE },
    { "passive",     NCS_PASSIVE },
};

uint32_t ApplyModifierList(uint32_t mask,
                           const std::string& expr,
                           const StrategyNamedFlag* table,
                           size_t tableSize,
                           bool isNonCombat,
                           std::string* pOutUnknown)
{
    std::string token;
    auto processToken = [&](const std::string& raw) {
        std::string s = TrimLower(raw);
        if (s.empty()) return;

        bool bRemove = false;
        if (s[0] == '+')
            s.erase(0, 1);
        else if (s[0] == '-')
        {
            bRemove = true;
            s.erase(0, 1);
        }
        if (s.empty()) return;

        uint32_t found = 0;
        for (size_t i = 0; i < tableSize; ++i)
        {
            if (s == table[i].name)
            {
                found = table[i].flag;
                break;
            }
        }
        if (found == 0)
        {
            if (pOutUnknown)
            {
                if (!pOutUnknown->empty()) pOutUnknown->append(", ");
                pOutUnknown->append(s);
            }
            return;
        }

        if (bRemove)
        {
            mask &= ~found;
        }
        else
        {
            mask |= found;
            // Enforce mutual exclusions for clarity
            if (isNonCombat)
            {
                if (found == NCS_STAY)   mask &= ~NCS_FOLLOW;
                if (found == NCS_FOLLOW) mask &= ~NCS_STAY;
            }
            else
            {
                if (found == CS_FOCUS_BOSS) mask &= ~CS_CLEAR_ADDS;
                if (found == CS_CLEAR_ADDS) mask &= ~CS_FOCUS_BOSS;
            }
        }
    };

    for (size_t i = 0; i <= expr.size(); ++i)
    {
        char ch = (i < expr.size()) ? expr[i] : ',';
        if (ch == ',' || ch == ';' || ch == ' ')
        {
            if (!token.empty())
            {
                processToken(token);
                token.clear();
            }
        }
        else
        {
            token.push_back(ch);
        }
    }
    return mask;
}

std::string FormatFlagMask(uint32_t mask, const StrategyNamedFlag* table, size_t tableSize)
{
    std::string out;
    for (size_t i = 0; i < tableSize; ++i)
    {
        if (mask & table[i].flag)
        {
            if (!out.empty()) out.append(", ");
            out.append(table[i].name);
        }
    }
    return out.empty() ? std::string("none") : out;
}

} // namespace

namespace BotStrategyUtil {

int ResolveUnlockedJobTier(int level, bool unlockByLevel) noexcept
{
    if (!unlockByLevel)
        return 2;
    if (level >= 45)
        return 2;
    if (level >= 15)
        return 1;
    return 0;
}

int ComputeSyncedBotLevel(int humanLevel, const PlayerBotsConfig& cfg) noexcept
{
    int minLv = ClampInt(cfg.minBotLevel, 1, 60);
    int maxLv = ClampInt(cfg.maxBotLevel, minLv, 60);
    if (!cfg.syncLevelToPlayer)
        return maxLv;

    int raw = (humanLevel > 0 ? humanLevel : 1) + cfg.levelOffset;
    return ClampInt(raw, minLv, maxLv);
}

int ComputeBaselineGearRankForLevel(int botLevel) noexcept
{
    // Standard Dragon Nest progression ranks by level bracket:
    // 0=Normal(D), 1=Magic(C), 2=Rare(B), 3=Epic(A)
    if (botLevel < 15) return 1; // Magic (Lv 1-14)
    if (botLevel < 32) return 2; // Rare  (Lv 15-31)
    if (botLevel < 50) return 2; // Rare  (Lv 32-49)
    return 3;                    // Epic  (Lv 50-60)
}

int ComputeSyncedGearRank(int botLevel, int humanEquippedCount, float humanAvgRank, const PlayerBotsConfig& cfg) noexcept
{
    int minR = ClampInt(cfg.minGearRank, 0, 5);
    int maxR = ClampInt(cfg.maxGearRank, minR, 5);

    if (cfg.gearSyncMode == GEAR_SYNC_FIXED_ENDGAME)
        return ClampInt(4, minR, maxR); // Unique/Epic endgame

    int baseRank = ComputeBaselineGearRankForLevel(botLevel);
    if (cfg.gearSyncMode == GEAR_SYNC_MATCH_PLAYER && humanEquippedCount >= 3)
    {
        baseRank = static_cast<int>(std::floor(humanAvgRank + 0.5f));
    }

    int finalRank = baseRank + cfg.gearQualityOffset;
    return ClampInt(finalRank, minR, maxR);
}

int ComputeSyncedEnchantLevel(int botLevel, int targetRank, int humanEquippedCount, float humanAvgEnchant, const PlayerBotsConfig& cfg) noexcept
{
    int maxE = ClampInt(cfg.maxEnchantLevel, 0, 15);
    if (cfg.gearSyncMode == GEAR_SYNC_FIXED_ENDGAME)
        return maxE;

    // Normal (white) items generally don't need high enhancement
    if (targetRank <= 0)
        return 0;

    int baseEnchant = 0;
    if (cfg.gearSyncMode == GEAR_SYNC_MATCH_PLAYER && cfg.matchPlayerEnchant && humanEquippedCount >= 3)
    {
        baseEnchant = static_cast<int>(std::floor(humanAvgEnchant + 0.5f));
    }
    else
    {
        // Gentle level-appropriate baseline when not matching player enchant
        if (botLevel >= 50)      baseEnchant = 8;
        else if (botLevel >= 40) baseEnchant = 6;
        else if (botLevel >= 24) baseEnchant = 4;
        else if (botLevel >= 15) baseEnchant = 2;
        else                     baseEnchant = 0;
    }

    return ClampInt(baseEnchant + cfg.enchantOffset, 0, maxE);
}

uint32_t ApplyCombatStrategyModifiers(uint32_t currentMask, const std::string& expr, std::string* pOutUnknown)
{
    return ApplyModifierList(currentMask, expr, kCombatStrategyTable, _countof(kCombatStrategyTable), false, pOutUnknown);
}

uint32_t ApplyNonCombatStrategyModifiers(uint32_t currentMask, const std::string& expr, std::string* pOutUnknown)
{
    return ApplyModifierList(currentMask, expr, kNonCombatStrategyTable, _countof(kNonCombatStrategyTable), true, pOutUnknown);
}

std::string FormatCombatStrategies(uint32_t mask)
{
    return FormatFlagMask(mask, kCombatCanonicalNames, _countof(kCombatCanonicalNames));
}

std::string FormatNonCombatStrategies(uint32_t mask)
{
    return FormatFlagMask(mask, kNonCombatCanonicalNames, _countof(kNonCombatCanonicalNames));
}

} // namespace BotStrategyUtil

bool PlayerBotsModule::OnLoadConfig(const WCHAR* moduleConfigDirPath)
{
    const std::wstring path = ResolveModuleConfigPath(moduleConfigDirPath, this);

    // [PlayerBots]
    m_config.enabled                 = (ReadIniInt(L"PlayerBots", L"Enabled", 1, path) != 0);
    m_config.excludeEasyAndNormal    = (ReadIniInt(L"PlayerBots", L"ExcludeEasyAndNormal", 1, path) != 0);
    m_config.maxBotsPerParty         = ClampInt(ReadIniInt(L"PlayerBots", L"MaxBotsPerParty", 7, path), 0, 7);
    m_config.autoSummonInDungeons    = (ReadIniInt(L"PlayerBots", L"AutoSummonInDungeons", 1, path) != 0);
    m_config.autoSummonInNests       = (ReadIniInt(L"PlayerBots", L"AutoSummonInNests", 1, path) != 0);
    m_config.useRecommendedPartySize = (ReadIniInt(L"PlayerBots", L"UseRecommendedPartySize", 1, path) != 0);

    // [Scaling]
    m_config.syncLevelToPlayer       = (ReadIniInt(L"Scaling", L"SyncLevelToPlayer", 1, path) != 0);
    m_config.levelOffset             = ClampInt(ReadIniSignedInt(L"Scaling", L"LevelOffset", 0, path), -20, 20);
    m_config.minBotLevel             = ClampInt(ReadIniInt(L"Scaling", L"MinBotLevel", 1, path), 1, 60);
    m_config.maxBotLevel             = ClampInt(ReadIniInt(L"Scaling", L"MaxBotLevel", 60, path), m_config.minBotLevel, 60);
    m_config.unlockJobByLevel        = (ReadIniInt(L"Scaling", L"UnlockJobByLevel", 1, path) != 0);
    m_config.resyncOnStageTransition = (ReadIniInt(L"Scaling", L"ResyncOnStageTransition", 1, path) != 0);

    // [Gear]
    m_config.gearSyncMode            = ClampInt(ReadIniInt(L"Gear", L"GearSyncMode", GEAR_SYNC_MATCH_PLAYER, path), 0, 2);
    m_config.gearQualityOffset       = ClampInt(ReadIniSignedInt(L"Gear", L"GearQualityOffset", 0, path), -3, 3);
    m_config.minGearRank             = ClampInt(ReadIniInt(L"Gear", L"MinGearRank", 0, path), 0, 5);
    m_config.maxGearRank             = ClampInt(ReadIniInt(L"Gear", L"MaxGearRank", 5, path), m_config.minGearRank, 5);
    m_config.matchPlayerEnchant      = (ReadIniInt(L"Gear", L"MatchPlayerEnchant", 1, path) != 0);
    m_config.enchantOffset           = ClampInt(ReadIniSignedInt(L"Gear", L"EnchantOffset", 0, path), -15, 15);
    m_config.maxEnchantLevel         = ClampInt(ReadIniInt(L"Gear", L"MaxEnchantLevel", 12, path), 0, 15);
    m_config.matchPlayerPotentials   = (ReadIniInt(L"Gear", L"MatchPlayerPotentials", 1, path) != 0);
    m_config.matchPlayerCashCostumes = (ReadIniInt(L"Gear", L"MatchPlayerCashCostumes", 1, path) != 0);
    m_config.matchPlayerGlyphs       = (ReadIniInt(L"Gear", L"MatchPlayerGlyphs", 1, path) != 0);
    m_config.matchPlayerTalismans    = (ReadIniInt(L"Gear", L"MatchPlayerTalismans", 1, path) != 0);

    // [Skills]
    m_config.syncSkillsToLevel         = (ReadIniInt(L"Skills", L"SyncSkillsToLevel", 1, path) != 0);
    m_config.respectSkillPrerequisites = (ReadIniInt(L"Skills", L"RespectSkillPrerequisites", 1, path) != 0);
    m_config.skillPowerPercent         = ClampInt(ReadIniInt(L"Skills", L"SkillPowerPercent", 0, path), 0, 100);
    m_config.healerManaReservePercent  = ClampInt(ReadIniInt(L"Skills", L"HealerManaReservePercent", 25, path), 0, 80);
    m_config.dpsManaConservePercent    = ClampInt(ReadIniInt(L"Skills", L"DpsManaConservePercent", 15, path), 0, 80);

    // [Strategies]
    m_config.defaultCombatStrategies = ReadIniStringA(
        L"Strategies", L"DefaultCombatStrategies",
        "+aoe,+burst,+threat,+avoid_aoe,+assist,+save_mana,+break_props,+dodge_cancel", path);
    m_config.defaultNonCombatStrategies = ReadIniStringA(
        L"Strategies", L"DefaultNonCombatStrategies",
        "+follow,+buff,+gather_gate", path);
    m_config.applyInstanceStrategies = (ReadIniInt(L"Strategies", L"ApplyInstanceStrategies", 1, path) != 0);

    // [CombatAI] — also read legacy [PlayerBots] keys as fallback so old configs keep working
    int legacyAggro  = ReadIniInt(L"PlayerBots", L"AggroRange", 1500, path);
    int legacyFollow = ReadIniInt(L"PlayerBots", L"FollowDistance", 300, path);
    m_config.aggroRange               = ClampInt(ReadIniInt(L"CombatAI", L"AggroRange", legacyAggro, path), 400, 4000);
    m_config.followDistance           = ClampInt(ReadIniInt(L"CombatAI", L"FollowDistance", legacyFollow, path), 120, 1200);
    m_config.globalSkillDelayMs       = ClampInt(ReadIniInt(L"CombatAI", L"GlobalSkillDelayMs", 260, path), 80, 1500);
    m_config.reactionJitterMs         = ClampInt(ReadIniInt(L"CombatAI", L"ReactionJitterMs", 55, path), 0, 300);
    m_config.healThresholdDungeon     = ClampInt(ReadIniInt(L"CombatAI", L"HealThresholdDungeon", 75, path), 30, 95);
    m_config.healThresholdNest        = ClampInt(ReadIniInt(L"CombatAI", L"HealThresholdNest", 82, path), 35, 95);
    m_config.criticalHealThreshold    = ClampInt(ReadIniInt(L"CombatAI", L"CriticalHealThreshold", 42, path), 15, 75);
    m_config.selfDefensiveHpThreshold = ClampInt(ReadIniInt(L"CombatAI", L"SelfDefensiveHpThreshold", 35, path), 10, 75);
    m_config.overkillSkipHpPercent    = ClampInt(ReadIniInt(L"CombatAI", L"OverkillSkipHpPercent", 18, path), 0, 50);
    m_config.allySpreadRadius         = ClampInt(ReadIniInt(L"CombatAI", L"AllySpreadRadius", 115, path), 0, 300);
    m_config.assistHumanLeader        = (ReadIniInt(L"CombatAI", L"AssistHumanLeader", 1, path) != 0);
    m_config.allowBotChatCommands     = (ReadIniInt(L"CombatAI", L"AllowBotChatCommands", 1, path) != 0);
    m_config.enableTacticalCallouts   = (ReadIniInt(L"CombatAI", L"EnableTacticalCallouts", 1, path) != 0);

    return true;
}

bool PlayerBotsModule::ShouldAutoSummonBots(int difficulty, bool isNest) const noexcept
{
    if (!m_config.enabled)
        return false;
    // Easy (0) and Normal (1) regular stages never summon bots when ExcludeEasyAndNormal is active
    if (m_config.excludeEasyAndNormal && !isNest && (difficulty == 0 || difficulty == 1))
        return false;
    return isNest ? m_config.autoSummonInNests : m_config.autoSummonInDungeons;
}

const PlayerBotsConfig& PlayerBotsModule::GetActiveConfig() noexcept
{
    static const PlayerBotsConfig s_DefaultConfig;
    IModule* pMod = ModuleRegistry::Instance().GetModule("Player-Bots");
    if (pMod)
    {
        auto* pBots = dynamic_cast<PlayerBotsModule*>(pMod);
        if (pBots)
            return pBots->GetConfig();
    }
    return s_DefaultConfig;
}

void Addmod_Player_BotsScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new PlayerBotsModule()));
}

} // namespace Modules
} // namespace Irine
