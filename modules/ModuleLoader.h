// =============================================================================
// IrineCore — modules/ModuleLoader.h
// Registers and initializes all CMake-enabled IrineCore modules.
// =============================================================================
#pragma once

#include "../common/platform/Platform.h"

namespace Irine {
namespace Modules {

void RegisterBuiltinModules();
bool LoadAllModules(const WCHAR* moduleConfigDirPath = L"./module");

} // namespace Modules
} // namespace Irine
