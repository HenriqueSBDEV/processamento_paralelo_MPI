#ifndef FILTER_SOURCE_H
#define FILTER_SOURCE_H

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "img_source.h"

// Função de filtro sequencial
void aplicar_filtro_gaussiano_sequencial(Imagem *img, int iteracoes);

// Função de filtro gaussiano MPI
void aplicar_filtro_gaussiano_mpi(
    unsigned char *pixels,
    unsigned char *resultado,
    int largura,
    int altura,
    int canais,
    int iteracoes
);

#endif