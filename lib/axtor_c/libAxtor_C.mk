### libAxtorC.so
AXTORC_SOURCES=$(wildcard lib/axtor_c/**/*.cpp) $(wildcard lib/axtor_c/*.cpp)
AXTORC_OBJECTS=$(patsubst %.cpp,build/%.o,${AXTORC_SOURCES})

$(LIB_AXTORC): $(AXTORC_OBJECTS) $(LIB_AXTOR)
	$(CXX) $(OPTLEVEL) -shared -o $@  $(LDFLAGS) $(LLVM_LIBS) $(AXTORC_OBJECTS) $(LIB_AXTOR)

all:: $(LIB_AXTORC)

install:: $(LIB_AXTORC)
	cp -r include/axtor_c $(PREFIX)/include
