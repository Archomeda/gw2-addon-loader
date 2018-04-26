# Guild Wars 2 Addon Loader
***Note:** This is still in early beta. Some features may drastically change over time during their development. Be prepared to frequently reconfigure your installation. Also, expect bugs.*  
***Note:** It has **not** been approved for usage with Guild Wars 2 yet. Use at your own risk.*

Guild Wars 2 Addon Loader is an unofficial addon for Guild Wars 2.
It acts as a master addon that makes it easy to manage your Guild Wars 2 addons without the need to sort the files manually.

**Note:** For now, existing Guild Wars 2 addons have to be adapted in order to make them compatible with *Guild Wars 2 Addon Loader*.
This means that older addons will not work without a few changes by their developers.
There are plans to look into supporting these addons, but due to the complex nature of these addons, it's quite an effort to support them.
Improving stability, applying bugfixes and implementing other missing features have a higher priority.

## Planned features
- Investigating support for legacy addons (aka arcdps, GW2Mounts, ReShade, GW2Hook, and other d3d9 addons)

## Installation
As long as *Guild Wars 2 Addon Loader* is in its early stages, there will be no pre-built binaries available for download.
You'll have to compile them yourself by using Visual Studio 2017.

Once you have the binaries, move the *d3d9.dll* file into *bin* or *bin64* folder inside the Guild Wars 2 installation folder.

### Addons
Any addons should be installed inside the *bin/addons* or *bin64/addons* folder inside the Guild Wars 2 installation folder.
They will be detected on a Guild Wars 2 client restart.

Addons will not be activated by default, you will have to open the addons panel (Alt + Shift + F11) and manually activate them.
Once an addon is working properly, it will be activated automatically upon every Guild Wars 2 client restart.

## Developing addons
The header file *include/gw2addon-native.h* contains information what to export in your DLL in order to make it compatible with *Guild Wars 2 Addon Loader*.
Keep in mind that during this development phase, this file is likely to (significantly) change over time without keeping a version history.
By developing addons against *Guild Wars 2 Addon Loader*, you acknowledge that during this phase, you have to keep your addon up-to-date in order to prevent it from breaking in newer versions.

Inside the *src* folder, there are some examples on how to create native addons.
There are also a few pre-made addons available inside the same folder.

## Third-party software
*Guild Wars 2 Addon Loader* uses a number of third-party software.
Please refer to their GitHub repository or website for more information.

- [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)
- [ImGui 1.53](https://github.com/ocornut/imgui)
- [JSON for Modern C++ 3.1.2](https://github.com/nlohmann/json)
- [miniz 2.0.7](https://github.com/richgel999/miniz)
- [simpleini 4.17](https://github.com/brofield/simpleini)
- [spdlog 0.16.3](https://github.com/gabime/spdlog)
- [Various stb libraries](https://github.com/nothings/stb) (font-atlas-packer-only)
- [zlib 1.2.11](https://zlib.net/) (font-atlas-packer only)

## Disclaimer
This is an **unofficial** library.
The author of this library is not associated with ArenaNet nor with any of its partners.
Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners.
By using this software, you agree that it is at your own risk and that you assume all responsibility.
There is no warranty for using this software.

If you do not agree with this, please do not use this software.
