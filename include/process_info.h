#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>

class Process {
public:
    Process(int pid);
    int getPid() const;
    std::string getUser() const;
    std::string getCommand() const;
    double getCpuUtilization() const;
    long getMemory() const;
    std::string getState() const;
    
    // For sorting
    bool operator<(const Process& other) const;
    static bool compareByMemory(const Process& a, const Process& b);
    static bool compareByCpu(const Process& a, const Process& b);
    
    void updateCpuUtilization(long active_jiffies, long total_jiffies);

private:
    int pid_;
    std::string user_;
    std::string command_;
    double cpu_utilization_;
    long memory_;
    std::string state_;
    long last_active_jiffies_;
    long last_total_jiffies_;
    
    void updateProcessInfo();
    long getActiveJiffies() const;
    long getTotalJiffies() const;
};

#endif