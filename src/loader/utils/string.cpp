#include "string.h"

using namespace std;

namespace loader::utils {

    string cstr2str(char* cstr, int maxLength) {
        // Either take the string until the first null terminator, or take the maximum length to prevent reading past the buffer
        const char* end = find(cstr, cstr + maxLength, '\0');
        return string(cstr, end - cstr);
    }

}
