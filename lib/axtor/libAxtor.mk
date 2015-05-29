### libAxtor.so
AXTOR_SOURCES=$(wildcard lib/axtor/*/*.cpp) $(wildcard lib/axtor/*.cpp) $(wildcard lib/axtor/*/*/*.cpp)
AXTOR_OBJECTS=$(patsubst %.cpp,build/%.o,${AXTOR_SOURCES})

$(LIB_AXTOR): ${AXTOR_OBJECTS}
	$(CXX) -shared $(OPTLEVEL) -o $@ $(LDFLAGS) $(LLVM_LIBS) $(AXTOR_OBJECTS)

install:: $(LIB_AXTOR)
	cp -r include/axtor $(PREFIX)/include

all:: $(LIB_AXTOR)
