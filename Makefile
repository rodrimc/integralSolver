
all:
	gcc -Wall integralSolver_v1.c -o integralSolver_v1 -fopenmp -lm
	gcc -Wall integralSolver_v2.c -o integralSolver_v2 -fopenmp -lm
