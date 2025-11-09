CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LIBS = -lncurses

SRCS = main.cpp system_info.cpp ui_manager.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = system_monitor

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean