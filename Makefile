CC=gcc
CXX=g++
CXXFLAGS+=-I include -fPIC -std=c++0x

SOURCES = esort/main.cpp esort/program_options.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = bin/external_sort

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@

$(TARGET): $(OBJECTS)
	${CXX} $(OBJECTS) -o $(TARGET) $(LIBS)
all:
	${TARGET}
clean:
	rm -f *.o $(TARGET)

strip:
	$(STRIP) $(TARGET)

