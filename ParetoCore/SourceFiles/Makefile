OBJS = AFElasticity.o  Timer.o IntegerSet.o queue.o


LIBS =
CPPFLAGS = -w -O3 -I../inc/hexmesh -I../SourceFiles -fopenmp


fire:$(OBJS)
	ar -cvr libAFElasticity.a  $(OBJS)
	mv libAFElasticity.a ./lib

.o:.cpp	
	g++ $(CPPFLAGS) $<

clean:
	\rm -rf *.o

