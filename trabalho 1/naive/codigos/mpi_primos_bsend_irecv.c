#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int primo (long int n) { /* mpi_primos.c  */
    int i;

    for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
        if(n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0;
    long int i, n;
    int meu_ranque, num_procs, inicio, salto;

    if (argc < 2) {
        printf("Valor inválido! Entre com um valor do maior inteiro\n");
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    t_inicial = MPI_Wtime();
    inicio = 3 + meu_ranque*2;
    salto = num_procs*2;
    int cont_recebidos[num_procs];
    MPI_Request request[num_procs];

    if (meu_ranque == 0) {
        for (int p = 1; p < num_procs; p++) {
            MPI_Irecv(&(cont_recebidos[p]), 1, MPI_INT, p, 0, MPI_COMM_WORLD, &(request[p]));
        }
    }

    for (i = inicio; i <= n; i += salto)
        if(primo(i) == 1) cont++;

    if(meu_ranque != 0) {
        int tamanho_buffer = sizeof(int) + MPI_BSEND_OVERHEAD; //tem que deixar uma folguinha no buffer
        void *meu_buffer = malloc(tamanho_buffer);
        MPI_Buffer_attach(meu_buffer, tamanho_buffer);
        MPI_Bsend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Buffer_detach(&meu_buffer, &tamanho_buffer);
        free(meu_buffer);
    } else {
        total = cont;

        for (int k = 1; k < num_procs; k++) {
            MPI_Wait(&(request[k]), MPI_STATUS_IGNORE);
            total += cont_recebidos[k];
        }
    }

    t_final = MPI_Wtime();
    if (meu_ranque == 0) {
        total += 1;    /* Acrescenta o dois, que também é primo */
        printf("Quant. de primos entre 1 e n: %d \n", total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
    }
    MPI_Finalize();
    return(0);
}
