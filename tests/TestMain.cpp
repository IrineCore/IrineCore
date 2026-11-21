// =============================================================================
// IrineCore — tests/TestMain.cpp
// Main entry point for the IrineCore automated CTest verification suite.
// =============================================================================
#include "IrineCore.h"
#include <cstdio>
#include <string>

int RunUnitTests();
#ifdef IRINE_HAS_TOOLS_TESTS
int RunToolsTests();
#endif
int RunIntegrationTests(const WCHAR* moduleConfigDirPath);
int RunNetworkingTests();
int RunCompatibilityTests(const WCHAR* serverConfigDirPath);

int main(int argc, char* argv[])
{
    Irine::Core::Application::InitializeProcess("IrineCoreTests");
    std::printf("=================================================================\n");
    std::printf(" %s — Automated Test Suite\n", Irine::Core::Application::GetFrameworkVersion());
    std::printf("=================================================================\n");

    std::wstring serverConfigDir = L"./config";
    std::wstring moduleConfigDir = L"./module";
    if (argc > 1 && argv[1])
        serverConfigDir = Irine::Platform::Utf8ToWide(argv[1]);
    if (argc > 2 && argv[2])
        moduleConfigDir = Irine::Platform::Utf8ToWide(argv[2]);

    int totalFailures = 0;
    totalFailures += RunUnitTests();
#ifdef IRINE_HAS_TOOLS_TESTS
    totalFailures += RunToolsTests();
#endif
    totalFailures += RunIntegrationTests(moduleConfigDir.c_str());
    totalFailures += RunNetworkingTests();
    totalFailures += RunCompatibilityTests(serverConfigDir.c_str());

    std::printf("=================================================================\n");
    if (totalFailures == 0)
    {
        std::printf(" ALL IRINECORE TEST SUITES PASSED (0 failures)\n");
        std::printf("=================================================================\n");
        return 0;
    }
    std::fprintf(stderr, " IRINECORE TEST SUITE FAILED (%d failures)\n", totalFailures);
    return 1;
}
