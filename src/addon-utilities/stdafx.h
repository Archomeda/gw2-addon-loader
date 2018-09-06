#pragma once

// Suppress deprecation warnings
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // Yeah fuck off here... not in the mood to write my own converter

// Standard
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

// Windows
#define WIN32_LEAN_AND_MEAN
#define NOWINSTYLES
#define NOMENUS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NODRAWTEXT
#define NOKERNEL
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9tex.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>

// Third-party
#include <MinHook.h>
#include <SimpleIni.h>

// Project
#include <gw2addon-native.h>
