# IrineCore Build & CLI Configuration (`conf/`)

`conf/dist/` holds tracked default build and CLI settings for both Linux (`config.sh`) and Windows (`config.ps1`).

## How to Customize Locally

1. Copy the template for your platform:
   - **Linux / macOS / Git Bash**:
     ```bash
     cp conf/dist/config.sh conf/config.sh
     ```
   - **Windows PowerShell**:
     ```powershell
     Copy-Item conf\dist\config.ps1 conf\config.ps1
     ```
2. Edit `conf/config.sh` or `conf/config.ps1` (both are gitignored).
3. Run `./irine.sh` / `.\irine.ps1` or `apps/compiler/compile.{sh,ps1}` — defaults from `conf/dist/` are loaded first and overridden automatically by your local `conf/config.{sh,ps1}`.

### Common Overrides
- `IRINE_BUILD_TYPE` — `Release`, `RelWithDebInfo`, or `Debug`
- `IRINE_MODULES` — `static` or `disabled`
- `DISABLED_IRINE_MODULES` — semicolon-separated module names to exclude (e.g. `"examples"`)
- `IRINE_CMAKE_EXTRA_ARGS` — extra flags forwarded directly to `cmake` configure
- `IRINE_USE_CCACHE` — enable/disable `ccache` on Linux (`ON` / `OFF`)
- `IRINE_LOG_ROTATE_KEEP` — number of rotated server log archives to retain
