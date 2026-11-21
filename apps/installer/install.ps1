# =============================================================================
# IrineCore — apps/installer/install.ps1
# Bootstraps build environment and runs apps/compiler/compile.ps1
# =============================================================================
& "$PSScriptRoot\..\compiler\compile.ps1" @args
