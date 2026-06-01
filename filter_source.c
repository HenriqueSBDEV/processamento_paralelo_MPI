#include "filter_source.h"
#include <mpi.h>
#include <string.h>

// Função de teste de corretude usando os códigos disponibilizados no repositório do professor para validar a lógica do filtro antes de aplicar na imagem real.
void testar_corretude_professor() {
    printf("=== TESTE DE CORRETUDE ===\n");
    
    // Matriz 5x5 sintetica do notebook
    double imagem[5][5] = {
        {1, 2, 3, 2, 1},
        {2, 4, 6, 4, 2},
        {3, 6, 9, 6, 3},
        {2, 4, 6, 4, 2},
        {1, 2, 3, 2, 1}
    };
    double buffer[5][5] = {0};

    // Kernel 3x3 normalizado retirado do output do código Python disponibilizado pelo professor
    const double kernel_3x3[3][3] = {
        {0.07511361, 0.1238414,  0.07511361},
        {0.1238414,  0.20417996, 0.1238414},
        {0.07511361, 0.1238414,  0.07511361}
    };

    int pad = 1; // Padding para kernel 3x3
    int iteracoes = 2; // Exigido no notebook

    // Ponteiros para estratégia de alteração de ponteiros (duas matrizes, uma para leitura e outra para escrita, alternando a cada iteração)
    double (*leitura)[5] = imagem;
    double (*escrita)[5] = buffer;

    // Mesma lógica da função principal
    for (int iter = 0; iter < iteracoes; iter++) {
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                double soma = 0.0;
                
                for (int ky = -pad; ky <= pad; ky++) {
                    for (int kx = -pad; kx <= pad; kx++) {
                        int py = y + ky;
                        int px = x + kx;

                        // Tratamento de Borda 
                        if (py < 0) py = 0;
                        else if (py >= 5) py = 4;
                        
                        if (px < 0) px = 0;
                        else if (px >= 5) px = 4;

                        soma += leitura[py][px] * kernel_3x3[ky + pad][kx + pad];
                    }
                }
                escrita[y][x] = soma;
            }
        }
        
        // Troca os ponteiros
        double (*temp)[5] = leitura;
        leitura = escrita;
        escrita = temp;
    }

    // Imprime o resultado formatado igual ao do notebook Python (2 casas decimais)
    printf("Resultado em C apos %d iteracoes:\n", iteracoes);
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            printf("%.2f  ", leitura[y][x]);
        }
        printf("\n");
    }
    printf("=======================================\n\n");
}

// Função de filtro sequencial
void aplicar_filtro_gaussiano_sequencial(Imagem *img, int iteracoes) {
    int largura = img->largura;
    int altura = img->altura;
    int canais = img->canais;
    int total_bytes = largura * altura * canais;

    // Aloca um buffer temporário para salvar os cálculos da iteração atual
    unsigned char *buffer = (unsigned char *)malloc(total_bytes * sizeof(unsigned char));
    if (!buffer) {
        printf("Erro ao alocar buffer temporário.\n");
        exit(1);
    }

    // Kernel 5x5 normalizado retirado do output do código Python disponibilizado pelo professor
    const float kernel_5x5[5][5] = {
        {0.00296902, 0.01330621, 0.02193823, 0.01330621, 0.00296902},
        {0.01330621, 0.05963430, 0.09832033, 0.05963430, 0.01330621},
        {0.02193823, 0.09832033, 0.16210282, 0.09832033, 0.02193823},
        {0.01330621, 0.05963430, 0.09832033, 0.05963430, 0.01330621},
        {0.00296902, 0.01330621, 0.02193823, 0.01330621, 0.00296902}
    };
    
    // Para kernel 5x5, o padding necessário é de 2 pixels em cada direção
    int pad = 2; 

    // Ponteiros para a técnica de alteração de ponteiros (duas matrizes, uma para leitura e outra para escrita, alternando a cada iteração)
    unsigned char *leitura = img->pixels;
    unsigned char *escrita = buffer;

    for (int iter = 0; iter < iteracoes; iter++) {
        
        // Varre a imagem pixel por pixel
        for (int y = 0; y < altura; y++) {
            for (int x = 0; x < largura; x++) {
                
                // Trata cada canal de cor independentemente (funciona para PGM e PPM)
                for (int c = 0; c < canais; c++) {
                    float soma = 0.0;

                    // Percorre a máscara/kernel 5x5
                    for (int ky = -pad; ky <= pad; ky++) {
                        for (int kx = -pad; kx <= pad; kx++) {
                            
                            // Calcula as coordenadas reais do vizinho
                            int py = y + ky;
                            int px = x + kx;

                            // Lógica de borda, como se fosse o mode='edge' do np.pad do Python
                            if (py < 0) py = 0;
                            else if (py >= altura) py = altura - 1;
                            
                            if (px < 0) px = 0;
                            else if (px >= largura) px = largura - 1;

                            // Mapeia o 2D para o índice 1D contíguo
                            int indice_pixel = (py * largura + px) * canais + c;
                            
                            // Multiplica e soma o valor do pixel pelo valor correspondente do kernel
                            soma += leitura[indice_pixel] * kernel_5x5[ky + pad][kx + pad];
                        }
                    }
                    
                    // Salva o resultado no buffer de escrita
                    int indice_saida = (y * largura + x) * canais + c;
                    escrita[indice_saida] = (unsigned char)soma;
                }
            }
        }
        
        // Troca os ponteiros para a próxima iteração
        unsigned char *temp = leitura;
        leitura = escrita;
        escrita = temp;
    }

    // Se o número de iterações for ímpar, o resultado final terá parado no 'buffer'.
    if (iteracoes % 2 != 0) {
        for (int i = 0; i < total_bytes; i++) {
            img->pixels[i] = buffer[i];
        }
    }

    // Libera a memória que foi alocada dinamicamente nesta função
    free(buffer);
}


void aplicar_filtro_gaussiano_mpi(
    unsigned char *pixels,
    unsigned char *resultado,
    int largura,
    int altura,
    int canais,
    int iteracoes)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 1. Enviar dimensões da imagem do Rank 0 para todos os outros
    MPI_Bcast(&largura, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&altura, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&canais, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2. Calcular a divisão balanceada de linhas para o Scatterv/Gatherv
    int *linhas_por_proc = (int *)malloc(size * sizeof(int));
    int *send_counts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));

    int linhas_base = altura / size;
    int resto = altura % size;
    int deslocamento_linhas = 0;

    for (int i = 0; i < size; i++) {
        linhas_por_proc[i] = linhas_base + (i < resto ? 1 : 0);
        send_counts[i] = linhas_por_proc[i] * largura * canais;
        displs[i] = deslocamento_linhas * largura * canais;
        deslocamento_linhas += linhas_por_proc[i];
    }

    int minhas_linhas = linhas_por_proc[rank];
    int tamanho_linha_bytes = largura * canais;

    //Alocação dos buffers locais incluindo espaço para as "Ghost/Halo Rows"
    int pad = 2; // Kernel 5x5 precisa de 2 linhas extras em cima e 2 embaixo
    int total_linhas_local = minhas_linhas + 2 * pad;
    int total_bytes_local = total_linhas_local * tamanho_linha_bytes;

    unsigned char *buffer_A = (unsigned char *)calloc(total_bytes_local, sizeof(unsigned char));
    unsigned char *buffer_B = (unsigned char *)calloc(total_bytes_local, sizeof(unsigned char));

    if (!buffer_A || !buffer_B) {
        printf("[Rank %d] Erro ao alocar buffers locais.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Ponteiro apontando exatamente para onde começam os dados reais locais (pulando o halo superior)
    unsigned char *meus_dados_reais = buffer_A + (pad * tamanho_linha_bytes);

    // Distribuir fatias da imagem original para todos os processos
    MPI_Scatterv(
        pixels, send_counts, displs, MPI_BYTE,
        meus_dados_reais, minhas_linhas * tamanho_linha_bytes, MPI_BYTE,
        0, MPI_COMM_WORLD
    );

    const float kernel_5x5[5][5] = {
        {0.00296902, 0.01330621, 0.02193823, 0.01330621, 0.00296902},
        {0.01330621, 0.05963430, 0.09832033, 0.05963430, 0.01330621},
        {0.02193823, 0.09832033, 0.16210282, 0.09832033, 0.02193823},
        {0.01330621, 0.05963430, 0.09832033, 0.05963430, 0.01330621},
        {0.00296902, 0.01330621, 0.02193823, 0.01330621, 0.00296902}
    };

    int vizinho_cima = (rank > 0) ? rank - 1 : MPI_PROC_NULL;
    int vizinho_baixo = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;

    unsigned char *leitura = buffer_A;
    unsigned char *escrita = buffer_B;

    // Loop de iterações do Filtro Gaussiano paralelo
    for (int iter = 0; iter < iteracoes; iter++)
    {
        // Atualiza o ponteiro de onde estão os dados válidos a serem enviados nesta iteração
        meus_dados_reais = leitura + (pad * tamanho_linha_bytes);

        // --- TROCA DE FRONTEIRAS (HALO ROWS) VIA MPI_SENDRECV ---
        // Envia as 2 primeiras linhas locais para cima, recebe as 2 linhas de halo de baixo
        MPI_Sendrecv(
            meus_dados_reais, pad * tamanho_linha_bytes, MPI_BYTE, vizinho_cima, 0,
            meus_dados_reais + (minhas_linhas * tamanho_linha_bytes), pad * tamanho_linha_bytes, MPI_BYTE, vizinho_baixo, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // Envia as 2 últimas linhas locais para baixo, recebe as 2 linhas de halo de cima
        MPI_Sendrecv(
            meus_dados_reais + ((minhas_linhas - pad) * tamanho_linha_bytes), pad * tamanho_linha_bytes, MPI_BYTE, vizinho_baixo, 1,
            leitura, pad * tamanho_linha_bytes, MPI_BYTE, vizinho_cima, 1,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // Se for a borda absoluta de cima da imagem inteira (Rank 0), replica a primeira linha real no halo superior
        if (rank == 0) {
            for (int p = 0; p < pad; p++) {
                memcpy(leitura + (p * tamanho_linha_bytes), meus_dados_reais, tamanho_linha_bytes);
            }
        }
        // Se for a borda absoluta de baixo da imagem inteira (Último Rank), replica a última linha real no halo inferior
        if (rank == size - 1) {
            unsigned char *ultima_linha_real = meus_dados_reais + ((minhas_linhas - 1) * tamanho_linha_bytes);
            for (int p = 0; p < pad; p++) {
                memcpy(meus_dados_reais + ((minhas_linhas + p) * tamanho_linha_bytes), ultima_linha_real, tamanho_linha_bytes);
            }
        }

        // O loop varre apenas o espaço de dados reais atribuído ao processo corrente (deslocado por 'pad')
        for (int y = pad; y < minhas_linhas + pad; y++)
        {
            for (int x = 0; x < largura; x++)
            {
                for (int c = 0; c < canais; c++)
                {
                    float soma = 0.0f;

                    for (int ky = -pad; ky <= pad; ky++)
                    {
                        for (int kx = -pad; kx <= pad; kx++)
                        {
                            // py e px calculados no sistema de coordenadas local do buffer expandido
                            int py = y + ky;
                            int px = x + kx;

                            // Tratamento horizontal de borda idêntico ao sequencial
                            if (px < 0)
                                px = 0;
                            else if (px >= largura)
                                px = largura - 1;

                            // Mapeamento idêntico ao sequencial para buscar o pixel vizinho (com os halos populados!)
                            int indice_pixel = (py * largura + px) * canais + c;

                            soma += leitura[indice_pixel] * kernel_5x5[ky + pad][kx + pad];
                        }
                    }

                    int indice_saida = (y * largura + x) * canais + c;
                    escrita[indice_saida] = (unsigned char)soma;
                }
            }
        }

        // Troca de ponteiros local idêntica à lógica do sequencial
        unsigned char *temp = leitura;
        leitura = escrita;
        escrita = temp;
    }

    // Garante que a referência aponta para onde o resultado final da última iteração pousou
    meus_dados_reais = leitura + (pad * tamanho_linha_bytes);

    // Juntar todas as fatias processadas de volta no buffer 'resultado' do Rank 0
    MPI_Gatherv(
        meus_dados_reais, minhas_linhas * tamanho_linha_bytes, MPI_BYTE,
        resultado, send_counts, displs, MPI_BYTE,
        0, MPI_COMM_WORLD
    );

    // Liberação das estruturas auxiliares e buffers de cada processo
    free(buffer_A);
    free(buffer_B);
    free(linhas_por_proc);
    free(send_counts);
    free(displs);
}