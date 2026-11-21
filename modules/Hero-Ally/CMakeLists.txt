# =============================================================================
# IrineCore Module — modules/Hero-Ally/CMakeLists.txt
# IrineCore self-registering module (can be disabled without core edits).
# =============================================================================
irine_register_module(
    NAME "Hero-Ally"
    LOADER_FUNC "Addmod_Hero_AllyScripts"
    COMMON_SOURCES
        "src/HeroAllyModule.cpp"
    GAMESERVER_SOURCES
        "src/DnHeroAllyController.cpp"
)
