//
// Created by dev7z on 16.09.24.
//

// dev7z here, this shit is not created by me. i found it in the deep depths of the internet (cant remember where but i think stackoverflow)

#ifndef CCOLORS_H
#define CCOLORS_H

#include <ostream>
namespace Color {
    enum Code {
        FG_RED          = 31,
        FG_GREEN        = 32,
        FG_YELLOW       = 33,
        FG_BLUE         = 34,
        FG_MAGENTA      = 35,
        FG_CYAN         = 36,
        FG_LIGHTGRAY    = 37,
        FG_DARKGRAY     = 90,
        FG_LIGHTRED     = 91,
        FG_LIGHTGREEN   = 92,
        FG_LIGHTYELLOW  = 93,
        FG_LIGHTBLUE    = 94,
        FG_LIGHTMAGENTA = 95,
        FG_LIGHTCYAN    = 96,
        FG_DEFAULT      = 39,

        BG_RED          = 41,
        BG_GREEN        = 42,
        BG_YELLOW       = 43,
        BG_BLUE         = 44,
        BG_MAGENTA      = 45,
        BG_CYAN         = 46,
        BG_LIGHTGRAY    = 47,
        BG_DARKGRAY     = 100,
        BG_LIGHTRED     = 101,
        BG_LIGHTGREEN   = 102,
        BG_LIGHTYELLOW  = 103,
        BG_LIGHTBLUE    = 104,
        BG_LIGHTMAGENTA = 105,
        BG_LIGHTCYAN    = 106,
        BG_DEFAULT      = 49,
    };
    class Modifier {
        Code code;
    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Modifier& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };
}


#endif //CCOLORS_H
