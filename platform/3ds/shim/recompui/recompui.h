// Stands in for the desktop UI library's header in the shared host sources
// (src/game/recomp_extension_api.cpp, recomp_data_api.cpp), which only use
// its message box. On the 3DS the runtime's error handler shows it.
#ifndef BK_3DS_RECOMPUI_SHIM_H
#define BK_3DS_RECOMPUI_SHIM_H

#include "ultramodern/error_handling.hpp"

namespace recompui {
    inline void message_box(const char* msg) {
        ultramodern::error_handling::message_box(msg);
    }
}

#endif
