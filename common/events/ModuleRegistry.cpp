// =============================================================================
// IrineCore — common/events/ModuleRegistry.cpp
// OpenSource Dragon Nest Server Emulator modular extension interface & registry.
// =============================================================================
#include "ModuleRegistry.h"
#include <cstring>
#include <algorithm>
#if !defined(_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace Irine {
namespace Modules {

namespace {

bool FileExistsRegularW(const std::wstring& path)
{
#if defined(_WIN32)
    DWORD attr = ::GetFileAttributesW(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    std::string narrow = Irine::Platform::NormalizePath(Irine::Platform::WideToUtf8(path.c_str()));
    struct stat sb{};
    return (::stat(narrow.c_str(), &sb) == 0 && !S_ISDIR(sb.st_mode));
#endif
}

std::wstring AsciiToWide(const char* s)
{
    std::wstring out;
    if (!s) return out;
    while (*s)
    {
        out.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*s)));
        ++s;
    }
    return out;
}

bool HasModuleConfigExt(const std::wstring& name)
{
    auto endsWith = [&](const wchar_t* suffix) {
        const size_t slen = std::wcslen(suffix);
        if (name.size() < slen) return false;
        for (size_t i = 0; i < slen; ++i)
        {
            wchar_t a = name[name.size() - slen + i];
            wchar_t b = suffix[i];
            if (a >= L'A' && a <= L'Z') a = static_cast<wchar_t>(a - L'A' + L'a');
            if (a != b) return false;
        }
        return true;
    };
    return endsWith(L".ini") || endsWith(L".ini.dist") || endsWith(L".conf") || endsWith(L".conf.dist");
}

} // namespace

std::wstring ResolveModuleConfigPath(const WCHAR* configDirPath,
                                     const WCHAR* moduleFolderName,
                                     const WCHAR* configFileName)
{
    std::wstring base = (configDirPath && configDirPath[0]) ? configDirPath : L"./module";
    if (!base.empty() && base.back() != L'/' && base.back() != L'\\')
        base.push_back(L'/');

    const std::wstring folder = (moduleFolderName && moduleFolderName[0]) ? moduleFolderName : L"";

    std::vector<std::wstring> candidateFiles;
    if (configFileName && configFileName[0])
    {
        candidateFiles.push_back(configFileName);
        candidateFiles.push_back(std::wstring(configFileName) + L".dist");
    }
    if (!folder.empty())
    {
        candidateFiles.push_back(folder + L".ini");
        candidateFiles.push_back(folder + L".ini.dist");
        candidateFiles.push_back(folder + L".conf");
        candidateFiles.push_back(folder + L".conf.dist");
    }

    for (const auto& file : candidateFiles)
    {
        const std::wstring flatPath = base + file;
        if (FileExistsRegularW(flatPath))
            return flatPath;
    }

    if (!folder.empty())
    {
        for (const auto& file : candidateFiles)
        {
            const std::wstring nestedModPath = base + folder + L"/modules/" + file;
            if (FileExistsRegularW(nestedModPath))
                return nestedModPath;
            const std::wstring nestedConfPath = base + folder + L"/conf/" + file;
            if (FileExistsRegularW(nestedConfPath))
                return nestedConfPath;
        }
    }

    return candidateFiles.empty() ? base : (base + candidateFiles.front());
}

std::wstring ResolveModuleConfigPath(const WCHAR* configDirPath,
                                     const IModule* module)
{
    if (!module)
        return ResolveModuleConfigPath(configDirPath, nullptr, nullptr);

    const std::wstring modName = AsciiToWide(module->GetName());
    const char* explicitFile = module->GetConfigFileName();
    const std::wstring cfgFile = (explicitFile && explicitFile[0]) ? AsciiToWide(explicitFile) : L"";
    return ResolveModuleConfigPath(configDirPath, modName.c_str(), cfgFile.empty() ? nullptr : cfgFile.c_str());
}

std::vector<std::wstring> DiscoverModuleConfigFiles(const WCHAR* configDirPath)
{
    std::vector<std::wstring> results;
    std::wstring base = (configDirPath && configDirPath[0]) ? configDirPath : L"./module";
    if (!base.empty() && base.back() != L'/' && base.back() != L'\\')
        base.push_back(L'/');

#if defined(_WIN32)
    WIN32_FIND_DATAW fd;
    HANDLE hFind = ::FindFirstFileW((base + L"*").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            std::wstring name = fd.cFileName;
            if (HasModuleConfigExt(name))
                results.push_back(base + name);
        } while (::FindNextFileW(hFind, &fd));
        ::FindClose(hFind);
    }
#else
    std::string narrowBase(base.begin(), base.end());
    DIR* dir = ::opendir(narrowBase.c_str());
    if (dir)
    {
        while (struct dirent* ent = ::readdir(dir))
        {
            std::wstring name = AsciiToWide(ent->d_name);
            if (HasModuleConfigExt(name))
                results.push_back(base + name);
        }
        ::closedir(dir);
    }
#endif
    std::sort(results.begin(), results.end());
    return results;
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
