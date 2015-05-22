EXTRACTOR=$(PREFIX)/bin/axtor
EXTRACTOR_SOURCES=$(wildcard tools/extractor/*.cpp)
EXTRACTOR_OBJECTS=$(patsubst %.cpp,build/%.o,$(EXTRACTOR_SOURCES))

$(EXTRACTOR): $(EXTRACTOR_OBJECTS) $(LIB_AXTOROCL) $(LIB_AXTOR)
	$(CXX) $(OPTLEVEL) $(WARNLEVEL) -o $@ $^ $(LDFLAGS)

all:: $(EXTRACTOR)
