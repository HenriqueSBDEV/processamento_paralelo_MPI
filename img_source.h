#ifndef IMG_SOURCE_H
#define IMG_SOURCE_H

#include <stdio.h>
#include <stdlib.h>

// Struct para armazenar as informações da imagem, incluindo largura, altura, número de canais (RGB ou Cinza) e um ponteiro para os dados dos pixels.
typedef struct {
    int largura;
    int altura;
    int canais; // Vai guardar 3 para PPM (RGB) ou 1 para PGM (Cinza)
    unsigned char *pixels;
} Imagem;

Imagem ler_imagem_ppm_p6(const char *caminho);

Imagem ler_imagem_pgm_p5(const char *caminho);

void salvar_imagem_pgm_p5(const char *caminho, Imagem *img);

void salvar_imagem_ppm_p6(const char *caminho, Imagem *img);

#endif // IMG_SOURCE_H