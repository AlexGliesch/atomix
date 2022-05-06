CC = g++
DEFINES = 
override CFLAGS += -Wall -Wextra -pedantic -std=c++1z -MD -MP -O3 $(DEFINES)
LFLAGS =
INCLUDES =
LIBS = -lpthread -lstdc++
SRC = $(wildcard *.cpp BlossomMatching/*.cpp BlossomMatching/GEOM/*.cpp BlossomMatching/MinCost/*.cpp)
HEADERS = $(wildcard *.h *.inl)
OBJS = $(SRC:.cpp=.o)
TARGET = atomix
RM = rm -f *.o *.d $(TARGET) BlossomMatching/*.o BlossomMatching/GEOM/*.o BlossomMatching/MinCost/*.o BlossomMatching/*.d BlossomMatching/GEOM/*.d BlossomMatching/MinCost/*.d

ifdef SystemRoot # Windows
	LIBS += -lpsapi
	RM = del /Q *.o $(TARGET).exe
	BUILD = $(TARGET)
else 
	ifeq ($(shell uname -o),Cygwin)
		LIBS += -lpsapi
		BUILD = $(TARGET)
		# BUILD = msbuild
		# RM = MSBuild.exe /p:Configuration=Release /t:clean
	else 
		BUILD = $(TARGET)
	endif
endif

all: $(BUILD)
	
msbuild:
	MSBuild.exe /p:Configuration=Release

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)

-include $(SRC:.cpp=.d)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

rebuild: clean all

clean:
	$(RM)
