#include "ThreadInfo.h"

using namespace std;

namespace loader::diagnostics {

    const string ProcessPriorityToString(ProcessPriority priority) {
        switch (priority) {
        case ProcessPriority::ProcessIdle:
            return "Idle";
        case ProcessPriority::ProcessBelowNormal:
            return "Below Normal";
        case ProcessPriority::ProcessNormal:
            return "Normal";
        case ProcessPriority::ProcessAboveNormal:
            return "Above Normal";
        case ProcessPriority::ProcessHigh:
            return "High";
        case ProcessPriority::ProcessRealtime:
            return "Realtime";
        default:
            return to_string(priority);
        }
    }

    const string ThreadPriorityToString(ThreadPriority priority) {
        switch (priority) {
        case ThreadPriority::ThreadIdle:
            return "Idle";
        case ThreadPriority::ThreadLowest:
            return "Lowest";
        case ThreadPriority::ThreadBelowNormal:
            return "Below Normal";
        case ThreadPriority::ThreadNormal:
            return "Normal";
        case ThreadPriority::ThreadAboveNormal:
            return "Above Normal";
        case ThreadPriority::ThreadHighest:
            return "Highest";
        case ThreadPriority::ThreadTimeCritical:
            return "Realtime";
        default:
            return to_string(priority);
        }
    }

    ThreadPriority ThreadInfo::DetermineThreadPriority(ProcessPriority processPriority, DWORD priority) {
        /*
                 THREAD |   Idle   |  Lowest  |  Below   |  Normal  |  Above   | Highest  | Time
           PROCESS      |          |          |  Normal  |          |  Normal  |          | Critical
          --------------|----------|----------|----------|----------|----------|----------|----------
           Idle         |    1     |    2     |    3     |    4     |    5     |    6     |    15
           Below Normal |    1     |    4     |    5     |    6     |    7     |    8     |    15
           Normal       |    1     |    6     |    7     |    8     |    9     |    10    |    15
           Above Normal |    1     |    8     |    9     |    10    |    11    |    12    |    15
           High         |    1     |    11    |    12    |    13    |    14    |    15    |    15
           Realtime     |    16    |    22    |    23    |    24    |    25    |    26    |    31
        */

        switch (priority) {
        case 1:
            if (processPriority != ProcessPriority::ProcessIdle) {
                return ThreadPriority::ThreadIdle;
            }
            break;
        case 15:
            if (processPriority != ProcessPriority::ProcessRealtime) {
                return ThreadPriority::ThreadTimeCritical;
            }
            break;
        case 16:
            if (processPriority == ProcessPriority::ProcessRealtime) {
                return ThreadPriority::ThreadIdle;
            }
            break;
        case 31:
            if (processPriority == ProcessPriority::ProcessIdle) {
                return ThreadPriority::ThreadTimeCritical;
            }
            break;
        }
        
        DWORD modPriority = priority - static_cast<DWORD>(processPriority);
        switch (modPriority) {
        case THREAD_PRIORITY_LOWEST:
            return ThreadPriority::ThreadLowest;
        case THREAD_PRIORITY_BELOW_NORMAL:
            return ThreadPriority::ThreadBelowNormal;
        case THREAD_PRIORITY_NORMAL:
            return ThreadPriority::ThreadNormal;
        case THREAD_PRIORITY_ABOVE_NORMAL:
            return ThreadPriority::ThreadAboveNormal;
        case THREAD_PRIORITY_HIGHEST:
            return ThreadPriority::ThreadHighest;
        default:
            return static_cast<ThreadPriority>(priority);
        }
    }

}
