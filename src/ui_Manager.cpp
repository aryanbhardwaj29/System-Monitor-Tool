cat > ui_manager.cpp << 'ENHANCED_UI'
#include "ui_manager.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstddef>


using namespace std;

UIManager::UIManager() : current_sort(SORT_CPU), sort_descending(true), 
                        selected_process(0), should_exit(false) {
}

UIManager::~UIManager() {
    if (main_win) delwin(main_win);
    endwin();
}

void UIManager::initializeUI() {
    initscr();
    start_color();  // Enable colors
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);
    
    // Define color pairs
    init_pair(1, COLOR_RED, COLOR_BLACK);     // High usage
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Good
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Warning
    init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Headers
    init_pair(5, COLOR_WHITE, COLOR_BLUE);    // Selection
    
    int height, width;
    getmaxyx(stdscr, height, width);
    main_win = newwin(height, width, 0, 0);
}

void UIManager::mainLoop() {
    while (!should_exit) {
        werase(main_win);
        
        // Get system info
        SystemInfo sys_info = SystemInfoReader::getSystemInfo();
        vector<ProcessInfo> processes = SystemInfoReader::getProcessList();
        
        // Sort processes
        processes = sortProcesses(processes);
        
        // Draw UI
        drawHeader(sys_info);
        drawProcessList(processes);
        drawFooter();
        
        wrefresh(main_win);
        handleInput();
    }
}

void UIManager::drawHeader(const SystemInfo& sys_info) {
    wattron(main_win, A_BOLD);
    wattron(main_win, COLOR_PAIR(4));
    mvwprintw(main_win, 0, 0, " 🚀 SYSTEM MONITOR - Press 'q' to quit | 'k' to kill process ");
    wattroff(main_win, COLOR_PAIR(4));
    wattroff(main_win, A_BOLD);
    
    // CPU usage with color coding
    wattron(main_win, A_BOLD);
    if (sys_info.cpu_usage > 80) {
        wattron(main_win, COLOR_PAIR(1));
    } else if (sys_info.cpu_usage > 60) {
        wattron(main_win, COLOR_PAIR(3));
    } else {
        wattron(main_win, COLOR_PAIR(2));
    }
    mvwprintw(main_win, 1, 0, "💻 CPU Usage: %.1f%%", sys_info.cpu_usage);
    wattroff(main_win, COLOR_PAIR(1));
    wattroff(main_win, COLOR_PAIR(2));
    wattroff(main_win, COLOR_PAIR(3));
    
    // Memory usage with color coding
    double memory_percent = (double)sys_info.used_memory / sys_info.total_memory * 100.0;
    wattron(main_win, A_BOLD);
    if (memory_percent > 85) {
        wattron(main_win, COLOR_PAIR(1));
    } else if (memory_percent > 70) {
        wattron(main_win, COLOR_PAIR(3));
    } else {
        wattron(main_win, COLOR_PAIR(2));
    }
    mvwprintw(main_win, 2, 0, "💾 Memory: %.1fGB / %.1fGB (%.1f%%)", 
             sys_info.used_memory / 1024.0 / 1024.0,
             sys_info.total_memory / 1024.0 / 1024.0,
             memory_percent);
    wattroff(main_win, COLOR_PAIR(1));
    wattroff(main_win, COLOR_PAIR(2));
    wattroff(main_win, COLOR_PAIR(3));
    wattroff(main_win, A_BOLD);
    
    // Process count
    wattron(main_win, COLOR_PAIR(4));
    mvwprintw(main_win, 3, 0, "📊 Processes: %d total, %d running", 
             sys_info.total_processes, sys_info.running_processes);
    wattroff(main_win, COLOR_PAIR(4));
             
    // Separator
    wattron(main_win, COLOR_PAIR(4));
    mvwhline(main_win, 4, 0, '=', getmaxx(main_win));
    wattroff(main_win, COLOR_PAIR(4));
}

void UIManager::drawProcessList(const vector<ProcessInfo>& processes) {
    // Column headers
    wattron(main_win, A_BOLD | A_REVERSE);
    wattron(main_win, COLOR_PAIR(4));
    mvwprintw(main_win, 5, 0, " PID   ");
    mvwprintw(main_win, 5, 8, " USER         ");
    mvwprintw(main_win, 5, 22, " CPU%%  ");
    mvwprintw(main_win, 5, 30, " MEM%%  ");
    mvwprintw(main_win, 5, 38, " MEMORY      ");
    mvwprintw(main_win, 5, 52, " STATE ");
    mvwprintw(main_win, 5, 60, " COMMAND");
    wattroff(main_win, COLOR_PAIR(4));
    wattroff(main_win, A_BOLD | A_REVERSE);
    
    int max_rows = getmaxy(main_win) - 8;
    int display_count = min((int)processes.size(), max_rows);
    
    for (int i = 0; i < display_count; i++) {
        const ProcessInfo& proc = processes[i];
        int row = 6 + i;
        
        // Highlight selected process
        if (i == selected_process) {
            wattron(main_win, COLOR_PAIR(5));
            wattron(main_win, A_BOLD);
        }
        
        // PID
        mvwprintw(main_win, row, 0, " %-5d", proc.pid);
        
        // User (truncate if too long)
        string user_display = proc.user;
        if (user_display.length() > 12) {
            user_display = user_display.substr(0, 9) + "...";
        }
        mvwprintw(main_win, row, 8, " %-12s", user_display.c_str());
        
        // CPU with color
        if (proc.cpu_usage > 50) {
            wattron(main_win, COLOR_PAIR(1));
        } else if (proc.cpu_usage > 20) {
            wattron(main_win, COLOR_PAIR(3));
        }
        mvwprintw(main_win, row, 22, " %5.1f", proc.cpu_usage);
        wattroff(main_win, COLOR_PAIR(1));
        wattroff(main_win, COLOR_PAIR(3));
        
        // Memory with color
        if (proc.memory_usage > 10) {
            wattron(main_win, COLOR_PAIR(1));
        } else if (proc.memory_usage > 5) {
            wattron(main_win, COLOR_PAIR(3));
        }
        mvwprintw(main_win, row, 30, " %5.1f", proc.memory_usage);
        wattroff(main_win, COLOR_PAIR(1));
        wattroff(main_win, COLOR_PAIR(3));
        
        // Smart memory display
        string mem_display;
        if (proc.memory_kb < 1024) {
            mem_display = to_string(proc.memory_kb) + " KB";
        } else if (proc.memory_kb < 1024 * 1024) {
            double mb = proc.memory_kb / 1024.0;
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%.1f MB", mb);
            mem_display = buffer;
        } else {
            double gb = proc.memory_kb / (1024.0 * 1024.0);
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%.1f GB", gb);
            mem_display = buffer;
        }
        mvwprintw(main_win, row, 38, " %-11s", mem_display.c_str());
        
        // State with emoji
        string state_display;
        if (proc.state == "R") state_display = "🏃";
        else if (proc.state == "S") state_display = "💤";
        else if (proc.state == "Z") state_display = "☠️";
        else state_display = "❓";
        state_display += proc.state;
        
        mvwprintw(main_win, row, 52, " %-6s", state_display.c_str());
        
        // Command name
        string name_display = proc.name;
        int max_name_width = getmaxx(main_win) - 61;
        if (name_display.length() > max_name_width) {
            name_display = name_display.substr(0, max_name_width - 3) + "...";
        }
        mvwprintw(main_win, row, 60, " %s", name_display.c_str());
        
        if (i == selected_process) {
            wattroff(main_win, COLOR_PAIR(5));
            wattroff(main_win, A_BOLD);
        }
    }
}

void UIManager::drawFooter() {
    int height = getmaxy(main_win);
    
    // Separator
    wattron(main_win, COLOR_PAIR(4));
    mvwhline(main_win, height - 2, 0, '=', getmaxx(main_win));
    wattroff(main_win, COLOR_PAIR(4));
    
    // Footer with instructions
    wattron(main_win, A_BOLD);
    wattron(main_win, COLOR_PAIR(3));
    
    mvwprintw(main_win, height - 1, 0, 
             "🛠️ Sort: F1(CPU) F2(MEM) F3(PID) | 🔥 Kill: k | 🚪 Quit: q");
    
    wattroff(main_win, COLOR_PAIR(3));
    wattroff(main_win, A_BOLD);
}

void UIManager::handleInput() {
    int ch = getch();
    
    switch (ch) {
        case 'q':
        case 'Q':
            should_exit = true;
            break;
        case KEY_UP:
            if (selected_process > 0) selected_process--;
            break;
        case KEY_DOWN:
            selected_process++;
            break;
        case 'k':
        case 'K': {
            auto processes = SystemInfoReader::getProcessList();
            processes = sortProcesses(processes);
            if (selected_process < processes.size()) {
                int pid_to_kill = processes[selected_process].pid;
                SystemInfoReader::killProcess(pid_to_kill);
            }
            break;
        }
        case KEY_F(1):
            current_sort = SORT_CPU;
            sort_descending = true;
            selected_process = 0;
            break;
        case KEY_F(2):
            current_sort = SORT_MEMORY;
            sort_descending = true;
            selected_process = 0;
            break;
        case KEY_F(3):
            current_sort = SORT_PID;
            sort_descending = false;
            selected_process = 0;
            break;
    }
}

vector<ProcessInfo> UIManager::sortProcesses(vector<ProcessInfo> processes) {
    switch (current_sort) {
        case SORT_CPU:
            sort(processes.begin(), processes.end(), 
                 [](const ProcessInfo& a, const ProcessInfo& b) {
                     return a.cpu_usage > b.cpu_usage;
                 });
            break;
        case SORT_MEMORY:
            sort(processes.begin(), processes.end(), 
                 [](const ProcessInfo& a, const ProcessInfo& b) {
                     return a.memory_usage > b.memory_usage;
                 });
            break;
        case SORT_PID:
            sort(processes.begin(), processes.end(), 
                 [](const ProcessInfo& a, const ProcessInfo& b) {
                     return a.pid < b.pid;
                 });
            break;
        case SORT_NAME:
            sort(processes.begin(), processes.end(), 
                 [](const ProcessInfo& a, const ProcessInfo& b) {
                     return a.name < b.name;
                 });
            break;
    }
    
    return processes;
}
ENHANCED_UI