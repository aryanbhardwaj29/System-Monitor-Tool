#include "../include/process.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <pwd.h>
#include <algorithm>
#include <cmath>

Process::Process(int pid) : pid_(pid), cpu_utilization_(0.0), last_active_jiffies_(0), last_total_jiffies_(0) {
    updateProcessInfo();
}

void Process::updateProcessInfo() {
    // Read process status from /proc/[pid]/status
    std::stringstream path;
    path << "/proc/" << pid_ << "/status";
    
    std::ifstream status_file(path.str());
    if (status_file.is_open()) {
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.find("Name:") == 0) {
                command_ = line.substr(6);
            } else if (line.find("State:") == 0) {
                state_ = line.substr(7, 1);
            } else if (line.find("VmRSS:") == 0) {
                // Memory in kB
                std::string mem_str = line.substr(7);
                mem_str = mem_str.substr(0, mem_str.find(" kB"));
                memory_ = std::stol(mem_str) * 1024; // Convert to bytes
            } else if (line.find("Uid:") == 0) {
                // Get user from UID
                std::string uid_str = line.substr(5);
                uid_str = uid_str.substr(0, uid_str.find("\t"));
                uid_t uid = std::stoi(uid_str);
                struct passwd *pw = getpwuid(uid);
                if (pw) {
                    user_ = pw->pw_name;
                } else {
                    user_ = uid_str;
                }
            }
        }
        status_file.close();
    }
}

long Process::getActiveJiffies() const {
    std::stringstream path;
    path << "/proc/" << pid_ << "/stat";
    std::ifstream stat_file(path.str());
    
    if (stat_file.is_open()) {
        std::string line;
        std::getline(stat_file, line);
        std::istringstream iss(line);
        
        // Skip the first 13 fields
        for (int i = 0; i < 13; ++i) {
            std::string dummy;
            iss >> dummy;
        }
        
        // Read utime, stime, cutime, cstime
        long utime, stime, cutime, cstime;
        iss >> utime >> stime >> cutime >> cstime;
        
        return utime + stime + cutime + cstime;
    }
    return 0;
}

long Process::getTotalJiffies() const {
    std::ifstream stat_file("/proc/stat");
    if (stat_file.is_open()) {
        std::string line;
        std::getline(stat_file, line); // First line: cpu total
        std::istringstream iss(line);
        
        std::string cpu;
        iss >> cpu;
        
        long total = 0;
        long time;
        while (iss >> time) {
            total += time;
        }
        return total;
    }
    return 0;
}

void Process::updateCpuUtilization(long active_jiffies, long total_jiffies) {
    if (last_active_jiffies_ != 0 && last_total_jiffies_ != 0) {
        long active_diff = active_jiffies - last_active_jiffies_;
        long total_diff = total_jiffies - last_total_jiffies_;
        
        if (total_diff > 0) {
            cpu_utilization_ = static_cast<double>(active_diff) / total_diff * 100.0;
            cpu_utilization_ = std::min(100.0, std::max(0.0, cpu_utilization_));
        }
    }
    
    last_active_jiffies_ = active_jiffies;
    last_total_jiffies_ = total_jiffies;
}

int Process::getPid() const { return pid_; }
std::string Process::getUser() const { return user_; }
std::string Process::getCommand() const { return command_; }
double Process::getCpuUtilization() const { return cpu_utilization_; }
long Process::getMemory() const { return memory_; }
std::string Process::getState() const { return state_; }

bool Process::operator<(const Process& other) const {
    return memory_ > other.memory_; // Sort by memory (descending)
}

bool Process::compareByMemory(const Process& a, const Process& b) {
    return a.memory_ > b.memory_;
}

bool Process::compareByCpu(const Process& a, const Process& b) {
    return a.cpu_utilization_ > b.cpu_utilization_;
}