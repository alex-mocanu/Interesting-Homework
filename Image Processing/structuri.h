#include "bmp_header.h"
#include <stdio.h>
#include <stdlib.h>

struct pixel{
  unsigned char R, G, B;
}; //structura ce retine caracteristicile de culoare ale unui pixel

struct coord{
  unsigned int x, y;
}; //strucutra ce retine coordonatele unui pixel

struct cluster{
  unsigned int nr_elem, ind;
  struct coord sj;
  struct coord ds; //coordonate colturi stanga-jos si dreapta-sus ale unui cluster
};

struct param_clus{
  FILE* f;
  struct pixel **m, ref, off;
  unsigned int h, w;
  double P;
  unsigned int **a;
  struct cluster* v;
  unsigned int *nr;
}; //structura ce retine parametrii functiei de la prima cerinta
