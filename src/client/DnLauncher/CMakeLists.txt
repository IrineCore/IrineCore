# =============================================================================
# IrineCore Client (Windows x64 / Win32) — DnLauncher & ResourceUSA
# Uses Static MFC (/MT) + Native Windows GDI+ + EtFileSystem + Utility + TinyXML + zlib
# =============================================================================
set(CMAKE_MFC_FLAG 1)

add_library(ResourceUSA STATIC
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/ResourceUSA/ResourceInit.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/ResourceUSA/stdafx.cpp"
)
target_include_directories(ResourceUSA PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ResourceUSA"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_definitions(ResourceUSA PRIVATE WIN32 NDEBUG _LIB UNICODE _UNICODE)

add_executable(DnLauncher WIN32
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Country/USA/DnLauncherDlgUSA.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Country/USA/DnServiceModuleUSA.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnControlData.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnExplorerDlg.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnFIrstPatchDlg.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnFirstPatchInfo.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnHtmlView.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnLauncher.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnLauncherDlg.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnOptionData.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnOptionDlg.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnPatchInfo.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnPatchThread.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnServiceModule.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/DnUtil.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Extern/ColorStatic.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Extern/LogWnd.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Extern/MD5Checksum.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Extern/SkinProgress.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/Extern/xSkinButton.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/DnLauncher/stdafx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DnLauncher.rc"
)
target_include_directories(DnLauncher PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/Extern"
    "${CMAKE_CURRENT_SOURCE_DIR}/Country/USA"
    "${CMAKE_CURRENT_SOURCE_DIR}/ResourceUSA"
    "${CMAKE_CURRENT_SOURCE_DIR}/ResourceUSA/res"
    "${CMAKE_SOURCE_DIR}/Icons"
    ${IRINE_CLIENT_COMMON_INCLUDES}
)
target_compile_definitions(DnLauncher PRIVATE
    WIN32 _WINDOWS NDEBUG UNICODE _UNICODE
    _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES _CRT_SECURE_NO_WARNINGS
)
target_link_directories(DnLauncher PRIVATE
    "${IRINE_DXSDK_LIB_DIR}"
)
target_link_libraries(DnLauncher PRIVATE
    ResourceUSA
    irine_etfilesystem
    irine_utility
    dep_tinyxml
    dep_zlib
    gdiplus
    legacy_stdio_definitions.lib
    shlwapi wininet setupapi cabinet version dxguid d3d9 winmm
)
set_target_properties(DnLauncher PROPERTIES OUTPUT_NAME "DnLauncher")
target_link_options(DnLauncher PRIVATE
    "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'"
)
install(TARGETS DnLauncher RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")
