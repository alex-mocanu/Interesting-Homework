#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NAME
#include "structuri.h"
#endif

void cautare(unsigned int** a, unsigned int h, unsigned int w, unsigned int i, unsigned int j, unsigned int k);
//functie de cautare a pixelilor dintr-un cluster

int compar(struct cluster* a, struct cluster* b)
{
  return a->nr_elem - b->nr_elem;
} //functie de comparare a clusterelor in functie de numarul lor de elemente

void dim_clus(struct param_clus x)
{
  unsigned int i, j, t = 1;
  struct cluster* b;

//Marcam pixelii "valizi":
  for(i = 0; i < x.h; i++)
    for(j = 0; j < x.w; j++)
      if(x.ref.B - x.off.B <= x.m[i][j].B && x.ref.B + x.off.B >= x.m[i][j].B)
        if(x.ref.G - x.off.G <= x.m[i][j].G && x.ref.G + x.off.G >= x.m[i][j].G)
          if(x.ref.R - x.off.R <= x.m[i][j].R && x.ref.R + x.off.R >= x.m[i][j].R)
            x.a[i][j] = 1;

//Marcam potentialele clusterele intr-o matrice sablon:
  for(i = 0; i < x.h; i++)
    for(j = 0; j < x.w; j++)
      if(x.a[i][j] == 1)
      {
        t++;
        cautare(x.a, x.h, x.w, i, j, t);
      }

//Retinem caracteristicile potentialelor clustere:
  b = calloc(t, sizeof(struct cluster));
  for(i = 0; i < x.h; i++)
    for(j = 0; j < x.w; j++)
      if(x.a[i][j] != 0)
      {
        b[x.a[i][j] - 2].ind = x.a[i][j];
        b[x.a[i][j] - 2].nr_elem++;
        if(b[x.a[i][j] - 2].nr_elem == 1)
        {
          b[x.a[i][j] - 2].sj.x = i;
          b[x.a[i][j] - 2].sj.y = j;
          b[x.a[i][j] - 2].ds.x = i;
          b[x.a[i][j] - 2].ds.y = j;
        }

        else
        {
          b[x.a[i][j] - 2].sj.x = i;

          if(j < b[x.a[i][j] - 2].sj.y)
          b[x.a[i][j] - 2].sj.y = j;

          if(j > b[x.a[i][j] - 2].ds.y)
          b[x.a[i][j] - 2].ds.y = j;
        }
      }
  qsort(b, t - 1, sizeof(struct cluster), compar);

  i = 0;
  while(b[i].nr_elem < (unsigned int)(x.h * x.w * x.P) && i < t - 1)
    i++;

//Retinem acum caracteristicile clusterelor:
  *x.nr = t - 1 - i;
  for(j = 0; j < *x.nr; j++)
  {
    x.v[j] = b[j + i];
    fprintf(x.f, "%d ", x.v[j].nr_elem); //afisam numarul de elemente ale clusterelor
                                        //in ordine crescatoare
  }
}

void cautare(unsigned int** a, unsigned int h, unsigned int w, unsigned int i, unsigned int j, unsigned int k)
{
  if(i == 0 && j == 0)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i, j + 1, k);
      cautare(a, h, w, i + 1, j, k);
    }

  if(i == h - 1 && j == w - 1)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i, j - 1, k);
      cautare(a, h, w, i - 1, j, k);
    }

  if(i == 0 && j == w - 1)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i, j - 1, k);
      cautare(a, h, w, i + 1, j, k);
    }

  if(i == h - 1 && j == 0)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i, j + 1, k);
      cautare(a, h, w, i - 1, j, k);
    }

  if(i == 0 && j != 0 && j != w - 1)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i, j - 1, k);
      cautare(a, h, w, i, j + 1, k);
      cautare(a, h, w, i + 1, j, k);
    }

  if(i == h - 1 && j != 0 && j != w - 1)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i, j - 1, k);
      cautare(a, h, w, i, j + 1, k);
      cautare(a, h, w, i - 1, j, k);
    }

  if(i != 0 && i != h - 1 && j == 0)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i - 1, j, k);
      cautare(a, h, w, i + 1, j, k);
      cautare(a, h, w, i, j + 1, k);
    }

  if(i != 0 && i != h - 1 && j == w - 1)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i - 1, j, k);
      cautare(a, h, w, i + 1, j, k);
      cautare(a, h, w, i, j - 1, k);
    }

  if(i != 0 && i != h - 1 && j != 0 && j != w - 1)
    if(a[i][j] == 1)
    {
      a[i][j] = k;
      cautare(a, h, w, i - 1, j, k);
      cautare(a, h, w, i + 1, j, k);
      cautare(a, h, w, i, j - 1, k);
      cautare(a, h, w, i, j + 1, k);
    }
}

