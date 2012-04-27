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

all: ${PROG_NAME} ${PROG_NAME}.1.gz

# Link
${PROG_NAME}: ${OBJS}
	${CXX} ${LDFLAGS} -o ${PROG_NAME} ${OBJS}

${PROG_NAME}.1.gz: ${PROG_NAME}.1
	gzip -c ${PROG_NAME}.1 > ${PROG_NAME}.1.gz

install: duplo
	install -m 0755 duplo ${PREFIX}/bin
	install -d ${PREFIX}/share/docs/duplo
	install -m 0644 README ${PREFIX}/share/docs/duplo
	install -m 0644 README ${PREFIX}/share/docs/duplo
	install -m 0644 duplo.1.gz ${PREFIX}/share/man/man1

# Remove all object files and gzipped man page.
clean:	
	rm -f ./*.o
	rm -f ./${PROG_NAME}.1.gz
	rm -f ./${PROG_NAME}



