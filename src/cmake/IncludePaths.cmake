# =============================================================================
# IrineCore — src/cmake/IncludePaths.cmake
# Shared include search paths for core libraries, modules, server targets,
# and Windows-only client/launcher targets.
# =============================================================================
set(IRINE_SHARED_BASE_INCLUDES
    "${CMAKE_SOURCE_DIR}/common"
    "${CMAKE_SOURCE_DIR}/common/core"
    "${CMAKE_SOURCE_DIR}/common/platform"
    "${CMAKE_SOURCE_DIR}/common/platform/interfaces"
    "${CMAKE_SOURCE_DIR}/common/networking"
    "${CMAKE_SOURCE_DIR}/common/database"
    "${CMAKE_SOURCE_DIR}/common/logging"
    "${CMAKE_SOURCE_DIR}/common/configuration"
    "${CMAKE_SOURCE_DIR}/common/threading"
    "${CMAKE_SOURCE_DIR}/common/synchronization"
    "${CMAKE_SOURCE_DIR}/common/events"
    "${CMAKE_SOURCE_DIR}/common/utilities"
    "${CMAKE_SOURCE_DIR}/common/memory"
    "${CMAKE_SOURCE_DIR}/common/Protocol"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil"
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem"
    "${CMAKE_SOURCE_DIR}/common/EtResManager"
    "${CMAKE_SOURCE_DIR}/common/EtStringManager"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase"
    "${CMAKE_SOURCE_DIR}/common/EtActionCoreBase"
    "${CMAKE_SOURCE_DIR}/common/VoiceChat"
    "${CMAKE_SOURCE_DIR}/deps/lua51"
    "${CMAKE_SOURCE_DIR}/deps/tinyxml"
    "${CMAKE_SOURCE_DIR}/deps/zlib"
    "${CMAKE_SOURCE_DIR}/deps/boost"
    "${CMAKE_SOURCE_DIR}/game"
    "${CMAKE_SOURCE_DIR}/game/characters"
    "${CMAKE_SOURCE_DIR}/game/inventory"
    "${CMAKE_SOURCE_DIR}/game/items"
    "${CMAKE_SOURCE_DIR}/game/equipment"
    "${CMAKE_SOURCE_DIR}/game/skills"
    "${CMAKE_SOURCE_DIR}/game/combat"
    "${CMAKE_SOURCE_DIR}/game/monsters"
    "${CMAKE_SOURCE_DIR}/game/quests"
    "${CMAKE_SOURCE_DIR}/game/maps"
    "${CMAKE_SOURCE_DIR}/game/dungeons"
    "${CMAKE_SOURCE_DIR}/game/parties"
    "${CMAKE_SOURCE_DIR}/game/guilds"
)

set(IRINE_COMMON_INCLUDES
    ${IRINE_SHARED_BASE_INCLUDES}
    "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv"
    "${CMAKE_SOURCE_DIR}/common/VoiceChat/DolbyAxon/IceLib64"
    "${CMAKE_SOURCE_DIR}/deps/etmath"
    "${CMAKE_SOURCE_DIR}/modules"
    "${CMAKE_SOURCE_DIR}/src/server/shared"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon"
)

set(IRINE_CLIENT_COMMON_INCLUDES
    ${IRINE_SHARED_BASE_INCLUDES}
    "${CMAKE_SOURCE_DIR}/common/VoiceChat/DolbyAxon/ClientLib"
    "${CMAKE_SOURCE_DIR}/src/client/engine"
    "${CMAKE_SOURCE_DIR}/src/client/engine/EternityEngine"
    "${CMAKE_SOURCE_DIR}/src/client/engine/EternityEngine/D3DDevice9"
    "${CMAKE_SOURCE_DIR}/src/client/engine/Input"
)
