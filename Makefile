CXX=clang++
CXXFLAGS=-Wall -Wextra -pedantic -std=c++14 -g

CXXFILES=main.cpp register_machine.cpp
OBJFILES=$(addprefix $(OBJDIR)/,$(CXXFILES:.cpp=.o))
OBJDIR=obj

TARGET=reg2stack

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJFILES)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJFILES)

.PHONY: clean
