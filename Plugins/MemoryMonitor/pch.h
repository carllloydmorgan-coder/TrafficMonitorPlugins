// pch.h: the precompiled header.

#ifndef PCH_H
#define PCH_H

#include "framework.h"

#define SAFE_DELETE(p) do \
{\
    if(p != nullptr) \
    { \
        delete p; \
        p = nullptr; \
    } \
} while (false)

#endif //PCH_H
