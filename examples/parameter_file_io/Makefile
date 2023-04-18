# DLP 3D Scanner SDK directories
SDK_INCLUDE_DIR 	= ../../include
SDK_LIB_DIR 		= ../../lib
SDK_LIBS 			= -l dlp3d_core

#SDK_FLAGS = -I$(SDK_INCLUDE_DIR) -L$(SDK_LIB_DIR) $(SDK_LIBS)
SDK_FLAGS = -I$(SDK_INCLUDE_DIR) -L$(SDK_LIB_DIR) $(SDK_LIBS)

# Compiler instructions
CXX = g++
CXXFLAGS = -Wall 

# Clean up instructions
RM = rm -f

TARGET = parameter_file_io

all: $(TARGET)

clean:
	$(RM) $(TARGET) $(TARGET).exe *.o	

$(TARGET): $(TARGET).cpp
	$(CXX) $(TARGET).cpp $(CXXFLAGS) $(SDK_FLAGS) -o $(TARGET) 

