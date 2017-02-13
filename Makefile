CXX=clang++
CXXFLAGS=-Wall -Wextra -pedantic -std=c++14 -g -Wno-unused-parameter

CXXFILES=main.cpp register_machine.cpp
OBJFILES=$(addprefix $(OBJDIR)/,$(CXXFILES:.cpp=.o))
OBJDIR=obj

TARGET=z80interp

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJFILES)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJFILES)

.PHONY: clean
