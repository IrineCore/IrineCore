// =============================================================================
// IrineCore — modules/examples/src/ExampleModule.h
// Template demonstrating how to create an IrineCore module.
// =============================================================================
#pragma once

#include "ModuleRegistry.h"

namespace Irine {
namespace Modules {

class ExampleModule final : public IModule
{
public:
    const char* GetName() const noexcept override { return "Example-Module"; }
    const char* GetConfigFileName() const noexcept override { return "Example-Module.ini"; }
    bool        IsEnabled() const noexcept override { return m_enabled; }

    bool OnLoadConfig(const WCHAR* /*moduleConfigDirPath*/) override { return true; }
    void OnServerInitialize() override {}
    void OnServerShutdown() override {}

private:
    bool m_enabled = false;
};

} // namespace Modules
} // namespace Irine
