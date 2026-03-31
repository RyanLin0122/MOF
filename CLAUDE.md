# CLAUDE.md

## Build

User builds with Visual Studio 2022, **Debug | x64**.

```bash
"/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin/MSBuild.exe" MOF.vcxproj "-p:Configuration=Debug" "-p:Platform=x64" "-nologo" "-verbosity:minimal" "-m:1"
```

Note: `-m:1` forces single-threaded build to avoid PDB lock conflicts when VS is open.

Filter for real errors:

```bash
... 2>&1 | grep -iE "error" | grep -v "C1083"
```

## Project structure

- `mofclient.c` — ground truth (decompiled binary), all C++ code is restored from this file
- `inc/` — headers
- `src/` — source files
- Platform: x64, Visual Studio 2022 (v143), Debug, C++17 (/std:c++17), Windows 10 SDK
