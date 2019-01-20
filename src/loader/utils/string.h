#pragma once
#include "../stdafx.h"

namespace loader::utils {

    std::string cstr2str(char* cstr, int length);

    template<typename T>
    std::string to_string_with_precision(const T value, const int precision) {
        std::ostringstream ss;
        ss.precision(precision);
        ss << std::fixed << value;
        return ss.str();
    }

}
