
#include <windows.h>
#include <iostream>

int main() {
    HANDLE volume = CreateFileW(
        L"\\\\.\\C:",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (volume == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open volume. Error: "
                  << GetLastError() << '\n';
        return 1;
    }

    std::cout << "Volume opened successfully!\n";

    CloseHandle(volume);
    return 0;
}
