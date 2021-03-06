#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NAME
#include "structuri.h"
#endif

void crop(struct pixel** m, struct cluster* c, unsigned int nr, char* name)
{
  unsigned int i, j;
  int n;
  char cifre[10], nume_fis[20], padding = 0;
  struct bmp_fileheader f_h;
  struct bmp_infoheader i_h;

  for(i = 0; i < nr; i++)
  {
//Cream headerele clusterelor:
    i_h.biSize = 40;
    i_h.width = c[i].ds.y - c[i].sj.y + 1;
    i_h.height = c[i].sj.x - c[i].ds.x + 1;
    i_h.planes = 1;
    i_h.bitPix = 24;
    i_h.biCompression = 0;
    i_h.biSizeImage = 3 * i_h.height * i_h.width + i_h.height * ((4 - 3 * i_h.width % 4) % 4);
    i_h.biXPelsPerMeter = 0;
    i_h.biYPelsPerMeter = 0;
    i_h.biClrUsed = 0;
    i_h.biClrImportant = 0;

    f_h.fileMarker1 = 'B';
    f_h.fileMarker2 = 'M';
    f_h.bfSize = 54 + i_h.biSizeImage;
    f_h.unused1 = 0;
    f_h.unused2 = 0;
    f_h.imageDataOffset = 54;

    n = i + 1;
    j = 0;
    while(n > 0)
    {
      cifre[j] = n % 10 + '0';
      n /= 10;
      j++;
    }

//Denumim imaginea clusterului:
    strcpy(nume_fis, name);
    nume_fis[strlen(name) - 4] = '_';
    nume_fis[strlen(name) - 3] = 'c';
    nume_fis[strlen(name) - 2] = 'r';
    nume_fis[strlen(name) - 1] = 'o';
    nume_fis[strlen(name)] = 'p';
    for(n = j - 1; n >= 0; n--)
      nume_fis[strlen(name) + j - n] = cifre[n];
    nume_fis[strlen(name) + j + 1] = '.';
    nume_fis[strlen(name) + j + 2] = 'b';
    nume_fis[strlen(name) + j + 3] = 'm';
    nume_fis[strlen(name) + j + 4] = 'p';
    nume_fis[strlen(name) + j + 5] = '\0';

//Generam imaginea clusterului:
    FILE* f = fopen(nume_fis, "wb");
    fwrite(&f_h, sizeof(struct bmp_fileheader), 1, f);
    fwrite(&i_h, sizeof(struct bmp_infoheader), 1, f);
    fseek(f, f_h.imageDataOffset, SEEK_SET);
    for(j = c[i].ds.x; j <= c[i].sj.x; j++)
    {
        for(n = c[i].sj.y; (unsigned int)n <= c[i].ds.y; n++)
        {
            fwrite(&m[j][n].B, 1, 1, f);
            fwrite(&m[j][n].G, 1, 1, f);
            fwrite(&m[j][n].R, 1, 1, f);
        }
        for(n = 0; n < (4 - 3 * i_h.width % 4) % 4; n++)
            fwrite(&padding, 1, 1, f); //adaugam octetii de padding
    }
  }
}

