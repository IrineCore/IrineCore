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

class IModule;

// Automatically resolves a module configuration file by probing standard locations & extensions:
//   1. <configDirPath>/<file> (or <ModName>.ini / .conf / .conf.dist)
//   2. <configDirPath>/<moduleFolderName>/modules/<file> (or <ModName>.ini / .conf / .conf.dist)
//   3. <configDirPath>/<moduleFolderName>/conf/<file>    (or <ModName>.ini / .conf / .conf.dist)
std::wstring ResolveModuleConfigPath(const WCHAR* configDirPath,
                                     const WCHAR* moduleFolderName,
                                     const WCHAR* configFileName = nullptr);

// Convenience overload: automatically derives module folder & config name from IModule::GetName()/GetConfigFileName().
std::wstring ResolveModuleConfigPath(const WCHAR* configDirPath,
                                     const IModule* module);

// Dynamically enumerates all .ini / .conf / .conf.dist files inside a module config directory.
std::vector<std::wstring> DiscoverModuleConfigFiles(const WCHAR* configDirPath);

class IModule
{
public:
    virtual ~IModule() = default;

    virtual const char* GetName() const noexcept = 0;
    // Optional override: returning nullptr (default) auto-resolves <GetName()>.ini / .conf / .conf.dist.
    virtual const char* GetConfigFileName() const noexcept { return nullptr; }
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
