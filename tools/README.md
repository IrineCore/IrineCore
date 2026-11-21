# IrineCore Packaging & Patching Tools

Native x64 command-line utilities for packing Dragon Nest `.pak` archives, creating ready-to-play clients from unpacked `GameRes` folders, generating incremental and full patches, and verifying MD5 checksums.

All tools are built automatically with CMake (`-DIRINE_BUILD_TOOLS=ON`, enabled by default) and installed to `env/dist/tools/`.

---

## 1. `DnClientCreator` — Client Builder & `.pak` Manager

Packs a complete unpacked `GameRes` / asset directory into split `Resource00.pak`, `Resource01.pak`, ... archives (v11 `CEtPackingFile` format, 512 MiB default split), embeds `\version.cfg` inside `Resource00.pak`, writes `Version.cfg`, and copies binaries/configs into a ready-to-play client folder.

```bash
# Build a complete packed client from an unpacked GameRes directory + built binaries
DnClientCreator --input ./GameRes --bin-dir ./env/client --out ./dist-client --version 354 --md5 --verify-output

# Overlay region assets (Resource_USA / MapData_USA on top of Resource / MapData)
DnClientCreator --input ./GameRes --nation USA --out ./dist-client --version 354 --max-pak-mb 512

# Pack an arbitrary folder directly into ResourceXX.pak under \resource\
DnClientCreator --input ./MyResourceFolder --pack-all --virtual-root Resource --out ./dist-client

# Inspect, verify, extract, or optimize existing .pak archives
DnClientCreator --list ./dist-client/Resource00.pak
DnClientCreator --verify ./dist-client
DnClientCreator --extract ./dist-client --out ./unpacked-gameres
DnClientCreator --optimize ./dist-client/Resource00.pak
```

---

## 2. `PatchBuilder` — Incremental Patch Builder

Produces versioned patches compatible with `DnLauncher.exe` and `deploy/webserver/scripts/sync-patches.sh`:
- `<patch-root>/<0000000N>/Patch<0000000N>.pak`
- `<patch-root>/<0000000N>/Patch<0000000N>.pak.md5`
- `<patch-root>/<0000000N>/Patch<0000000N>.txt` (`D` deletes and `C` root-copy entries, with `DnLauncher.exe` ordered last)
- `<patch-root>/PatchInfoServer.cfg`

```bash
# Build a patch from a staging folder of changed files
PatchBuilder --input ./patch-staging --out ./patches --version 355 --verify

# Automatically diff an older client (packed or unpacked) against a new GameRes build
PatchBuilder --old ./client-v354 --new ./gameres-v355 --out ./patches --verify
```

---

## 3. `DnMakeFullPatch` — Full Patch & Patch Merger

Creates a full-patch distribution (`<out>/<relpath>.zip`, `fullpatch_svr.cfg`, `fullpatch_svr.md5`) or consolidates a sequence of incremental `PatchXXXXXXXX.pak` files into a single archive without recompressing.

```bash
# Build full-patch zip tree + fullpatch_svr.cfg + fullpatch_svr.md5
DnMakeFullPatch ./dist-client ./fullpatch-out

# Merge incremental patches v350..v355 into a single consolidated .pak
DnMakeFullPatch --merge-paks ./patches --from 350 --to 355 --out ./PatchMerged.pak
```

---

## 4. `DnMakeMD5` — MD5 Checksum Generator & Verifier

Generates `<file>.md5` (or `.MD5`) sidecars and consolidated manifests, and verifies files or manifests.

```bash
# Single or multiple files (drop-in compatible with legacy DnMakeMD5.exe)
DnMakeMD5 Patch00000355.pak

# Batch-generate checksums or a manifest for a directory
DnMakeMD5 --dir ./dist-client --ext .pak --manifest ./dist-client/ClientMD5List.txt

# Verify a file against its .md5 sidecar, or verify an entire manifest
DnMakeMD5 --verify Patch00000355.pak
DnMakeMD5 --verify ./dist-client/ClientMD5List.txt
```
