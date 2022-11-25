OUTPUT = lightpad
CXX = g++
CXXFLAGS = -pipe --std=c++11 -Wall -Wextra -D ENABLE_EXT
all : build
build :
	@echo "[CXX] main.cpp -> $(OUTPUT)"
	@$(CXX) -Os -s $(CXXFLAGS) $(LDFLAGS) main.cpp -o $(OUTPUT)
debug :
	@echo "[CXX] main.cpp -> $(OUTPUT)"
	@$(CXX) -g $(CXXFLAGS) $(LDFLAGS) main.cpp -o $(OUTPUT)
clean :
	@echo "[RM] $(OUTPUT)"
	@rm $(OUTPUT)