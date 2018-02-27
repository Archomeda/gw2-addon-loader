#pragma once
#include <string>

namespace loader {
    namespace utils {

        std::string u8(const std::wstring& wstr);
        std::wstring u16(const std::string& str);

    }
}
