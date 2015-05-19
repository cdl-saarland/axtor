.PHONY: all

PREFIX?=$(INSTALL_DIR)

# source definitions
AXTOR_SOURCES=$(wildcard lib/axtor/**/*.cpp) $(wildcard lib/axtor/*.cpp)
AXTOR_OBJECTS=$(patsubst lib/%.cpp,build/%.o,${AXTOR_SOURCES})

# target definitions
LIB_AXTOR=$(PREFIX)/lib/libAxtor.so

# LLVM
LLVM_CXXFLAGS:=`llvm-config --cxxflags`
LLVM_LDFLAGS:=`llvm-config --ldflags`
LLVM_LIBS:=`llvm-config --libs`

# config
WARNLEVEL=-Wall -Werror

# compiler
CXX=g++
CXXFLAGS=-fno-rtti -c -fPIC -Iinclude $(LLVM_CXXFLAGS) $(WARNLEVEL)
LDFLAGS=-shared -fPIC -Iinclude $(LLVM_LDFLAGS)


$(LIB_AXTOR): ${AXTOR_OBJECTS}
	$(CXX) -o $@ $(LDFLAGS) $(LLVM_LIBS) $(AXTOR_OBJECTS)


build/%.o: lib/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -o $@ $(CXXFLAGS) $^

	
	
	
.PHONY: clean
clean:
	rm -rf build/
