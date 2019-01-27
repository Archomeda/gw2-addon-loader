#include "encoding.h"

using namespace std;

namespace loader::utils {

    string u8(const wstring& wstr) {
        // For some reason codecvt fails for at least 1 person in a very specific loading stage...
        //return wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wstr);

        // So let's use WinAPI then
        if (wstr.empty()) {
            return string();
        }
        const auto cwstr = wstr.c_str();
        const int length = static_cast<int>(wstr.length());
        const int numChars = static_cast<int>(WideCharToMultiByte(CP_UTF8, 0, cwstr, length, NULL, 0, NULL, NULL));
        string str;
        str.resize(numChars);
        if (WideCharToMultiByte(CP_UTF8, 0, cwstr, length, &str[0], numChars, NULL, NULL)) {
            return str;
        }
        return string();
    }

    wstring u16(const string& str) {
        // For some reason codecvt fails for at least 1 person in a very specific loading stage...
        //return wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(str);

        // So let's use WinAPI then
        if (str.empty()) {
            return wstring();
        }
        const auto cstr = str.c_str();
        const int length = static_cast<int>(str.length());
        const int numChars = static_cast<int>(MultiByteToWideChar(CP_UTF8, 0, cstr, length, NULL, 0));
        wstring wstr;
        wstr.resize(numChars);
        if (MultiByteToWideChar(CP_UTF8, 0, cstr, length, &wstr[0], numChars)) {
            return wstr;
        }
        return wstring();
    }

}
