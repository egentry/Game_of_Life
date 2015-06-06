CC=gcc
MPICC = mpicc


SERIAL_OBJS= GoL_serial.o GoL.o 
SERIAL_EXE = GoL_serial
SERIAL_FLAGS = -Og -g

MPI_OBJS= GoL_MPI.o GoL.o communicate.o io.o
MPI_EXE = GoL_MPI
MPI_FLAGS       =  -O2
MPI_FLAGS_DEBUG =  -Og -g

TEST_OBJS = get_performance_constants.o GoL.o communicate.o
TEST_EXE = get_performance_constants


.c.o: 
	$(MPICC) -c $< $(MPI_FLAGS)

all:$(MPI_EXE)
all:$(SERIAL_EXE)
all:$(TEST_EXE)

$(MPI_EXE): $(MPI_OBJS)
	$(MPICC) -o $(MPI_EXE) $(MPI_OBJS) $(MPI_FLAGS)

$(SERIAL_EXE): $(SERIAL_OBJS)
	$(CC) -o $(SERIAL_EXE) $(SERIAL_OBJS) $(SERIAL_FLAGS) 

$(TEST_EXE): $(TEST_OBJS)
	$(MPICC) -o $(TEST_EXE) $(TEST_OBJS) $(MPI_FLAGS)


clean:
	rm -f *.o data/*.dat
	rm -f $(SERIAL_EXE)
	rm -f $(MPI_EXE)
	rm -f $(TEST_EXE)
