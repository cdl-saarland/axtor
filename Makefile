PREFIX?=$(INSTALL_DIR)

# product definitions
LIB_AXTOR=$(PREFIX)/lib/libAxtor.so
LIB_AXTOROCL=$(PREFIX)/lib/libAxtor_OpenCL.so

all: $(LIB_AXTOR) $(LIB_AXTOROCL)


### configuration

# LLVM
LLVM_CXXFLAGS:=`llvm-config --cxxflags`
LLVM_LDFLAGS:=`llvm-config --ldflags`
LLVM_LIBS:=`llvm-config --libs`

# compiler customization
WARNLEVEL=-Wall -Werror

# compiler
CXX=g++
CXXFLAGS=-fno-rtti -c -fPIC -Iinclude $(LLVM_CXXFLAGS) $(WARNLEVEL)
LDFLAGS=-shared -fPIC -Iinclude $(LLVM_LDFLAGS)

### libAxtor.so
# source definitions
AXTOR_SOURCES=$(wildcard lib/axtor/**/*.cpp) $(wildcard lib/axtor/*.cpp)
AXTOR_OBJECTS=$(patsubst lib/%.cpp,build/%.o,${AXTOR_SOURCES})

$(LIB_AXTOR): ${AXTOR_OBJECTS}
	$(CXX) -o $@ $(LDFLAGS) $(LLVM_LIBS) $(AXTOR_OBJECTS)

### libAxtorOpenCL.so

AXTOROCL_SOURCES=$(wildcard lib/axtor_ocl/**/*.cpp) $(wildcard lib/axtor_ocl/*.cpp)
AXTOROCL_OBJECTS=$(patsubst lib/%.cpp,build/%.o,${AXTOROCL_SOURCES})

$(LIB_AXTOROCL): $(AXTOROCL_OBJECTS)
	$(CXX) -o $@  $(LDFLAGS) $(LLVM_LIBS) $(AXTOROCL_OBJECTS)


## Generic build rules

build/%.o: lib/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -o $@ $(CXXFLAGS) $^
	
	
.PHONY: clean
clean:
	rm -rf build/
