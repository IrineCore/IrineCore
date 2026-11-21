# =============================================================================
# IrineCore — deps/CMakeLists.txt
# Bundled third-party dependencies (zlib, tinyxml, lua51, etmath).
# =============================================================================
add_subdirectory(zlib)
add_subdirectory(tinyxml)
add_subdirectory(lua51)
add_subdirectory(etmath)

if(IRINE_BUILD_CLIENT)
    add_subdirectory(video)
endif()
