#include "img_source.h"
#include "filter_source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 5)
    {
        if(rank == 0)
        {
            printf("Uso: %s <entrada.p?m> <saida.p?m> <iteracoes> <modo: S ou P>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    const char *entrada = argv[1];
    const char *saida = argv[2];
    int iteracoes = atoi(argv[3]);
    char modo = argv[4][0];

    Imagem img;
    // Inicializa a estrutura para os ranks != 0 não usarem lixo de memória
    img.largura = 0;
    img.altura = 0;
    img.canais = 0;
    img.pixels = NULL;

    // Apenas o rank 0 lê a imagem do disco
    if (rank == 0)
    {
        if (strstr(entrada, ".pgm"))
        {
            printf("Lendo imagem PGM (cinza): %s...\n", entrada);
            img = ler_imagem_pgm_p5(entrada);
        }
        else if (strstr(entrada, ".ppm"))
        {
            printf("Lendo imagem PPM (RGB): %s...\n", entrada);
            img = ler_imagem_ppm_p6(entrada);
        }
        else
        {
            printf("Erro: Formato de entrada nao suportado. Use .pgm ou .ppm\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    }

    double start = 0.0, stop = 0.0;

    if (modo == 'S' || modo == 's')
    {
        if(rank == 0)
        {
            printf("Modo SEQUENCIAL | Kernel 5x5 | %d iteracoes\n", iteracoes);

            start = MPI_Wtime();
            aplicar_filtro_gaussiano_sequencial(&img, iteracoes);
            stop = MPI_Wtime();

            printf("Tempo de Execucao: %f segundos\n", stop - start);

            if (img.canais == 3)
                salvar_imagem_ppm_p6(saida, &img);
            else
                salvar_imagem_pgm_p5(saida, &img);

            free(img.pixels);
        }
    }
    else if (modo == 'P' || modo == 'p')
    {
        if(rank == 0)
        {
            printf("Modo MPI | Kernel 5x5 | %d iteracoes | %d processos\n",
                   iteracoes, size);
        }

        unsigned char *resultado = NULL;
        
        // Apenas o rank 0 aloca o buffer do resultado final completo
        if(rank == 0)
        {
            int total_bytes = img.largura * img.altura * img.canais;
            resultado = (unsigned char *)malloc(total_bytes);
            if (!resultado) {
                printf("Erro ao alocar buffer de resultado no rank 0.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            start = MPI_Wtime();
        }

        // CORREÇÃO CRUCIAL: TODOS os processos entram aqui juntos
        aplicar_filtro_gaussiano_mpi(
            img.pixels,    // Será lido apenas no rank 0 dentro da função
            resultado,     // Será escrito apenas no rank 0 dentro da função
            img.largura,   // O valor real será propagado via MPI_Bcast interno
            img.altura,
            img.canais,
            iteracoes
        );

        if(rank == 0)
        {
            stop = MPI_Wtime();

            if (img.pixels) free(img.pixels);
            img.pixels = resultado;

            printf("Tempo de Execucao: %f segundos\n", stop - start);

            if (img.canais == 3)
                salvar_imagem_ppm_p6(saida, &img);
            else
                salvar_imagem_pgm_p5(saida, &img);

            free(img.pixels);
        }
    }
    else
    {
        if(rank == 0) printf("Erro: Modo invalido. Use 'S' ou 'P'.\n");
        MPI_Finalize();
        return 1;
    }

    MPI_Finalize();
    return 0;
}