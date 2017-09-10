#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functii.h" //header ce contine declaratiile functiilor de baza

void pix_read(FILE* f, struct pixel* x)
{
  fread(&x->B, 1, 1, f);
  fread(&x->G, 1, 1, f);
  fread(&x->R, 1, 1, f);
} //citire a parametrilor de culoare ai unui pixel

int compare(struct cluster* a, struct cluster* b)
{
  return a->ind - b->ind;
} //functie de sortare a clusterelor dupa ordinea gasirii lor

int main()
{
  FILE* intr = fopen("input.txt", "r");
  struct bmp_fileheader f_head;
  struct bmp_infoheader i_head;
  struct pixel ref, off; //pixelii referinta si offset
  struct pixel **matrix, **matrix_cpy; //matricea de pixeli si o copie a ei
  char red, padding = 0; //caracter redundant si caracter pentru padding
  char output_name[20] = {"output.bmp"};
  double P; //procentul pentru gasirea clusterelor
  unsigned int i, j, k;
  unsigned int **matrix_clus, nr_clus = 0; //matrice de marcare a clusterelor si numarul lor
  struct cluster *clus; //structura in care retinem caracteristicile clusterelor

  fscanf(intr, "%hhu%c%hhu%c%hhu%c", &ref.R, &red, &ref.G, &red, &ref.B, &red);
  fscanf(intr, "%hhu%c%hhu%c%hhu", &off.R, &red, &off.G, &red, &off.B);
  fscanf(intr, "%lf", &P);

//Aici incepe cerinta 1:
  FILE* g1 =fopen("output.txt", "w");
  FILE* bmp = fopen("input.bmp", "rb");
  //Citire date pentru file header:
  fread(&f_head, sizeof(struct bmp_fileheader), 1, bmp);

  //Citire date pentru image header:
  fread(&i_head, sizeof(struct bmp_infoheader), 1, bmp);
  i_head.biXPelsPerMeter = 0;
  i_head.biYPelsPerMeter = 0;

  matrix = calloc(i_head.height, sizeof(struct pixel*));
  for(i = 0; i < (unsigned int)i_head.height; i++)
    matrix[i] = calloc(i_head.width, sizeof(struct pixel));

  matrix_cpy = calloc(i_head.height, sizeof(struct pixel*));
  for(i = 0; i < (unsigned int)i_head.height; i++)
    matrix_cpy[i] = calloc(i_head.width, sizeof(struct pixel));

  matrix_clus = calloc(i_head.height, sizeof(int*));
  for(i = 0; i < (unsigned int)i_head.height; i++)
    matrix_clus[i] = calloc(i_head.width, sizeof(int));

  clus = calloc(i_head.height * i_head.width, sizeof(struct cluster));

//Citim matricea imagine
  fseek(bmp, f_head.imageDataOffset, SEEK_SET);
  unsigned int poz = f_head.imageDataOffset; //pozitia de inceput a unei linii din matricea imagine
  for(i = 0; i < (unsigned int)i_head.height; i++)
  {
    for(j = 0; j < (unsigned int)i_head.width; j++)
    {
      pix_read(bmp, &matrix[i][j]);
      matrix_cpy[i][j] = matrix[i][j]; //realizarea copiei matricii imagine
    }
    poz += 3 * i_head.width + (4 - (3 * (i_head.width)) % 4) % 4;
    fseek(bmp, 0, poz); //pozitionare pe noua linie
  }

  //Afisam raspunsul pentru cerinta 1:
  struct param_clus x; //structura ce contine parametrii unei functii
  x.f = g1;
  x.m = matrix;
  x.ref = ref;
  x.off = off;
  x.h = i_head.height;
  x.w = i_head.width;
  x.P = P;
  x.a = matrix_clus;
  x.v = clus;
  x.nr = &nr_clus;
  dim_clus(x); //functia de cautare a clusterelor si afisare a dimensiunilor lor
              //in ordine crescatoare

//Aici incepe cerinta 2:
  char nFisOut[20] = {"output_blur.bmp"}; //numele imaginii blurate

  FILE* g2 = fopen(nFisOut, "wb");

//Generam si afisam matricea blurata
  blur(matrix_cpy, i_head.height, i_head.width, clus, nr_clus, matrix_clus);
  fwrite(&f_head, sizeof(struct bmp_fileheader), 1, g2);
  fwrite(&i_head, sizeof(struct bmp_infoheader), 1, g2);
  for(i = 0; i < (unsigned int)i_head.height; i++)
  {
    for(j = 0; j < (unsigned int)i_head.width; j++)
    {
      fwrite(&matrix_cpy[i][j].B, 1, 1, g2);
      fwrite(&matrix_cpy[i][j].G, 1, 1, g2);
      fwrite(&matrix_cpy[i][j].R, 1, 1, g2);
    }
    for(k = 0; k < (4 - (3 * (i_head.width)) % 4) % 4; k++)
      fwrite(&padding, 1, 1, g2); //adaugam octetii de padding
  }

//Aici incepe cerinta 3:
  //Sortam clusterele dupa ordinea aparitiei (indice):
  qsort(clus, nr_clus, sizeof(struct cluster), compare);
  crop(matrix, clus, nr_clus, output_name); //generare imagini separate pentru clustere

  for(i = 0; i < (unsigned int)i_head.height; i++)
    free(matrix[i]);
  free(matrix);

  for(i = 0; i < (unsigned int)i_head.height; i++)
    free(matrix_cpy[i]);
  free(matrix_cpy);

  for(i = 0; i < (unsigned int)i_head.height; i++)
    free(matrix_clus[i]);
  free(matrix_clus);

  free(clus);

  return 0;
}
