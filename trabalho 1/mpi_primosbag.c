#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#define TAMANHO 500000

int primo (int n) {
int i;
	for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
			if(n%i == 0) return 0;
	}
	return 1;
}

int strategySendRecieve(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Send(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            stop++;
        }

        while (stop < (num_procs - 1)) {
            MPI_Recv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
            total += cont;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99;
                stop++;
            }

            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } else {
        while (estado.MPI_TAG != 99) {
            MPI_Recv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
            if (estado.MPI_TAG != 99) {
                for (i = inicio, cont = 0; i < (inicio + TAMANHO) && i < n; i += 2) {
                    if (primo(i) == 1) {
                        cont++;
                    }
                }
                MPI_Send(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);   
            }
        }
    }

    return total;
}

int main(int argc, char *argv[]) { /* mpi_primosbag.c  */
double t_inicial, t_final;
int total = 0;
int n;
int meu_ranque, num_procs, raiz=0;
/* Verifica o número de argumentos passados */
	if (argc < 2) {
        printf("Entre com o valor do maior inteiro como parâmetro para o programa.\n");
       	 return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
/* Se houver menos que dois processos aborta */
    if (num_procs < 2) {
        printf("Este programa deve ser executado com no mínimo dois processos.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
       	return(1);
    }
/* Registra o tempo inicial de execução do programa */
    t_inicial = MPI_Wtime();
    total = strategySendRecieve(n, meu_ranque, num_procs, raiz);
	if (meu_ranque == 0) {
		t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
		printf("Quant. de primos entre 1 e %d: %d \n", n, total);
		printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);  	 
	}
/* Finaliza o programa */
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return(0);
}