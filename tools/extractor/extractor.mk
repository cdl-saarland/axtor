EXTRACTOR=build/bin/axtor
EXTRACTOR_SOURCES=$(wildcard tools/extractor/*.cpp)
EXTRACTOR_OBJECTS=$(patsubst %.cpp,build/%.o,$(EXTRACTOR_SOURCES))

$(EXTRACTOR): $(EXTRACTOR_OBJECTS) $(LIB_AXTORC) $(LIB_AXTOR)
	mkdir -p $(dir $@)
	$(CXX) $(OPTLEVEL) $(WARNLEVEL) -o $@ $^ $(LDFLAGS)

all:: $(EXTRACTOR)


install:: $(EXTRACTOR)
	cp $(EXTRACTOR) $(PREFIX)/bin
