# =============================================================================
# IrineCore Module — modules/examples/CMakeLists.txt
# Template demonstrating IrineCore self-registering module layout.
# =============================================================================
irine_register_module(
    NAME "examples"
    LOADER_FUNC "Addmod_examplesScripts"
    COMMON_SOURCES
        "src/ExampleModule.cpp"
    NO_CONFIG_INSTALL
)
