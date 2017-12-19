PLATFORM	?= $(shell echo $(shell uname) | tr '[:upper:]' '[:lower:]')
ifeq (${PLATFORM}, darwin)
	include Makefile.osx
endif
ifeq (${PLATFORM}, android)
	include Makefile.android
endif

LIBNAME		= lib${APINAME}-jni.a

ifdef JAVA_HOME
	JAVA		= ${JAVA_HOME}/bin/java
	JAVAC		= ${JAVA_HOME}/bin/javac
else
	JAVA		= java
	JAVAC		= javac
endif

JAVAFLAGS	= -XX:MaxPermSize=128M
JAVACFLAGS  = -source 1.8 -target 1.8

CPPFLAGS	+= -g0 -O2 -Wall -Werror -Wno-long-long -std=c++11

BUILDDIR	= build
GENDIR		= ${BUILDDIR}/${APINAME}/source

PLATFORM_BUILDDIR		= $(abspath ${BUILDDIR}/${APINAME}/${PLATFORM})

APIGENERATOR_SRCS		:= $(wildcard *.java)
APIGENERATOR_CLASSES	:= $(addprefix $(BUILDDIR)/,$(APIGENERATOR_SRCS:%.java=%.class))

static-apilib: ${GENDIR}/API.h ${GENDIR}/Makefile
	@make -C ${GENDIR} LIBNAME=${LIBNAME} BUILDDIR=${PLATFORM_BUILDDIR} static-lib

compile-static-apilib:
	@make -C ${GENDIR} LIBNAME=${LIBNAME} BUILDDIR=${PLATFORM_BUILDDIR} static-lib

api: ${GENDIR}/API.h ${GENDIR}/Makefile
	@make -C ${GENDIR}  BUILDDIR=${PLATFORM_BUILDDIR} compile

api-module: ${GENDIR}/Makefile ;
${GENDIR}/Makefile: Makefile.api *.cpp *.h | ${GENDIR}
	cp *.h *.cpp ${GENDIR}/
	cp Makefile.api ${GENDIR}/Makefile

api-source: ${GENDIR}/API.h ;
${GENDIR}/API.h: ${APIJAR} ${GPSJAR} ${APIGENERATOR_CLASSES} templates/* | ${GENDIR}
	${JAVA} ${JAVAFLAGS} -cp ${BUILDDIR} APIGenerator ${GENDIR} "${APIJAR};${GPSJAR}" ${APICLASSES}

api-generator: ${APIGENERATOR_CLASSES} ;
${BUILDDIR}/%.class: %.java | ${BUILDDIR}
	${JAVAC} ${JAVACFLAGS} -d ${BUILDDIR} $<

clean:
	@rm -fr ${BUILDDIR}

${GENDIR}:
	@mkdir -p ${GENDIR}

${BUILDDIR}:
	@mkdir -p ${BUILDDIR}
