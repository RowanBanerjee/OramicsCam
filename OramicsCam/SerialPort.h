#pragma once
// Minimal Windows serial helpers for sending a vector to the ESP32.

#define NOMINMAX              // stops windows.h from clashing with std::min/max and OpenCV
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

// Opens e.g. "COM7" at 115200 8N1. Returns INVALID_HANDLE_VALUE on failure.
inline HANDLE openSerial(const std::string& port) {
    std::string path = "\\\\.\\" + port;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not open " << port << " (error " << GetLastError()
            << "). Is a serial monitor holding the port?\n";
        return h;
    }

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(h, &dcb)) { CloseHandle(h); return INVALID_HANDLE_VALUE; }
    dcb.BaudRate = CBR_115200;   // must match Serial.begin() on the ESP32
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(h, &dcb)) { CloseHandle(h); return INVALID_HANDLE_VALUE; }

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 300;
    to.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(h, &to);
    return h;
}

inline std::string readReply(HANDLE h) {
    char buf[1024];
    DWORD got = 0;
    if (ReadFile(h, buf, sizeof(buf) - 1, &got, nullptr) && got > 0) {
        buf[got] = '\0';
        return std::string(buf);
    }
    return "";
}

// Sends one vector as a comma-separated line ending in '\n'.
// If the vector is longer than maxPoints, evenly spaced samples are sent instead
// (the ESP32 sketch currently accepts at most 32 values per line).
inline bool sendVector(HANDLE h, const std::vector<double>& v, size_t maxPoints = 32) {
    if (v.empty()) return false;

    size_t n = v.size() < maxPoints ? v.size() : maxPoints;
    std::string msg;
    char tmp[32];
    for (size_t i = 0; i < n; ++i) {
        size_t idx = (n == 1) ? 0 : i * (v.size() - 1) / (n - 1);  // evenly spaced pick
        std::snprintf(tmp, sizeof(tmp), "%.3f", v[idx]);
        if (i) msg += ',';
        msg += tmp;
    }
    msg += '\n';

    PurgeComm(h, PURGE_RXCLEAR);  // discard the ESP32's echo replies so they don't pile up
    DWORD written = 0;
    return WriteFile(h, msg.c_str(), (DWORD)msg.size(), &written, nullptr) && written == msg.size();
}