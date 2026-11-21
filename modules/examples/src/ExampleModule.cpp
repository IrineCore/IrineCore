// =============================================================================
// IrineCore — modules/examples/src/ExampleModule.cpp
// IrineCore module loader entry point for the template module.
// =============================================================================
#include "ExampleModule.h"

namespace Irine {
namespace Modules {

void Addmod_examplesScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new ExampleModule()));
}

} // namespace Modules
} // namespace Irine
