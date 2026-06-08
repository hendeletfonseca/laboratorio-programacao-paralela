#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define THREADS 4
#define TAMANHO_TAREFA 500000

int primo (long int n) {
	long int i;
       
	for (i = 3; i < (long int)(sqrt(n) + 1); i+=2) 
	     if (n%i == 0) 
	         return 0;
	return 1;
}

long int contar_primos_tarefa(long int inicio, long int fim) {
    long int candidato;
    long int total = 0;

    for (candidato = inicio; candidato <= fim; candidato += 2) {
        if (primo(candidato)) {
            total++;
        }
    }

    return total;
}

int main(int argc, char *argv[]) {
    double t_inicio, t_fim;
    long int n, total = 0;
    long int tarefa, total_tarefas;

    if (argc < 2) {
        printf("Valor invalido! Entre com o valor do maior inteiro\n");
        return 0;
    }
    n = strtol(argv[1], (char **)NULL, 10);

    t_inicio = omp_get_wtime();

    total_tarefas = ((n - 3) / TAMANHO_TAREFA) + 1;
    
#pragma omp parallel for private(tarefa) reduction(+:total) schedule(guided,10) num_threads(THREADS)
    for (tarefa = 0; tarefa < total_tarefas; tarefa++) {

        long int inicio = 3 + tarefa * TAMANHO_TAREFA;
        long int fim = inicio + TAMANHO_TAREFA - 1;
        if (fim > n) {
            fim = n;
        }

        total += contar_primos_tarefa(inicio, fim);
    }

    if (n >= 2) {
        total++;
    }
    t_fim = omp_get_wtime();

    printf("Quant. de primos entre 1 e %ld: %ld \n", n, total);
    printf("Tempo de execucao: %f \n", t_fim - t_inicio);

    return 0;
}
