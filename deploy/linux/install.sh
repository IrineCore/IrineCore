#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/install.sh
#
# Installs prerequisites:
#   Ubuntu 18.04, 20.04, 22.04, 24.04, 26.04
#   Debian 9, 10, 11, 12, 13
#
# Installs:
#   - Microsoft ODBC Driver 18
#   - Microsoft SQL Server Tools 18 (sqlcmd, bcp)
#   - unixODBC development libraries
#   - GSSAPI dependency (Debian)
#   - CMake, Ninja, GCC/G++, and build dependencies
#
# Builds:
#   - IrineCore Release x86_64
#   - Uses -fshort-wchar
# =============================================================================

set -euo pipefail

# =============================================================================
# Directory Configuration
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

echo ""
echo "============================================================"
echo " IrineCore Linux Installer"
echo "============================================================"

# =============================================================================
# Detect Operating System
# =============================================================================

if [ ! -f /etc/os-release ]; then
    echo "[ERROR] Cannot detect operating system."
    exit 1
fi

. /etc/os-release

OS_ID="${ID}"
OS_VERSION="${VERSION_ID}"

echo "[IrineCore] Detected OS: ${OS_ID} ${OS_VERSION}"

# =============================================================================
# Validate Supported OS Versions
# =============================================================================

case "${OS_ID}" in
    ubuntu)
        case "${OS_VERSION}" in
            18.04|20.04|22.04|24.04|26.04) ;;
            *)
                echo "[ERROR] Ubuntu ${OS_VERSION} is not supported."
                exit 1
                ;;
        esac

        REPO_VERSION="${OS_VERSION}"
        ;;

    debian)
        DEBIAN_VERSION="${OS_VERSION%%.*}"

        case "${DEBIAN_VERSION}" in
            9|10|11|12|13) ;;
            *)
                echo "[ERROR] Debian ${OS_VERSION} is not supported."
                exit 1
                ;;
        esac

        REPO_VERSION="${DEBIAN_VERSION}"
        ;;

    *)
        echo "[ERROR] Unsupported operating system: ${OS_ID}"
        exit 1
        ;;
esac

# =============================================================================
# Check Required Commands
# =============================================================================

for cmd in curl dpkg apt-get cmake; do
    if ! command -v "${cmd}" >/dev/null 2>&1; then
        if [ "${cmd}" != "cmake" ]; then
            echo "[IrineCore] Missing command: ${cmd}"
        fi
    fi
done

# =============================================================================
# Install Base Build Dependencies
# =============================================================================

echo ""
echo "[IrineCore] Installing Linux build prerequisites..."

sudo apt-get update

sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    curl \
    ca-certificates \
    gnupg \
    unixodbc \
    unixodbc-dev \
    zlib1g-dev

# =============================================================================
# Configure Microsoft Repository
# =============================================================================

echo ""
echo "[IrineCore] Configuring Microsoft package repository..."

TEMP_DEB="$(mktemp --suffix=.deb)"

trap 'rm -f "${TEMP_DEB:-}"' EXIT

curl -fsSL \
    "https://packages.microsoft.com/config/${OS_ID}/${REPO_VERSION}/packages-microsoft-prod.deb" \
    -o "${TEMP_DEB}"

sudo dpkg -i "${TEMP_DEB}"

rm -f "${TEMP_DEB}"
TEMP_DEB=""

# =============================================================================
# Update Package Lists
# =============================================================================

echo ""
echo "[IrineCore] Updating Microsoft package lists..."

sudo apt-get update

# =============================================================================
# Install Microsoft ODBC Driver 18
# =============================================================================

echo ""
echo "[IrineCore] Installing Microsoft ODBC Driver 18..."

sudo ACCEPT_EULA=Y apt-get install -y msodbcsql18

# =============================================================================
# Install SQL Server Command Line Tools
# =============================================================================

echo ""
echo "[IrineCore] Installing SQL Server command-line tools..."

sudo ACCEPT_EULA=Y apt-get install -y mssql-tools18

# =============================================================================
# Install Debian GSSAPI Dependency
# =============================================================================

if [ "${OS_ID}" = "debian" ]; then
    echo ""
    echo "[IrineCore] Installing Debian GSSAPI dependency..."

    sudo apt-get install -y libgssapi-krb5-2
fi

# =============================================================================
# Configure Bash / Zsh PATH
# =============================================================================

echo ""
echo "[IrineCore] Configuring SQL Server tools PATH..."

TOOLS_PATH="/opt/mssql-tools18/bin"

case "$(basename "${SHELL:-bash}")" in
    zsh)
        SHELL_RC="${HOME}/.zshrc"
        ;;
    bash)
        SHELL_RC="${HOME}/.bashrc"
        ;;
    *)
        SHELL_RC="${HOME}/.profile"
        ;;
esac

# Create configuration file if missing
touch "${SHELL_RC}"

# Add PATH only once
if ! grep -qF "${TOOLS_PATH}" "${SHELL_RC}"; then
    echo 'export PATH="$PATH:/opt/mssql-tools18/bin"' >> "${SHELL_RC}"
fi

# Update current installer environment
export PATH="${PATH}:${TOOLS_PATH}"

# Source shell configuration
if [ -f "${SHELL_RC}" ]; then
    # shellcheck disable=SC1090
    source "${SHELL_RC}"
fi

# =============================================================================
# Verify Microsoft ODBC Installation
# =============================================================================

echo ""
echo "[IrineCore] Verifying Microsoft ODBC installation..."

if ! odbcinst -q -d | grep -qi "ODBC Driver 18"; then
    echo "[ERROR] Microsoft ODBC Driver 18 installation failed."
    exit 1
fi

echo "[OK] Microsoft ODBC Driver 18 installed."

# Verify sqlcmd
if command -v sqlcmd >/dev/null 2>&1; then
    echo "[OK] sqlcmd installed."
else
    echo "[ERROR] sqlcmd was not found."
    exit 1
fi

# Verify bcp
if command -v bcp >/dev/null 2>&1; then
    echo "[OK] bcp installed."
else
    echo "[ERROR] bcp was not found."
    exit 1
fi

# =============================================================================
# Configure CMake
# =============================================================================

echo ""
echo "[IrineCore] Configuring CMake..."
echo "[IrineCore] Build type: Release"
echo "[IrineCore] Architecture: x86_64"
echo "[IrineCore] Compiler flag: -fshort-wchar"

cmake -S "${ROOT_DIR}" \
      -B "${BUILD_DIR}" \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Release

# =============================================================================
# Build IrineCore
# =============================================================================

echo ""
echo "[IrineCore] Building all IrineCore server binaries..."

cmake --build "${BUILD_DIR}" --parallel

# =============================================================================
# Verify Build Output
# =============================================================================

echo ""
echo "[IrineCore] Verifying build output..."

if [ ! -d "${BUILD_DIR}/bin" ]; then
    echo "[ERROR] Build output directory not found: ${BUILD_DIR}/bin/"
    exit 1
fi

echo "[OK] Build output directory exists."

# =============================================================================
# Installation Summary
# =============================================================================

echo ""
echo "============================================================"
echo " IrineCore Installation Completed Successfully!"
echo "============================================================"
echo ""
echo " Operating System : ${OS_ID} ${OS_VERSION}"
echo " ODBC Driver      : Microsoft ODBC Driver 18"
echo " SQL Tools        : mssql-tools18"
echo " SQL PATH         : ${TOOLS_PATH}"
echo " Shell Config     : ${SHELL_RC}"
echo " Build Type       : Release"
echo " Architecture     : x86_64"
echo " Compiler Flag    : -fshort-wchar"
echo " Build Directory  : ${BUILD_DIR}/"
echo " Binary Directory : ${BUILD_DIR}/bin/"
echo ""
echo " Available Commands:"
echo "   sqlcmd -?"
echo "   bcp -?"
echo ""
echo "============================================================"
echo " IrineCore Linux build completed!"
echo "============================================================"
