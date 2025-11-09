#include "ui_manager.h"
#include <iostream>

int main() {
    try {
        UIManager ui_manager;
        ui_manager.initializeUI();
        ui_manager.mainLoop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}