# Compiler
CXX = g++

# Flags
CXXFLAGS = -O3
LDFLAGS =  ${CXXFLAGS}

# Define what extensions we use
.SUFFIXES : .cpp

# Name of executable
PROG_NAME = duplo

PREFIX=/usr/local

# List of object files
OBJS = StringUtil.o HashUtil.o ArgumentParser.o TextFile.o \
       SourceFile.o SourceLine.o Duplo.o FileType.o

# Build process

all: ${PROG_NAME}

# Link
${PROG_NAME}: ${OBJS}
	${CXX} ${LDFLAGS} -o ${PROG_NAME} ${OBJS}

install: duplo
	install -m 0755 duplo ${PREFIX}/bin
	install -d ${PREFIX}/share/docs/duplo
	install -m 0644 README ${PREFIX}/share/docs/duplo
	install -m 0644 README ${PREFIX}/share/docs/duplo

# Remove all object files
clean:	
	rm -f *.o




