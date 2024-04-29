#ifndef UTILITY_H
#define UTILITY_H

#include <wtypes.h>
#include <windows.h>

// Inline variables introduced in C++17 to handle definitions directly in header files
inline int SCREEN_WIDTH;
inline int SCREEN_HEIGHT;

inline void GetActualResolution(int& horizontal, int& vertical) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(DEVMODE));
    devMode.dmSize = sizeof(DEVMODE);

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        horizontal = devMode.dmPelsWidth;
        vertical = devMode.dmPelsHeight;
    }
    else {
        // Fallback if the EnumDisplaySettings fails
        horizontal = GetSystemMetrics(SM_CXSCREEN);
        vertical = GetSystemMetrics(SM_CYSCREEN);
    }
}

inline void InitializeResolution() {
    GetActualResolution(SCREEN_WIDTH, SCREEN_HEIGHT);
}
#endif // UTILITY_H
