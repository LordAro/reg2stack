CXX=clang++
CXXFLAGS=-Wall -Wextra -pedantic -std=c++14 -g

CXXFILES=main.cpp optimise.cpp register_convert.cpp register_machine.cpp stack_machine.cpp stackconvert_machine.cpp util.cpp
OBJFILES=$(addprefix $(OBJDIR)/,$(CXXFILES:.cpp=.o))
OBJDIR=obj

TARGET=reg2stack

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJFILES)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: $(TARGET)
	./test_runner.py

clean:
	rm -f $(TARGET)
	rm -f $(OBJFILES)

.PHONY: clean
