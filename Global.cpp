#include "Global.h"
#include <mutex>
namespace G {
    static std::mutex logMutex;
    static HANDLE logFile = INVALID_HANDLE_VALUE;
    void Log(const char* fmt, ...) {
        std::lock_guard lock(logMutex);
        if (logFile == INVALID_HANDLE_VALUE) {
            wchar_t path[MAX_PATH]{};
            if (!GetModuleFileNameW(SelfModule, path, MAX_PATH)) return;
            auto slash = wcsrchr(path, L'\\');
            if (!slash) return;
            *(slash + 1) = 0;
            if (wcscat_s(path, L"cs2esp.log")) return;
            logFile = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        }
        if (logFile == INVALID_HANDLE_VALUE) return;
        char buf[1024]{};
        va_list args; va_start(args, fmt);
        int n = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
        va_end(args);
        if (n <= 0) return;
        n = std::min(n, static_cast<int>(sizeof(buf)) - 2);
        buf[n++] = '\n';
        DWORD written = 0;
        WriteFile(logFile, buf, n, &written, nullptr);
    }
    void CloseLog() {
        std::lock_guard lock(logMutex);
        if (logFile != INVALID_HANDLE_VALUE) CloseHandle(logFile);
        logFile = INVALID_HANDLE_VALUE;
    }
}
