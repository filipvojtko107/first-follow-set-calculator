CXX = g++
CXXFLAGS = -std=c++17 
FFL_PATH = ..
SRC_FILES = $(FFL_PATH)/ffl.cpp main.cpp
TARGET = ffl_calculator


.PHONY: all
all: $(TARGET)


$(TARGET): $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -I$(FFL_PATH) $^ -o $@


.PHONY: clean
clean:
	rm -f $(TARGET)
	