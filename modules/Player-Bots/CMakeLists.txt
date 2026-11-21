# =============================================================================
# IrineCore Module — modules/Player-Bots/CMakeLists.txt
# IrineCore self-registering module (can be disabled without core edits).
# =============================================================================
irine_register_module(
    NAME "Player-Bots"
    LOADER_FUNC "Addmod_Player_BotsScripts"
    COMMON_SOURCES
        "src/PlayerBotsModule.cpp"
    GAMESERVER_SOURCES
        "src/DnAIBotBuildData.cpp"
        "src/DnAIBotController.cpp"
)
