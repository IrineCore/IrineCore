#!/usr/bin/env python3
"""
IrineCore — Plug-and-Play Module Manager
========================================
Manage modules under `modules/` consistently across Windows & Linux.

Commands:
  list                   List all discovered modules and their enabled/disabled state
  enable <name>          Enable a module (removes modules/<name>/.disabled)
  disable <name>         Disable a module without deleting it (creates .disabled marker)
  add <git-url> [name]   Clone an external module repository into modules/<name>
  update [name]          Pull latest changes for git-managed external module(s)
  create <name>          Scaffold a ready-to-build IrineCore starter module
  validate               Validate every modules/*/module.json manifest
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
MODULES_DIR = ROOT_DIR / "modules"


def sanitize_ident(raw: str) -> str:
    ident = re.sub(r"[^A-Za-z0-9_]", "_", raw)
    if ident and ident[0].isdigit():
        ident = "_" + ident
    return ident


def to_pascal(raw: str) -> str:
    parts = [p for p in re.split(r"[^A-Za-z0-9]+", raw) if p]
    return "".join(p[0].upper() + p[1:] for p in parts) or "Custom"


def discover_modules() -> list:
    mods = []
    if not MODULES_DIR.is_dir():
        return mods
    for entry in sorted(p for p in MODULES_DIR.iterdir() if p.is_dir()):
        name = entry.name
        if name.startswith(".") or name == "disabled":
            continue
        if not ((entry / "CMakeLists.txt").exists() or (entry / "src").is_dir()):
            continue
        disabled = (entry / ".disabled").exists()
        meta = {}
        manifest = entry / "module.json"
        if manifest.is_file():
            try:
                meta = json.loads(manifest.read_text(encoding="utf-8"))
            except Exception as exc:
                meta = {"_error": str(exc)}
        mods.append({
            "name": name,
            "path": entry,
            "enabled": not disabled,
            "is_git_repo": (entry / ".git").exists(),
            "meta": meta,
        })
    return mods


def cmd_list(_args) -> int:
    mods = discover_modules()
    if not mods:
        print("[IrineCore Modules] No modules discovered under modules/.")
        return 0

    print(f"[IrineCore Modules] Discovered {len(mods)} module(s):")
    for m in mods:
        state = "ENABLED " if m["enabled"] else "DISABLED"
        meta = m["meta"]
        ver = meta.get("version", "0.0.0")
        servers = ",".join(meta.get("servers", [])) or "common"
        desc = meta.get("description", "(no module.json description)")
        src = " [git]" if m["is_git_repo"] else ""
        print(f"  [{state}] {m['name']} v{ver} ({servers}){src}")
        print(f"             {desc}")
    return 0


def cmd_enable(args) -> int:
    mod_dir = MODULES_DIR / args.name
    if not mod_dir.is_dir():
        print(f"[IrineCore Modules][ERROR] Module not found: {mod_dir}", file=sys.stderr)
        return 1
    marker = mod_dir / ".disabled"
    if marker.exists():
        marker.unlink()
        print(f"[IrineCore Modules] Enabled '{args.name}' (removed .disabled). Rebuild to link it in.")
    else:
        print(f"[IrineCore Modules] Module '{args.name}' is already enabled.")
    return 0


def cmd_disable(args) -> int:
    mod_dir = MODULES_DIR / args.name
    if not mod_dir.is_dir():
        print(f"[IrineCore Modules][ERROR] Module not found: {mod_dir}", file=sys.stderr)
        return 1
    marker = mod_dir / ".disabled"
    if not marker.exists():
        marker.write_text("# Disabled via IrineCore module manager\n", encoding="utf-8")
        print(f"[IrineCore Modules] Disabled '{args.name}' (created .disabled). Rebuild to exclude it.")
    else:
        print(f"[IrineCore Modules] Module '{args.name}' is already disabled.")
    return 0


def cmd_add(args) -> int:
    url = args.url.strip()
    name = args.name
    if not name:
        tail = url.rstrip("/").split("/")[-1]
        name = tail[:-4] if tail.endswith(".git") else tail
    dest = MODULES_DIR / name
    if dest.exists():
        print(f"[IrineCore Modules][ERROR] Target already exists: {dest}", file=sys.stderr)
        return 1
    print(f"[IrineCore Modules] Cloning {url} into modules/{name} ...")
    rc = subprocess.call(["git", "clone", url, str(dest)])
    if rc != 0:
        return rc
    print(f"[IrineCore Modules] Installed module '{name}'.")
    print("  Next steps:")
    print("    1. Rebuild:  ./irine.sh compiler build   (or .\\irine.ps1 compiler build)")
    print("    2. Apply DB: ./irine.sh db update        (if the module includes data/sql/)")
    return 0


def cmd_update(args) -> int:
    mods = discover_modules()
    targets = [m for m in mods if m["is_git_repo"] and (not args.name or m["name"] == args.name)]
    if not targets:
        print("[IrineCore Modules] No external git-managed modules found to update.")
        return 0
    for m in targets:
        print(f"[IrineCore Modules] Updating {m['name']} ...")
        rc = subprocess.call(["git", "-C", str(m["path"]), "pull", "--ff-only"])
        if rc != 0:
            return rc
    print("[IrineCore Modules] Module update complete.")
    return 0


def cmd_create(args) -> int:
    raw_name = args.name.strip()
    if not re.match(r"^[A-Za-z][A-Za-z0-9_-]*$", raw_name):
        print("[IrineCore Modules][ERROR] Module name must start with a letter and contain only A-Z, 0-9, '-', '_'.", file=sys.stderr)
        return 1

    mod_dir = MODULES_DIR / raw_name
    if mod_dir.exists():
        print(f"[IrineCore Modules][ERROR] Module directory already exists: {mod_dir}", file=sys.stderr)
        return 1

    ident = sanitize_ident(raw_name)
    pascal = to_pascal(raw_name)
    loader_fn = f"Addmod_{ident}Scripts"

    (mod_dir / "src").mkdir(parents=True)
    (mod_dir / "modules").mkdir(parents=True)
    (mod_dir / "data" / "sql" / "updates").mkdir(parents=True)

    manifest = {
        "name": raw_name,
        "version": "1.0.0",
        "description": f"Custom IrineCore module '{raw_name}'.",
        "author": "Community",
        "caps": ["Cap-60"],
        "servers": ["DNGameServer"],
        "config": f"modules/{raw_name}.ini.dist",
        "sql_dir": "data/sql",
        "dependencies": [],
    }
    (mod_dir / "module.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    (mod_dir / "CMakeLists.txt").write_text(
        f"# =============================================================================\n"
        f"# IrineCore — modules/{raw_name}/CMakeLists.txt\n"
        f"# =============================================================================\n"
        f"irine_register_module(\n"
        f"    NAME \"{raw_name}\"\n"
        f"    LOADER_FUNC \"{loader_fn}\"\n"
        f"    COMMON_SOURCES\n"
        f"        \"${{CMAKE_CURRENT_SOURCE_DIR}}/src/{pascal}Module.cpp\"\n"
        f")\n",
        encoding="utf-8",
    )

    (mod_dir / "modules" / f"{raw_name}.ini.dist").write_text(
        f"; =============================================================================\n"
        f"; IrineCore — {raw_name} Module Configuration\n"
        f"; Copy to {raw_name}.ini to customize values locally.\n"
        f"; =============================================================================\n"
        f"[{pascal}]\n"
        f"Enable=1\n",
        encoding="utf-8",
    )

    (mod_dir / "src" / f"{pascal}Module.h").write_text(
        f"#pragma once\n"
        f"#include \"ModuleRegistry.h\"\n\n"
        f"namespace Irine {{\n"
        f"namespace Modules {{\n\n"
        f"class {pascal}Module final : public IModule\n"
        f"{{\n"
        f"public:\n"
        f"    const char* GetName() const noexcept override {{ return \"{raw_name}\"; }}\n"
        f"    const char* GetVersion() const noexcept override {{ return \"1.0.0\"; }}\n"
        f"    bool IsEnabled() const noexcept override {{ return m_enabled; }}\n\n"
        f"    bool OnLoadConfig(const WCHAR* moduleConfigDirPath) override;\n"
        f"    void OnServerInitialize() override;\n"
        f"    void OnServerShutdown() override;\n\n"
        f"private:\n"
        f"    bool m_enabled = true;\n"
        f"}};\n\n"
        f"}} // namespace Modules\n"
        f"}} // namespace Irine\n",
        encoding="utf-8",
    )

    (mod_dir / "src" / f"{pascal}Module.cpp").write_text(
        f"#include \"{pascal}Module.h\"\n\n"
        f"namespace Irine {{\n"
        f"namespace Modules {{\n\n"
        f"bool {pascal}Module::OnLoadConfig(const WCHAR* moduleConfigDirPath)\n"
        f"{{\n"
        f"    const std::wstring cfgPath = ResolveModuleConfigPath(moduleConfigDirPath, this);\n"
        f"    m_enabled = (::GetPrivateProfileIntW(L\"{pascal}\", L\"Enable\", 1, cfgPath.c_str()) != 0);\n"
        f"    return true;\n"
        f"}}\n\n"
        f"void {pascal}Module::OnServerInitialize() {{}}\n"
        f"void {pascal}Module::OnServerShutdown() {{}}\n\n"
        f"void {loader_fn}()\n"
        f"{{\n"
        f"    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new {pascal}Module()));\n"
        f"}}\n\n"
        f"}} // namespace Modules\n"
        f"}} // namespace Irine\n",
        encoding="utf-8",
    )

    (mod_dir / "data" / "sql" / "updates" / ".gitkeep").write_text("", encoding="utf-8")
    (mod_dir / "README.md").write_text(
        f"# {raw_name}\n\nCustom IrineCore module scaffolded via `irine module create`.\n",
        encoding="utf-8",
    )

    print(f"[IrineCore Modules] Created starter module at modules/{raw_name}/")
    print("  CMake auto-discovers it on the next configure/build.")
    return 0


def cmd_validate(_args) -> int:
    mods = discover_modules()
    errors = 0
    required_keys = ("name", "version", "description", "author", "caps", "servers")
    for m in mods:
        manifest = m["path"] / "module.json"
        if not manifest.is_file():
            print(f"[FAIL] modules/{m['name']}: missing module.json", file=sys.stderr)
            errors += 1
            continue
        meta = m["meta"]
        if "_error" in meta:
            print(f"[FAIL] modules/{m['name']}/module.json: invalid JSON ({meta['_error']})", file=sys.stderr)
            errors += 1
            continue
        missing = [k for k in required_keys if k not in meta]
        if missing:
            print(f"[FAIL] modules/{m['name']}/module.json: missing keys {missing}", file=sys.stderr)
            errors += 1
            continue
        print(f"[OK]   modules/{m['name']}/module.json (v{meta['version']})")
    return 1 if errors else 0


def main() -> int:
    parser = argparse.ArgumentParser(prog="module_manager.py", description="IrineCore Module Manager")
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("list", help="List all discovered modules").set_defaults(func=cmd_list)

    p_en = sub.add_parser("enable", help="Enable a disabled module")
    p_en.add_argument("name")
    p_en.set_defaults(func=cmd_enable)

    p_dis = sub.add_parser("disable", help="Disable a module without deleting it")
    p_dis.add_argument("name")
    p_dis.set_defaults(func=cmd_disable)

    p_add = sub.add_parser("add", help="Clone an external module repository into modules/")
    p_add.add_argument("url")
    p_add.add_argument("name", nargs="?")
    p_add.set_defaults(func=cmd_add)

    p_upd = sub.add_parser("update", help="Update external git-managed modules")
    p_upd.add_argument("name", nargs="?")
    p_upd.set_defaults(func=cmd_update)

    p_crt = sub.add_parser("create", help="Create a starter module scaffold under modules/")
    p_crt.add_argument("name")
    p_crt.set_defaults(func=cmd_create)

    sub.add_parser("validate", help="Validate all modules/*/module.json manifests").set_defaults(func=cmd_validate)

    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
