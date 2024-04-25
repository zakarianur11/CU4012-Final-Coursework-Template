#ifndef UTILITY_H
#define UTILITY_H

#include <wtypes.h>

// Inline variables introduced in C++17 to handle definitions directly in header files
inline int SCREEN_WIDTH;
inline int SCREEN_HEIGHT;

// Declaration and definition of the function directly in the header
inline void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    if (!GetWindowRect(hDesktop, &desktop)) {
        // Optionally handle errors here
        horizontal = 800; // default fallback width
        vertical = 600;  // default fallback height
        return;
    }
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

// Initialize function to set resolution values
inline void InitializeResolution() {
    GetDesktopResolution(SCREEN_WIDTH, SCREEN_HEIGHT);
}

#endif // UTILITY_H
