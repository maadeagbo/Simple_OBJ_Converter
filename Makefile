IDIR =./
CXX=g++
CFLAGS=-I$(IDIR) -ggdb -std=c++11 -Wall

ODIR=./obj
LDIR =./lib

LIBS=

_DEPS = DD_MeshUtility.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main_OC.o DD_ObjConverter.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 