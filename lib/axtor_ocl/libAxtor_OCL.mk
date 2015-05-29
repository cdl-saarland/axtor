### libAxtorOpenCL.so
AXTOROCL_SOURCES=$(wildcard lib/axtor_ocl/**/*.cpp) $(wildcard lib/axtor_ocl/*.cpp)
AXTOROCL_OBJECTS=$(patsubst %.cpp,build/%.o,${AXTOROCL_SOURCES})

$(LIB_AXTOROCL): $(AXTOROCL_OBJECTS) $(LIB_AXTOR)
	$(CXX) $(OPTLEVEL) -shared -o $@  $(LDFLAGS) $(LLVM_LIBS) $(AXTOROCL_OBJECTS) $(LIB_AXTOR)

all:: $(LIB_AXTOROCL)

install:: $(LIB_AXTOROCL)
	cp -r include/axtor_ocl $(PREFIX)/include
	cp $(LIB_AXTOROCL) $(PREFIX)/lib
