## Summary

<!-- Describe what this pull request changes and why. -->

## Category

- [ ] Bug fix
- [ ] New gameplay module (`modules/<ModuleName>/`)
- [ ] Cross-platform / build system improvement
- [ ] Networking / protocol compatibility fix
- [ ] Documentation / deployment / Docker update

## Verification Checklist

- [ ] Compiled cleanly with CMake (`.\apps\compiler\compile.ps1` or `./apps/compiler/compile.sh`)
- [ ] Automated test suite passes (`ctest --test-dir build --output-on-failure`)
- [ ] `cmake --install` stages binaries, `config/`, `module/`, and `systems/` cleanly into `env/dist/`
- [ ] External SQL Server database schema (`DNMembership`, `DNWorld`, `DNServerLog`) remains untouched
- [ ] Client wire packet ABI (`#pragma pack(1)`, `sizeof(WCHAR) == 2`) remains compatible with unmodified `DragonNest.exe`
- [ ] Custom features live inside `modules/<ModuleName>/{src,modules,resource,data,CMakeLists.txt}`
