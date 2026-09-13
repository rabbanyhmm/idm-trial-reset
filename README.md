# IDM Trial Reset (Native C++ Win32)

Open-source, lightweight Windows utility to reset the 30-day Internet Download Manager (IDM) trial and handle registration data. Rebuilt from scratch in native C++ using the Win32 API and Common Controls v6.

## Features

- **Trial Reset**: Clears expired evaluation state and restores default 30-day trial status.
- **Registration Tool**: Applies custom name registration parameters.
- **Auto Reset**: Background task flag `/trial` for automated startup evaluation resets.
- **Zero External Dependencies**: Pure C++ Win32 application compiled with MSVC (no AutoIt runtime required).
- **Dark/Light High-DPI UI**: Classic tabbed Win32 dialog matching original layout and icons.

## Requirements

- Windows 7, 8, 8.1, 10, 11 (x86 / x64)
- Visual Studio 2022 / C++17 Toolset (for building from source)

## Project Structure

```text
idm-trial-reset/
├── assets/
│   ├── app.ico
│   ├── manifest.xml
│   └── resource.rc
├── include/
│   ├── app.hpp
│   └── ui_helpers.hpp
├── src/
│   ├── app.cpp
│   ├── main.cpp
│   └── ui_helpers.cpp
├── idm-trial-reset.sln
└── idm-trial-reset.vcxproj
```

## Build Instructions

### Visual Studio IDE
Open `idm-trial-reset.sln` in Visual Studio 2022, select **Release | x64** or **Release | x86**, and press **Build Solution** (`Ctrl+Shift+B`).

### MSBuild (Command Line)
```powershell
MSBuild.exe idm-trial-reset.sln /p:Configuration=Release /p:Platform=x64
```

## Command Line Usage

Run background reset mode directly:
```cmd
"IDM Trial Reset.exe" /trial
```

## Disclaimer

This project is developed strictly for educational, technical demonstration, and interoperability research purposes.

- This software is not affiliated with, authorized, maintained, or endorsed by Tonec Inc. or the creators of Internet Download Manager.
- All trademarks, service marks, and company names are the property of their respective owners.
- If you use Internet Download Manager regularly, please support the developers by purchasing a valid license from their official website: [internetdownloadmanager.com](https://www.internetdownloadmanager.com/).
- Use of this software is at your own discretion and risk. The authors and contributors assume no responsibility or liability for any consequences arising from the use of this tool.

## Author & Credits

- **Developer:** [Rabbany](https://github.com/rabbanyhmm)
- **Repository:** [idm-trial-reset](https://github.com/rabbanyhmm/idm-trial-reset)


