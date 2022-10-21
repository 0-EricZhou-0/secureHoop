TARGET = main

SRCS  = $(shell find ./ -type f -name *.cpp)
HEADS = $(shell find ./ -type f -name *.h)
OBJS = $(SRCS:.cpp=.o)
ASMS = $(SRCS:.cpp=.s)
DEPS = Makefile.depend

INCLUDES = -I./include
CXXFLAGS = -O0 -Wall $(INCLUDES)
LDFLAGS = -lm


all: $(TARGET)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)
	objdump -D $(OBJS) > $(ASMS)

run: all
	@./$(TARGET)

.PHONY:  clean
clean:
	$(RM) $(OBJS) $(ASMS) $(TARGET)

-include $(DEPS)