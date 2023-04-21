CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra -DNDEBUG
INCLUDES = -Isrc
DEPFLAGS = -MMD -MP
TMPDIR = tmp

# Detect Windows
ifeq ($(OS), Windows_NT)
	MKDIR    := mkdir
	uname_S  := Windows
	SUFFIX   := .exe
else
ifeq ($(COMP), MINGW)
	MKDIR    := mkdir
	uname_S  := Windows
	SUFFIX   := .exe
else
	MKDIR   := mkdir -p
	LDFLAGS := -pthread
	uname_S := $(shell uname -s)
	SUFFIX  :=
endif
endif

SRC_FILES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)

NATIVE 	  := -march=native
OBJECTS   := $(patsubst %.cpp,$(TMPDIR)/%.o,$(SRC_FILES))
DEPENDS   := $(patsubst %.cpp,$(TMPDIR)/%.d,$(SRC_FILES))
TARGET    := fast-datagen

ifeq ($(build), debug)
	CXXFLAGS := -g3 -O3 -std=c++17 -Wall -Wextra -pedantic
endif

ifeq ($(build), release)
	CXXFLAGS := -O3 -std=c++17 -Wall -Wextra -pedantic -DNDEBUG
	LDFLAGS  := -lpthread -static -static-libgcc -static-libstdc++ -Wl,--no-as-needed
	NATIVE   := -march=x86-64
endif

# Different native flag for macOS
ifeq ($(uname_S), Darwin)
	NATIVE =	
	LDFLAGS =
endif

ifeq ($(san), asan)
	LDFLAGS += -fsanitize=address
endif

ifeq ($(san), memory)
	LDFLAGS += -fsanitize=memory -fPIE -pie
endif

ifeq ($(san), undefined)
	LDFLAGS += -fsanitize=undefined
endif

.PHONY: clean all FORCE

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(NATIVE) -MMD -MP -o $@ $^ $(LDFLAGS)

$(TMPDIR)/%.o: %.cpp | $(TMPDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(NATIVE) -MMD -MP -c $< -o $@ $(LDFLAGS)

$(TMPDIR):
	$(MKDIR) "$(TMPDIR)" "$(TMPDIR)/src" "$(TMPDIR)/src/engines" "$(TMPDIR)/src/third_party"

clean:
	rm -rf $(TMPDIR)

-include $(DEPENDS)
