<div align="center">
  <pre>
██╗██╗   ██╗██╗  ██╗
██║██║   ██║╚██╗██╔╝
██║██║   ██║ ╚███╔╝ 
██║╚██╗ ██╔╝ ██╔██╗ 
██║ ╚████╔╝ ██╔╝ ██╗
╚═╝  ╚═══╝  ╚═╝  ╚═╝
  </pre>
  <p align="center"> <b>IVX - InVertiX Engine</b></p>
</div>

## Tooling

### Formatting (clang-format)

`.clang-format` in the repo root is the single source of truth (LLVM base, 4 spaces,
80 columns, `Type *name`, braces attached). Everything first-party (`IVX/src`,
`VXEdit/src`, `VXEdit/scripts`, `Sandbox/src`) is already formatted; `deps/` and
vendored code are skipped on purpose.

```powershell
powershell -ExecutionPolicy Bypass -File tools/format.ps1          # rewrite in place
powershell -ExecutionPolicy Bypass -File tools/format.ps1 -Check   # CI-style check
```

### Language server (clangd)

The main build uses the Visual Studio generator, which cannot export a compile
database, so a second Ninja build directory is configured for clangd only:

```powershell
powershell -ExecutionPolicy Bypass -File tools/gen-compile-db.ps1           # Debug
powershell -ExecutionPolicy Bypass -File tools/gen-compile-db.ps1 -Fresh    # wipe + reconfigure
```

It writes `build/clangd/` and copies `compile_commands.json` to the repo root.
Re-run it after adding files or changing `CMakeLists.txt`.

`.clangd` holds a portable fallback (defines + include paths relative to the repo)
so headers still resolve before the database is generated. In VS Code the bundled
`.vscode/settings.json` points clangd at that database, disables the Microsoft C++
IntelliSense engine, and routes `editor.formatOnSave` through clangd, which in turn
uses `.clang-format`.
