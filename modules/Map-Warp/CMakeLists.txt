# =============================================================================
# IrineCore Module — modules/Map-Warp/CMakeLists.txt
# Smart Zone-Map NPC & Portal Warping Module (VillageServer + GameServer).
# =============================================================================
irine_register_module(
    NAME "Map-Warp"
    LOADER_FUNC "Addmod_Map_WarpScripts"
    COMMON_SOURCES
        "src/MapWarpModule.cpp"
    VILLAGESERVER_SOURCES
        "src/DnMapWarpVillage.cpp"
    GAMESERVER_SOURCES
        "src/DnMapWarpGame.cpp"
)
