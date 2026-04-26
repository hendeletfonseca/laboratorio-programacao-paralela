#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#define TAMANHO 500000

void registrar_csv_execucao(const char *funcao, int np, int n, double tempo_execucao) {
    const char *arquivo_csv = "resultados_execucao_bag_np1.csv";
    FILE *fp = fopen(arquivo_csv, "a+");
    if (fp == NULL) {
        perror("Erro ao abrir resultados_execucao_bag_np1.csv");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long tamanho = ftell(fp);

    if (tamanho == 0) {
        fprintf(fp, "funcao,np,n,tempo_execucao\n");
    }

    fprintf(fp, "%s,%d,%d,%.6f\n", funcao, np, n, tempo_execucao);
    fclose(fp);
}

int primo (int n) {
int i;
	for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
			if(n%i == 0) return 0;
	}
	return 1;
}

int strategySendRecv(int n, int meu_ranque, int num_procs, int raiz) {
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

int strategyIsendRecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;
    MPI_Request req;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Isend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, MPI_STATUS_IGNORE);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Isend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, MPI_STATUS_IGNORE);
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

            MPI_Isend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, MPI_STATUS_IGNORE);
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
                MPI_Isend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD, &req);   
                MPI_Wait(&req, MPI_STATUS_IGNORE);
            }
        }
    }

    return total;
}
int strategyRsendRecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    int ok = 1;
    MPI_Status estado;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Recv(&ok, 1, MPI_INT, dest, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Rsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Recv(&ok, 1, MPI_INT, dest, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Rsend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
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
            
            MPI_Recv(&ok, 1, MPI_INT, dest, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Rsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } else {
        while (estado.MPI_TAG != 99) {
            MPI_Send(&ok, 1, MPI_INT, raiz, 99, MPI_COMM_WORLD);
            MPI_Recv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
            if (estado.MPI_TAG != 99) {
                for (i = inicio, cont = 0; i < (inicio + TAMANHO) && i < n; i += 2) {
                    if (primo(i) == 1) {
                        cont++;
                    }
                }
                MPI_Rsend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            }
        }
    }

    return total;
}
int strategyBsendRecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0, buffer_size;
    void *buffer;
    MPI_Status estado;

    buffer_size = MPI_BSEND_OVERHEAD + sizeof(int);
    buffer = (void *)malloc(buffer_size);
    MPI_Buffer_attach(buffer, buffer_size);

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Bsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Bsend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
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

            MPI_Bsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
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
                MPI_Bsend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Buffer_detach(&buffer, &buffer_size);
    free(buffer);

    return total;
}
int strategySsendRecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Ssend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Ssend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
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

            MPI_Ssend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
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
                MPI_Ssend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            }
        }
    }

    return total;
}
int strategySendIrecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;
    MPI_Request req;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }
        for (; dest < num_procs; dest++) {
            MPI_Send(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            stop++;
        }
        while (stop < (num_procs - 1)) {
            MPI_Irecv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
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
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
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
int strategyIsendIrecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;
    MPI_Request req;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Isend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Isend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            stop++;
        }

        while (stop < (num_procs - 1)) {
            MPI_Irecv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            total += cont;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99;
                stop++;
            }

            MPI_Isend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            inicio += TAMANHO;
        }
    } else {
        while (estado.MPI_TAG != 99) {
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            if (estado.MPI_TAG != 99) {
                for (i = inicio, cont = 0; i < (inicio + TAMANHO) && i < n; i += 2) {
                    if (primo(i) == 1) {
                        cont++;
                    }
                }
                MPI_Isend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &estado);
            }
        }
    }

    return total;
}
int strategyRsendIrecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;
    MPI_Request req;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Rsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Rsend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            stop++;
        }

        while (stop < (num_procs - 1)) {
            MPI_Irecv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            total += cont;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99;
                stop++;
            }

            MPI_Rsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } else {
        while (estado.MPI_TAG != 99) {
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            if (estado.MPI_TAG != 99) {
                for (i = inicio, cont = 0; i < (inicio + TAMANHO) && i < n; i += 2) {
                    if (primo(i) == 1) {
                        cont++;
                    }
                }
                MPI_Rsend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            }
        }
    }

    return total;
}
int strategyBsendIrecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0, buffer_size;
    void *buffer;
    MPI_Status estado;
    MPI_Request req;

    buffer_size = MPI_BSEND_OVERHEAD + sizeof(int);
    buffer = (void *)malloc(buffer_size);
    MPI_Buffer_attach(buffer, buffer_size);

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Bsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }
        for (; dest < num_procs; dest++) {
            MPI_Bsend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            stop++;
        }
        while (stop < (num_procs - 1)) {
            MPI_Irecv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            total += cont;
            dest = estado.MPI_SOURCE;
            if (inicio > n) {
                tag = 99;
                stop++;
            }
            MPI_Bsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } else {
        while (estado.MPI_TAG != 99) {
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            if (estado.MPI_TAG != 99) {
                for (i = inicio, cont = 0; i < (inicio + TAMANHO) && i < n; i += 2) {
                    if (primo(i) == 1) {
                        cont++;
                    }
                }
                MPI_Bsend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Buffer_detach(&buffer, &buffer_size);
    free(buffer);

    return total;
}
int strategySsendIrecv(int n, int meu_ranque, int num_procs, int raiz) {
    int cont = 0, total = 0;
    int i, inicio = 3, dest, tag = 1, stop = 0;
    MPI_Status estado;
    MPI_Request req;

    if (meu_ranque == raiz) {
        for (dest = 1; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            MPI_Ssend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Workers sem tarefa inicial precisam receber tag 99 para nao bloquear em Recv.
        for (; dest < num_procs; dest++) {
            MPI_Ssend(&inicio, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            stop++;
        }

        while (stop < (num_procs - 1)) {
            MPI_Irecv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            total += cont;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99;
                stop++;
            }

            MPI_Ssend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } else {
        while (estado.MPI_TAG != 99) {
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &estado);
            if (estado.MPI_TAG != 99) {
                for (i = inicio, cont = 0; i < (inicio + TAMANHO) && i < n; i += 2) {
                    if (primo(i) == 1) {
                        cont++;
                    }
                }
                MPI_Ssend(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
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
    total = strategySendRecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Send/Recv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategySendRecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategyIsendRecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Isend/Recv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategyIsendRecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategyRsendRecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Rsend/Recv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategyRsendRecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategyBsendRecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Bsend/Recv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategyBsendRecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategySsendRecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Ssend/Recv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategySsendRecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategySendIrecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Send/Irecv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategySendIrecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategyIsendIrecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Isend/Irecv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategyIsendIrecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategyRsendIrecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Rsend/Irecv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategyRsendIrecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategyBsendIrecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Bsend/Irecv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategyBsendIrecv", num_procs, n, t_final - t_inicial);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();
    total = strategySsendIrecv(n, meu_ranque, num_procs, raiz);
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    /* Acrescenta o 2, que é primo */
        printf("Execução com Ssend/Irecv\n");
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
        registrar_csv_execucao("strategySsendIrecv", num_procs, n, t_final - t_inicial);
    }

/* Finaliza o programa */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return(0);
}