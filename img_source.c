
#include "img_source.h"

Imagem ler_imagem_ppm_p6(const char *caminho) {
    // Abre o arquivo em modo leitura binária ("rb")
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) {
        printf("Erro: Não foi possível abrir o arquivo %s\n", caminho);
        exit(1);
    }

    char formato[3];
    fscanf(arquivo, "%2s", formato);
    
    // Verifica se realmente é o formato P6
    if (formato[0] != 'P' || formato[1] != '6') {
        printf("Erro: O arquivo não é um PPM P6 válido.\n");
        fclose(arquivo);
        exit(1);
    }

    // Trata os comentários (GIMP costuma colocar comentários no cabeçalho)
    int c = getc(arquivo);
    while (c == ' ' || c == '\n' || c == '\r') { c = getc(arquivo); } // Pula espaços vazios
    
    while (c == '#') { // Se for um comentário
        while (getc(arquivo) != '\n'); // Ignora tudo até o fim da linha
        c = getc(arquivo);
        while (c == ' ' || c == '\n' || c == '\r') { c = getc(arquivo); }
    }
    ungetc(c, arquivo); // Devolve o último caractere válido para o buffer

    Imagem img;
    img.canais = 3; // PPM tem 3 canais: R, G e B
    int max_cor;
    
    // Lê as dimensões e o valor máximo de cor
    fscanf(arquivo, "%d %d", &img.largura, &img.altura);
    fscanf(arquivo, "%d", &max_cor);
    
    // Pula o exato último caractere de espaço/quebra de linha antes dos dados binários
    fgetc(arquivo); 

    // Alocação de Memória Contígua
    // Como é PPM (RGB), cada pixel tem 3 bytes (Vermelho, Verde e Azul)
    int total_bytes = img.largura * img.altura * 3;
    img.pixels = (unsigned char *)malloc(total_bytes * sizeof(unsigned char));

    if (!img.pixels) {
        printf("Erro: Falta de memória para alocar a imagem.\n");
        exit(1);
    }

    // Leitura RAW
    // Lê todos os pixels de uma única vez direto do disco para a RAM
    fread(img.pixels, sizeof(unsigned char), total_bytes, arquivo);

    fclose(arquivo);
    return img;
}

Imagem ler_imagem_pgm_p5(const char *caminho) {
    // Abre o arquivo em modo leitura binária ("rb")
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) {
        printf("Erro: Não foi possível abrir o arquivo %s\n", caminho);
        exit(1);
    }

    char formato[3];
    fscanf(arquivo, "%2s", formato);
    
    // Verifica se realmente é o formato P5
    if (formato[0] != 'P' || formato[1] != '5') {
        printf("Erro: O arquivo não é um PGM P5 válido.\n");
        fclose(arquivo);
        exit(1);
    }

    // Trata os comentários (GIMP costuma colocar comentários no cabeçalho)
    int c = getc(arquivo);
    while (c == ' ' || c == '\n' || c == '\r') { c = getc(arquivo); } // Pula espaços vazios
    
    while (c == '#') { // Se for um comentário
        while (getc(arquivo) != '\n'); // Ignora tudo até o fim da linha
        c = getc(arquivo);
        while (c == ' ' || c == '\n' || c == '\r') { c = getc(arquivo); }
    }
    ungetc(c, arquivo); // Devolve o último caractere válido para o buffer

    Imagem img;
    img.canais = 1;
    int max_cor;
    
    // Lê as dimensões e o valor máximo de cor
    fscanf(arquivo, "%d %d", &img.largura, &img.altura);
    fscanf(arquivo, "%d", &max_cor);
    
    // Pula o exato último caractere de espaço/quebra de linha antes dos dados binários
    fgetc(arquivo); 

    // Alocação de Memória Contígua
    // Como é PGM (Tons de cinza), cada pixel tem 1 byte
    int total_bytes = img.largura * img.altura * img.canais;
    img.pixels = (unsigned char *)malloc(total_bytes * sizeof(unsigned char));

    if (!img.pixels) {
        printf("Erro: Falta de memória para alocar a imagem.\n");
        exit(1);
    }

    // Leitura RAW
    // Lê todos os pixels de uma única vez direto do disco para a RAM
    fread(img.pixels, sizeof(unsigned char), total_bytes, arquivo);

    fclose(arquivo);
    return img;
}

void salvar_imagem_pgm_p5(const char *caminho, Imagem *img) {
    FILE *arquivo = fopen(caminho, "wb");
    if (!arquivo) {
        printf("Erro ao criar o arquivo de saída.\n");
        exit(1);
    }

    // Escreve o cabeçalho P5
    fprintf(arquivo, "P5\n%d %d\n255\n", img->largura, img->altura);
    
    // Grava os pixels binários de uma vez só
    int total_bytes = img->largura * img->altura * img->canais;
    fwrite(img->pixels, sizeof(unsigned char), total_bytes, arquivo);
    
    fclose(arquivo);
}

void salvar_imagem_ppm_p6(const char *caminho, Imagem *img) {
    FILE *arquivo = fopen(caminho, "wb");
    if (!arquivo) {
        printf("Erro ao criar o arquivo de saida PPM.\n");
        exit(1);
    }
    // Escreve o cabeçalho P6
    fprintf(arquivo, "P6\n%d %d\n255\n", img->largura, img->altura);

    // Grava os pixels binários (PPM tem 3 canais: RGB)
    fwrite(img->pixels, sizeof(unsigned char), img->largura * img->altura * 3, arquivo);
    fclose(arquivo);
}