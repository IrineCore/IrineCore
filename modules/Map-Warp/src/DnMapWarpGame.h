// =============================================================================
// IrineCore — modules/Map-Warp/src/DnMapWarpGame.h
// GameServer smart zone-map warp handler (WorldZone / Dungeon / Farm).
// =============================================================================
#pragma once

#include <cstdint>

class CDnPlayerActor;

namespace Irine {
namespace Modules {

// Authoritatively resolves an NPC or Portal/Gate on the actor's current game world
// and warps the player safely in front of it with navmesh, combat, and anti-cheat safety.
bool HandleGameMapWarp(CDnPlayerActor* pActor, uint8_t cTargetType, int nTargetID);

} // namespace Modules
} // namespace Irine
