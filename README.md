# IDM Trial Reset

Native Win32 C++ desktop interface matching the tabbed dialog layout, controls, and icon extracted from the AutoIt script.

## Project Structure

```text
idm-trial-reset/
├── .gitignore
├── CMakeLists.txt
├── README.md
├── assets/
│   ├── app.ico
│   └── resource.rc
├── include/
│   ├── app.hpp
│   └── ui_helpers.hpp
└── src/
    ├── app.cpp
    ├── main.cpp
    └── ui_helpers.cpp
```

## Building Across Architectures

### 64-bit (x64) Build
```cmd
cmd.exe /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"" && rc /fo assets/resource.res assets/resource.rc && cl /EHsc /std:c++17 /Iinclude /Isrc src/main.cpp src/app.cpp src/ui_helpers.cpp assets/resource.res User32.lib Gdi32.lib Comctl32.lib /Fe:""IDM Trial Reset.exe"""
```

### 32-bit (x86) Build
```cmd
cmd.exe /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars32.bat"" && rc /fo assets/resource.res assets/resource.rc && cl /EHsc /std:c++17 /Iinclude /Isrc src/main.cpp src/app.cpp src/ui_helpers.cpp assets/resource.res User32.lib Gdi32.lib Comctl32.lib /Fe:""IDM Trial Reset.exe"""
```

### CMake Build
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Architecture Compatibility
- **32-bit (x86) Build:** Runs on both 32-bit Windows and 64-bit Windows via WOW64.
- **64-bit (x64) Build:** Runs natively on 64-bit Windows.

## Author & Credits
- **Developer:** [Rabbany](https://github.com/rabbanyhmm)
- **Repository:** [idm-trial-reset](https://github.com/rabbanyhmm/idm-trial-reset)

