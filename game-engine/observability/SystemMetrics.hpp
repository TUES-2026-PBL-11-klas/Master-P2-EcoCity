#ifndef SYSTEM_METRICS_HPP
#define SYSTEM_METRICS_HPP

#include <fstream>
#include <string>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
    #pragma comment(lib, "psapi.lib")
    #include <tlhelp32.h>
#else
    #include <unistd.h>
#endif

struct SystemMetrics {
    double cpuPercent;
    long   memUsedKB;
    int    threadCount;
};

struct ProcStat {
    long long processTicks;
    long long wallMs;
};

inline ProcStat readProcStat() {
    ProcStat s{};

    #ifdef _WIN32
        FILETIME creation, exit, kernel, user;
        GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user);
        // FILETIME is in 100-nanosecond intervals — convert to 100ths of a second
        ULARGE_INTEGER k, u;
        k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime;   u.HighPart = user.dwHighDateTime;
        s.processTicks = (k.QuadPart + u.QuadPart) / 100000; // -> 100ths of a sec
    #else
        // Fields 14 and 15 in /proc/self/stat are utime and stime
        std::ifstream f("/proc/self/stat");
        std::string token;
        for (int i = 1; i <= 13; ++i) f >> token; // skip fields 1-13
        long long utime, stime;
        f >> utime >> stime;
        s.processTicks = utime + stime; // already in jiffies (100ths of a sec)
    #endif

    s.wallMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now().time_since_epoch()).count();

    return s;
}

inline SystemMetrics readSystemMetrics(const ProcStat& prev, const ProcStat& curr)
{
    SystemMetrics metrics{};

    long long tickDelta = curr.processTicks - prev.processTicks;
    long long msDelta   = curr.wallMs       - prev.wallMs;

    int cores = 1;
    #ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cores = sysInfo.dwNumberOfProcessors;

        PROCESS_MEMORY_COUNTERS pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
        metrics.memUsedKB    = static_cast<long>(pmc.WorkingSetSize / 1024);

        // Thread count from /proc doesn't exist on Windows — use a snapshot
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            THREADENTRY32 te{ sizeof(te) };
            DWORD pid = GetCurrentProcessId();
            if (Thread32First(snap, &te)) {
                do {
                    if (te.th32OwnerProcessID == pid) ++metrics.threadCount;
                } while (Thread32Next(snap, &te));
            }
            CloseHandle(snap);
        }
    #else
        cores = sysconf(_SC_NPROCESSORS_ONLN);

        std::ifstream status("/proc/self/status");
        std::string line;
        while (std::getline(status, line)) {
            if (line.rfind("VmRSS:", 0) == 0)
                metrics.memUsedKB = std::stol(line.substr(6));
            if (line.rfind("Threads:", 0) == 0)
                metrics.threadCount = std::stoi(line.substr(8));
        }
    #endif

    metrics.cpuPercent = (msDelta > 0) ? (100.0 * tickDelta * 10.0) / msDelta / cores : 0.0;

    return metrics;
}

#endif
