JAVA_HOME ?= $(shell /usr/libexec/java_home -v 1.7)

OBJDIR    := build

CPPFLAGS  := ${CPPFLAGS} -Iinclude -I${JAVA_HOME}/include -I${JAVA_HOME}/include/darwin
CXXFLAGS  := ${CXXFLAGS} -fno-rtti -fno-exceptions
LDFLAGS   := ${LDFLAGS} -framework JavaVM -L${OBJDIR} -Ldarwin -ljdk-7-jni
SRCS      := $(wildcard *.cpp)
OBJS      := $(addprefix $(OBJDIR)/,$(SRCS:%.cpp=%.o))

LD         = g++ ${LDFLAGS}

all: ${OBJDIR}/test
	${OBJDIR}/test

${OBJDIR}/test: ${OBJS}
	${LD} ${LDFLAGS} -o $@ $^

${OBJDIR}/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

${OBJS}: | ${OBJDIR}
     
${OBJDIR}:
	mkdir -p ${OBJDIR}

clean:
	rm -fr ${OBJDIR}
