# Guild Wars 2 Add-on Loader
**This readme file contains general information about the development.
For the usage information, please check the associated [GitHub pages website](https://archomeda.github.io/gw2-addon-loader/).**

***Note:** This is still in early beta. Some features may drastically change over time during their development. Be prepared to frequently reconfigure your installation. Also, expect bugs.*  
***Note:** It has **not** been approved for usage with Guild Wars 2 yet. Use at your own risk.*

Guild Wars 2 Add-on Loader is an unofficial add-on for Guild Wars 2.
It acts as a master add-on that makes it easy to manage your Guild Wars 2 add-ons without the need to sort the files manually.


## Compiling
This project uses Visual Studio 2017.
There are 4 build configuration settings:
- Debug: Debug version of the add-on loader
- DebugExamples: Debug version of the add-on loader and examples
- Release: Release version of the add-on loader
- Tools: Debug version of the tools to help the development of the add-on loader

All dependencies are included in the project folders.
Please make sure to checkout all git submodules.

## Developing add-ons
The header file *include/gw2addon-native.h* contains information what to export in your DLL in order to make it compatible.
Keep in mind that during this development phase, this file is likely to (significantly) change over time without keeping a version history.
By developing add-ons against the add-on loader, you acknowledge that during this phase, you have to keep your add-on up-to-date in order to prevent it from breaking in newer versions.

Inside the *src* folder, there are some examples on how to create native add-ons.
There are also a few pre-made add-ons available in the same folder.

## Third-party software
This project uses a number of third-party software.
Please refer to their GitHub repositories or websites for more information.

- [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)
- [ImGui 1.67](https://github.com/ocornut/imgui)
- [JSON for Modern C++ 3.5.0](https://github.com/nlohmann/json)
- [minhook 1.3.3](https://github.com/TsudaKageyu/minhook)
- [miniz 2.0.8](https://github.com/richgel999/miniz)
- [simpleini](https://github.com/brofield/simpleini)
- [spdlog 1.3.1](https://github.com/gabime/spdlog)
- [Various stb libraries](https://github.com/nothings/stb) (font-atlas-packer-only)
- [zlib 1.2.11](https://zlib.net/) (font-atlas-packer only)

## Disclaimer
This is an **unofficial** library.
The author of this library is not associated with ArenaNet nor with any of its partners.
Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners.
By using this software, you agree that it is at your own risk and that you assume all responsibility.
There is no warranty for using this software.

If you do not agree with this, please do not use this software.
