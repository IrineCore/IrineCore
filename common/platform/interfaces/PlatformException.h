// =============================================================================
// IrineCore — common/platform/interfaces/PlatformException.h
// Cross-platform structured exception handling (__try/__except vs try/catch)
// and crash dump / backtrace handler installation.
// =============================================================================
#pragma once

#include "PlatformTypes.h"

#if defined(_MSC_VER)
#define IRINE_TRY            __try
#define IRINE_EXCEPT(filter) __except(filter)
#else
#define IRINE_TRY            try
#define IRINE_EXCEPT(filter) catch (...)
#ifndef __try
#define __try                try
#endif
#ifndef __except
#define __except(filter)     catch (...)
#endif
#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION (-1)
#endif
#endif

namespace Irine {
namespace Platform {

void InstallCrashHandler(const char* appName) noexcept;

} // namespace Platform
} // namespace Irine
