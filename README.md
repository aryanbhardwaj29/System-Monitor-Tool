🚀 System Monitor — C++ Terminal Dashboard

A terminal-based system monitor built in C++ using the ncurses library.
It provides a real-time view of CPU usage, memory consumption, and active processes by reading data from Linux’s /proc filesystem.

🧠 Project Overview

This project recreates the core features of tools like top and htop, but from scratch — to understand how system-level monitoring works internally.
It displays dynamic system statistics inside the terminal with color-coded visuals for better readability.

⚙️ Features

📊 Real-time CPU and Memory tracking

🧩 Live Process List with sorting options

⚡ Color-coded UI for usage levels

🧠 Modular design (System Info, Process Info, UI Manager)

🛠️ Built completely from scratch in C++ using ncurses

🧱 Tech Stack

Language: C++

Libraries: ncurses

Platform: Linux

🧾 How to Run
# Clone the repository
git clone https://github.com/aryanbhardwaj/System-Monitor.git

# Move into the directory
cd System-Monitor

# Compile
g++ -o system_monitor src/*.cpp -Iinclude -lncurses

# Run
./system_monitor

👨‍💻 Author

Name: Aryan Bhardwaj
College: ITER – SOA University
Branch: Computer Science (Data Science)
Project: Wipro Capstone Project 2025 — System Monitor Tool
Email: baryan2910@gmail.com

Build Tool: g++
