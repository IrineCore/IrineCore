# =============================================================================
# IrineCore Module — modules/FPS-PING/CMakeLists.txt
# IrineCore self-registering module (can be disabled without core edits).
# =============================================================================
irine_register_module(
    NAME "FPS-PING"
    LOADER_FUNC "Addmod_FPS_PINGScripts"
    COMMON_SOURCES
        "src/FpsPingModule.cpp"
)
