#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "system_info.h"
#include <ncurses.h>

enum SortType {
    SORT_CPU,
    SORT_MEMORY,
    SORT_PID,
    SORT_NAME
};

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    void initializeUI();
    void mainLoop();
    
private:
    WINDOW* main_win;
    SortType current_sort;
    bool sort_descending;
    int selected_process;
    bool should_exit;
    
    void drawHeader(const SystemInfo& sys_info);
    void drawProcessList(const vector<ProcessInfo>& processes);
    void drawFooter();
    void handleInput();
    vector<ProcessInfo> sortProcesses(vector<ProcessInfo> processes);
};

#endif