PREFIX?=$(INSTALL_DIR)

# product definitions
LIB_AXTOR=$(PREFIX)/lib/libAxtor.so
LIB_AXTOROCL=$(PREFIX)/lib/libAxtor_OpenCL.so

all::


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

# libraries
include lib/axtor/libAxtor.mk
include lib/axtor_ocl/libAxtor_OCL.mk


# tools

# TODO


## Generic build rules

build/%.o: lib/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -o $@ $(CXXFLAGS) $^
	
	
.PHONY: clean
clean:
	rm -rf build/
