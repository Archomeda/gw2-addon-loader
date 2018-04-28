#pragma once
#include <MinHook.h>

template<typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, T* pDetour, T** ppOriginal) {
    return MH_CreateHook(pTarget, reinterpret_cast<LPVOID>(pDetour), reinterpret_cast<LPVOID*>(ppOriginal));
}

template<typename T>
inline MH_STATUS MH_CreateHookApiEx(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, T** ppOriginal) {
    return MH_CreateHookApi(pszModule, pszProcName, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}
