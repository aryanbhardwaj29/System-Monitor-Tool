#include "system_info.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

using namespace std;

SystemInfo SystemInfoReader::getSystemInfo() {
    SystemInfo info;
    
    // Get CPU usage
    info.cpu_usage = calculateCPUUsage();
    
    // Get memory info
    info.total_memory = getTotalMemory();
    
    ifstream meminfo("/proc/meminfo");
    string line;
    long available_memory = 0;
    
    while (getline(meminfo, line)) {
        if (line.find("MemAvailable:") == 0) {
            sscanf(line.c_str(), "MemAvailable: %ld kB", &available_memory);
            break;
        }
    }
    
    info.used_memory = info.total_memory - available_memory;
    info.free_memory = available_memory;
    
    // Get process counts
    auto processes = getProcessList();
    info.total_processes = processes.size();
    info.running_processes = 0;
    for (const auto& proc : processes) {
        if (proc.state == "R") info.running_processes++;
    }
    
    return info;
}

double SystemInfoReader::calculateCPUUsage() {
    ifstream statfile("/proc/stat");
    string line;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    
    getline(statfile, line);
    sscanf(line.c_str(), "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", 
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
    
    long total_idle = idle + iowait;
    long total_non_idle = user + nice + system + irq + softirq + steal;
    long total = total_idle + total_non_idle;
    
    // Simple calculation - in real implementation, you'd compare with previous values
    if (total == 0) return 0.0;
    return (double)total_non_idle / total * 100.0;
}

long SystemInfoReader::getTotalMemory() {
    ifstream meminfo("/proc/meminfo");
    string line;
    long total_memory = 0;
    
    while (getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            sscanf(line.c_str(), "MemTotal: %ld kB", &total_memory);
            break;
        }
    }
    return total_memory;
}

vector<ProcessInfo> SystemInfoReader::getProcessList() {
    vector<ProcessInfo> processes;
    DIR* proc_dir = opendir("/proc");
    struct dirent* entry;
    
    if (!proc_dir) return processes;
    
    while ((entry = readdir(proc_dir))) {
        if (entry->d_type == DT_DIR) {
            char* endptr;
            int pid = strtol(entry->d_name, &endptr, 10);
            
            if (*endptr == '\0') { // Valid PID
                ProcessInfo proc = getProcessInfo(pid);
                if (proc.pid != -1) {
                    processes.push_back(proc);
                }
            }
        }
    }
    
    closedir(proc_dir);
    return processes;
}

ProcessInfo SystemInfoReader::getProcessInfo(int pid) {
    ProcessInfo proc;
    proc.pid = -1; // Mark as invalid initially
    
    string stat_path = "/proc/" + to_string(pid) + "/stat";
    ifstream statfile(stat_path);
    
    if (!statfile) return proc;
    
    string line;
    getline(statfile, line);
    
    istringstream iss(line);
    string token;
    vector<string> tokens;
    
    while (getline(iss, token, ' ')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() < 24) return proc;
    
    proc.pid = pid;
    proc.name = tokens[1];
    if (proc.name.length() > 2 && proc.name[0] == '(' && proc.name.back() == ')') {
        proc.name = proc.name.substr(1, proc.name.length() - 2);
    }
    proc.state = tokens[2];
    
    // Get memory usage
    long rss_pages = stol(tokens[23]);
    proc.memory_kb = rss_pages * sysconf(_SC_PAGE_SIZE) / 1024;
    proc.memory_usage = (double)proc.memory_kb / getTotalMemory() * 100.0;
    
    // Simple CPU usage calculation (you'd need more sophisticated tracking for real usage)
    proc.cpu_usage = 0.0; // Simplified
    
    proc.user = getProcessUser(pid);
    
    return proc;
}

string SystemInfoReader::getProcessUser(int pid) {
    string status_path = "/proc/" + to_string(pid) + "/status";
    ifstream statusfile(status_path);
    string line;
    
    while (getline(statusfile, line)) {
        if (line.find("Uid:") == 0) {
            int uid;
            sscanf(line.c_str(), "Uid: %d", &uid);
            
            struct passwd* pw = getpwuid(uid);
            if (pw) return string(pw->pw_name);
            return to_string(uid);
        }
    }
    return "unknown";
}

bool SystemInfoReader::killProcess(int pid) {
    return kill(pid, SIGTERM) == 0;
}