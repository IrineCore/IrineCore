// =============================================================================
// IrineCore — common/events/ModuleRegistry.cpp
// OpenSource Dragon Nest Server Emulator modular extension interface & registry.
// =============================================================================
#include "ModuleRegistry.h"
#include <cstring>

namespace Irine {
namespace Modules {

namespace {

bool FileExistsRegularW(const std::wstring& path)
{
    DWORD attr = ::GetFileAttributesW(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

} // namespace

std::wstring ResolveModuleConfigPath(const WCHAR* configDirPath,
                                     const WCHAR* moduleFolderName,
                                     const WCHAR* configFileName)
{
    std::wstring base = (configDirPath && configDirPath[0]) ? configDirPath : L"./module";
    if (!base.empty() && base.back() != L'/' && base.back() != L'\\')
        base.push_back(L'/');

    const std::wstring file = configFileName ? configFileName : L"";
    const std::wstring flatPath = base + file;
    if (FileExistsRegularW(flatPath))
        return flatPath;

    if (moduleFolderName && moduleFolderName[0])
    {
        std::wstring nestedPath = base + moduleFolderName + L"/modules/" + file;
        if (FileExistsRegularW(nestedPath))
            return nestedPath;
    }

    return flatPath;
}

ModuleRegistry& ModuleRegistry::Instance() noexcept
{
    static ModuleRegistry s_instance;
    return s_instance;
}

void ModuleRegistry::RegisterModule(std::unique_ptr<IModule> module)
{
    if (!module) return;
    if (GetModule(module->GetName()) != nullptr) return;
    m_modules.push_back(std::move(module));
}

bool ModuleRegistry::LoadAllConfigs(const WCHAR* moduleConfigDirPath)
{
    bool allOk = true;
    for (auto& mod : m_modules)
    {
        if (!mod->OnLoadConfig(moduleConfigDirPath))
            allOk = false;
    }
    return allOk;
}

void ModuleRegistry::NotifyServerInitialize()
{
    for (auto& mod : m_modules)
    {
        if (mod->IsEnabled())
            mod->OnServerInitialize();
    }
}

void ModuleRegistry::NotifyServerShutdown()
{
    for (auto& mod : m_modules)
    {
        if (mod->IsEnabled())
            mod->OnServerShutdown();
    }
}

void ModuleRegistry::NotifyStageStart(int mapIndex, int difficulty, int partySize)
{
    for (auto& mod : m_modules)
    {
        if (mod->IsEnabled())
            mod->OnStageStart(mapIndex, difficulty, partySize);
    }
}

void ModuleRegistry::NotifyStageAreaTransition(int fromMapIndex, int toMapIndex)
{
    for (auto& mod : m_modules)
    {
        if (mod->IsEnabled())
            mod->OnStageAreaTransition(fromMapIndex, toMapIndex);
    }
}

void ModuleRegistry::NotifyStageClear(int mapIndex)
{
    for (auto& mod : m_modules)
    {
        if (mod->IsEnabled())
            mod->OnStageClear(mapIndex);
    }
}

IModule* ModuleRegistry::GetModule(const char* name) const noexcept
{
    if (!name) return nullptr;
    for (const auto& mod : m_modules)
    {
        if (std::strcmp(mod->GetName(), name) == 0)
            return mod.get();
    }
    return nullptr;
}

} // namespace Modules
} // namespace Irine
