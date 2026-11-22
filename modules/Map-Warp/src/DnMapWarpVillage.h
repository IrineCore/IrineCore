// =============================================================================
// IrineCore — modules/Map-Warp/src/DnMapWarpVillage.h
// VillageServer smart zone-map warp handler.
// =============================================================================
#pragma once

#include <cstdint>

class CDNUserSession;

namespace Irine {
namespace Modules {

// Authoritatively resolves an NPC or Portal/Gate on the session's current village field
// and warps the player safely in front of it.
bool HandleVillageMapWarp(CDNUserSession* pSession, uint8_t cTargetType, int nTargetID);

} // namespace Modules
} // namespace Irine
