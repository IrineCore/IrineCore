# =============================================================================
# IrineCore Client Engine (Windows Win32 Only) — RT Cut Scene Core
# =============================================================================
add_library(client_rtcutscenecore STATIC
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneData.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/IDnCutSceneDataModifier.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/IDnCutSceneDataReader.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutScenePlayer.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/ICustomActorProcessor.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneTable.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/IDnCutSceneRenderObject.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneActor.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneWeapon.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneActionBase.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/CameraData.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneWorld.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneWorldGrid.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneWorldSector.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneActProp.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneLightProp.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/DnCutSceneWorldProp.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/LuaDebugger.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/LuaDelegate.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/IResourcePathFinder.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/RTCutSceneCore/stdafx.cpp"
)
target_include_directories(client_rtcutscenecore PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src/client/engine/EternityEngine"
    "${CMAKE_SOURCE_DIR}/src/client/engine/FMODEngine"
    ${IRINE_CLIENT_COMMON_INCLUDES}
)
target_compile_definitions(client_rtcutscenecore PRIVATE WIN32 NDEBUG _LIB _CLIENT UNICODE _UNICODE)