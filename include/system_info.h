#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <vector>
#include <string>

struct ProcessInfo {
    int pid;
    std::string name;
    std::string user;
    double cpu_usage;
    double memory_usage;
    long memory_kb;
    std::string state;
};

struct SystemInfo {
    double cpu_usage;
    long total_memory;
    long used_memory;
    long free_memory;
    int running_processes;
    int total_processes;
};

class SystemInfoReader {
public:
    static SystemInfo getSystemInfo();
    static std::vector<ProcessInfo> getProcessList();
    static bool killProcess(int pid);
    
private:
    static double calculateCPUUsage();
    static long getTotalMemory();
    static ProcessInfo getProcessInfo(int pid);
    static std::string getProcessUser(int pid);
};

#endif