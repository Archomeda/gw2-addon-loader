#pragma once
#include "../stdafx.h"

namespace loader {
    namespace disasm {

#pragma pack(push, 1)

        struct JMP_REL {
            // E9 xxxxxxxx
            // JMP near, relative direct
            uint8_t opcode; // E9
            int32_t operand; // Relative destination address
        };

        struct CALL_ABS {
            // FF15 [xxxxxxxx]
            // CALL near, absolute indirect
            uint8_t opcode1; // FF
            uint8_t opcode2; // 15
            int32_t operand; // Relative indirect destination address in x64 to a 64-bit absolute address, absolute indirect destination address in x86 to a 32-bit absolute address
        };

#pragma pack(pop)

    }
}
