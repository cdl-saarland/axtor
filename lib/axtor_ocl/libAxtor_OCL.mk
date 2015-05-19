### libAxtorOpenCL.so
AXTOROCL_SOURCES=$(wildcard lib/axtor_ocl/**/*.cpp) $(wildcard lib/axtor_ocl/*.cpp)
AXTOROCL_OBJECTS=$(patsubst lib/%.cpp,build/%.o,${AXTOROCL_SOURCES})

$(LIB_AXTOROCL): $(AXTOROCL_OBJECTS) $(LIB_AXTOR)
	$(CXX) -o $@  $(LDFLAGS) $(LLVM_LIBS) $(AXTOROCL_OBJECTS) $(LIB_AXTOR)

all:: $(LIB_AXTOROCL)