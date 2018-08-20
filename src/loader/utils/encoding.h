#pragma once
#include "../stdafx.h"

namespace loader::utils {

    std::string u8(const std::wstring& wstr);
    std::wstring u16(const std::string& str);

}
