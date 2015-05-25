CC=gcc
MPICC = mpicc


SERIAL_OBJS= GoL_serial.o GoL.o 
SERIAL_EXE = GoL_serial
SERIAL_FLAGS = -Og -g

MPI_OBJS= GoL_MPI.o GoL.o communicate.o
MPI_EXE = GoL_MPI
MPI_FLAGS = -Og -g

.c.o: 
	$(MPICC) -c $< $(SERIAL_FLAGS)

# test: test.o
# 	$(MPICC) -o test test.o

MPI: $(MPI_OBJS)
	$(MPICC) -o $(MPI_EXE) $(MPI_OBJS) $(MPI_FLAGS)

serial: $(SERIAL_OBJS)
	$(CC) -o $(SERIAL_EXE) $(SERIAL_OBJS) $(SERIAL_FLAGS) 


clean:
	rm -f *.o
	rm -f $(SERIAL_EXE)
	rm -f $(MPI_EXE)