#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/install-odbc.sh
#
# Installs the SQL Server client stack required by the native Linux servers:
#   - Microsoft ODBC Driver 18 for SQL Server (msodbcsql18)
#   - Microsoft SQL Server command-line tools 18 (sqlcmd, bcp)
#   - unixODBC runtime + development libraries
#   - GSSAPI dependency (Debian)
#
# Supported:
#   Ubuntu 18.04, 20.04, 22.04, 24.04, 26.04
#   Debian 9, 10, 11, 12, 13
#
# Safe to re-run: steps that are already satisfied are skipped.
#
# Used by:
#   ./irine.sh install            (apps/installer/install.sh)
#   deploy/linux/install.sh
#
# Environment:
#   IRINE_SKIP_ODBC=1             Skip this script entirely
#   IRINE_SKIP_MSSQL_TOOLS=1      Install the ODBC driver only (no sqlcmd/bcp)
# =============================================================================

set -euo pipefail

if [[ "${IRINE_SKIP_ODBC:-0}" == "1" ]]; then
    echo "[IrineCore] IRINE_SKIP_ODBC=1 -> skipping Microsoft ODBC Driver 18 installation."
    exit 0
fi

TOOLS_PATH="/opt/mssql-tools18/bin"

if [[ "$(id -u)" -eq 0 ]]; then
    SUDO=()
else
    if ! command -v sudo >/dev/null 2>&1; then
        echo "[ERROR] sudo is required to install packages (or run this script as root)." >&2
        exit 1
    fi
    SUDO=(sudo)
fi

odbc_driver_installed() {
    command -v odbcinst >/dev/null 2>&1 && odbcinst -q -d 2>/dev/null | grep -qi "ODBC Driver 18 for SQL Server"
}

mssql_tools_installed() {
    [[ -x "${TOOLS_PATH}/sqlcmd" && -x "${TOOLS_PATH}/bcp" ]]
}

microsoft_repo_configured() {
    grep -rqs "packages.microsoft.com" /etc/apt/sources.list /etc/apt/sources.list.d/ 2>/dev/null
}

echo ""
echo "============================================================"
echo " IrineCore — Microsoft ODBC Driver 18 / SQL Server tools"
echo "============================================================"

# -----------------------------------------------------------------------------
# Detect and validate the operating system
# -----------------------------------------------------------------------------
if [[ ! -f /etc/os-release ]]; then
    echo "[ERROR] Cannot detect operating system (/etc/os-release missing)." >&2
    exit 1
fi

# shellcheck disable=SC1091
. /etc/os-release
OS_ID="${ID:-unknown}"
OS_VERSION="${VERSION_ID:-unknown}"
echo "[IrineCore] Detected OS: ${OS_ID} ${OS_VERSION}"

case "${OS_ID}" in
    ubuntu)
        case "${OS_VERSION}" in
            18.04|20.04|22.04|24.04|26.04) ;;
            *)
                echo "[ERROR] Ubuntu ${OS_VERSION} is not supported." >&2
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
                echo "[ERROR] Debian ${OS_VERSION} is not supported." >&2
                exit 1
                ;;
        esac
        REPO_VERSION="${DEBIAN_VERSION}"
        ;;
    *)
        echo "[ERROR] Unsupported operating system for automatic ODBC install: ${OS_ID}" >&2
        echo "        Install msodbcsql18 manually: https://learn.microsoft.com/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server" >&2
        exit 1
        ;;
esac

WANT_TOOLS=1
[[ "${IRINE_SKIP_MSSQL_TOOLS:-0}" == "1" ]] && WANT_TOOLS=0

if odbc_driver_installed && { [[ "${WANT_TOOLS}" -eq 0 ]] || mssql_tools_installed; }; then
    echo "[OK] Microsoft ODBC Driver 18 already installed."
    [[ "${WANT_TOOLS}" -eq 1 ]] && echo "[OK] sqlcmd / bcp already installed in ${TOOLS_PATH}."
else
    # -------------------------------------------------------------------------
    # Base prerequisites for adding the Microsoft repository
    # -------------------------------------------------------------------------
    echo ""
    echo "[IrineCore] Installing repository prerequisites..."
    "${SUDO[@]}" apt-get update
    "${SUDO[@]}" apt-get install -y curl ca-certificates gnupg unixodbc unixodbc-dev

    # -------------------------------------------------------------------------
    # Configure the Microsoft package repository (only once)
    # -------------------------------------------------------------------------
    if microsoft_repo_configured; then
        echo "[OK] Microsoft package repository already configured."
    else
        echo ""
        echo "[IrineCore] Configuring Microsoft package repository..."
        TEMP_DEB="$(mktemp --suffix=.deb)"
        trap 'rm -f "${TEMP_DEB:-}"' EXIT
        REPO_URL="https://packages.microsoft.com/config/${OS_ID}/${REPO_VERSION}/packages-microsoft-prod.deb"
        if ! curl -fsSL "${REPO_URL}" -o "${TEMP_DEB}"; then
            echo "[ERROR] Could not download ${REPO_URL}" >&2
            echo "        Microsoft may not publish packages for ${OS_ID} ${OS_VERSION} yet." >&2
            exit 1
        fi
        "${SUDO[@]}" dpkg -i "${TEMP_DEB}"
        rm -f "${TEMP_DEB}"
        TEMP_DEB=""
    fi

    echo ""
    echo "[IrineCore] Updating package lists..."
    "${SUDO[@]}" apt-get update

    # -------------------------------------------------------------------------
    # Microsoft ODBC Driver 18 (+ optional SQL Server tools)
    # -------------------------------------------------------------------------
    echo ""
    echo "[IrineCore] Installing Microsoft ODBC Driver 18..."
    "${SUDO[@]}" env ACCEPT_EULA=Y apt-get install -y msodbcsql18

    if [[ "${WANT_TOOLS}" -eq 1 ]]; then
        echo ""
        echo "[IrineCore] Installing SQL Server command-line tools (sqlcmd, bcp)..."
        "${SUDO[@]}" env ACCEPT_EULA=Y apt-get install -y mssql-tools18
    fi

    if [[ "${OS_ID}" == "debian" ]]; then
        echo ""
        echo "[IrineCore] Installing Debian GSSAPI dependency..."
        "${SUDO[@]}" apt-get install -y libgssapi-krb5-2
    fi
fi

# -----------------------------------------------------------------------------
# Add SQL Server tools to PATH for future shells (once)
# -----------------------------------------------------------------------------
if [[ "${WANT_TOOLS}" -eq 1 ]]; then
    case "$(basename "${SHELL:-bash}")" in
        zsh)  SHELL_RC="${HOME}/.zshrc" ;;
        bash) SHELL_RC="${HOME}/.bashrc" ;;
        *)    SHELL_RC="${HOME}/.profile" ;;
    esac
    touch "${SHELL_RC}"
    if ! grep -qF "${TOOLS_PATH}" "${SHELL_RC}"; then
        echo 'export PATH="$PATH:/opt/mssql-tools18/bin"' >> "${SHELL_RC}"
        echo "[IrineCore] Added ${TOOLS_PATH} to PATH in ${SHELL_RC} (open a new shell to use sqlcmd/bcp)."
    fi
    export PATH="${PATH}:${TOOLS_PATH}"
fi

# -----------------------------------------------------------------------------
# Verify
# -----------------------------------------------------------------------------
echo ""
echo "[IrineCore] Verifying Microsoft ODBC installation..."
if ! odbc_driver_installed; then
    echo "[ERROR] Microsoft ODBC Driver 18 is not registered with unixODBC (odbcinst -q -d)." >&2
    exit 1
fi
echo "[OK] Microsoft ODBC Driver 18 installed."

if [[ "${WANT_TOOLS}" -eq 1 ]]; then
    if mssql_tools_installed; then
        echo "[OK] sqlcmd and bcp installed (${TOOLS_PATH})."
    else
        echo "[ERROR] sqlcmd/bcp were not found in ${TOOLS_PATH}." >&2
        exit 1
    fi
fi
