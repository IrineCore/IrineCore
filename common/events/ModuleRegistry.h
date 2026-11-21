// =============================================================================
// IrineCore — common/events/ModuleRegistry.h
// OpenSource Dragon Nest Server Emulator modular extension interface & registry.
// =============================================================================
#pragma once

#include "../platform/Platform.h"
#include <string>
#include <vector>
#include <memory>

namespace Irine {
namespace Modules {

// Resolves a module configuration file from either:
//   1. <configDirPath>/<configFileName>                 (deployed env/dist/module/<Mod>.ini)
//   2. <configDirPath>/<moduleFolderName>/modules/<file> (source tree modules/<Mod>/modules/<Mod>.ini)
std::wstring ResolveModuleConfigPath(const WCHAR* configDirPath,
                                     const WCHAR* moduleFolderName,
                                     const WCHAR* configFileName);

class IModule
{
public:
    virtual ~IModule() = default;

    virtual const char* GetName() const noexcept = 0;
    virtual const char* GetConfigFileName() const noexcept = 0;
    virtual bool        IsEnabled() const noexcept = 0;

    // Lifecycle hooks
    virtual bool OnLoadConfig(const WCHAR* moduleConfigDirPath) = 0;
    virtual void OnServerInitialize() {}
    virtual void OnServerShutdown() {}

    // Gameplay stage & session hooks
    virtual void OnStageStart(int /*mapIndex*/, int /*difficulty*/, int /*partySize*/) {}
    virtual void OnStageAreaTransition(int /*fromMapIndex*/, int /*toMapIndex*/) {}
    virtual void OnStageClear(int /*mapIndex*/) {}
};

class ModuleRegistry
{
public:
    static ModuleRegistry& Instance() noexcept;

    void RegisterModule(std::unique_ptr<IModule> module);
    bool LoadAllConfigs(const WCHAR* moduleConfigDirPath = L"./module");
    void NotifyServerInitialize();
    void NotifyServerShutdown();

    void NotifyStageStart(int mapIndex, int difficulty, int partySize);
    void NotifyStageAreaTransition(int fromMapIndex, int toMapIndex);
    void NotifyStageClear(int mapIndex);

    IModule* GetModule(const char* name) const noexcept;
    const std::vector<std::unique_ptr<IModule>>& GetModules() const noexcept { return m_modules; }

private:
    ModuleRegistry() = default;
    std::vector<std::unique_ptr<IModule>> m_modules;
};

} // namespace Modules
} // namespace Irine
