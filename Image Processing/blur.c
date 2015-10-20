#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NAME
#include "structuri.h"
#endif

void blur(struct pixel** m, unsigned int h, unsigned int w, struct cluster* c, unsigned int nr, unsigned int** m_c)
{
  unsigned int i, j, k, x, y;
  struct pixel** a;
  a = malloc(h * sizeof(struct pixel*));
  for(i = 0; i < h; i++)
    a[i] = malloc(w * sizeof(struct pixel));

  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++)
      a[i][j] = m[i][j]; //realizam o copie a matricei imagine

  for(i = 0; i < 100; i++) //parcurgem de 100 de ori ciclul de blurare
  {
    for(j = 0; j < nr; j++) //bluram fiecare cluster
    {
      for(x = c[j].ds.x; x <= c[j].sj.x; x++)
        for(y = c[j].sj.y; y<= c[j].ds.y; y++)
        {
          if(m_c[x][y] == c[j].ind)
          {
            if(x == 0 && y == 0)
            {
              a[x][y].R = (m[x + 1][y].R + m[x][y + 1].R) / 2;
              a[x][y].G = (m[x + 1][y].G + m[x][y + 1].G) / 2;
              a[x][y].B = (m[x + 1][y].B + m[x][y + 1].B) / 2;
            }

            if(x == 0 && y == w - 1)
            {
              a[x][y].R = (m[x + 1][y].R + m[x][y - 1].R) / 2;
              a[x][y].G = (m[x + 1][y].G + m[x][y - 1].G) / 2;
              a[x][y].B = (m[x + 1][y].B + m[x][y - 1].B) / 2;
            }

            if(x == h - 1 && y == 0)
            {
              a[x][y].R = (m[x - 1][y].R + m[x][y + 1].R) / 2;
              a[x][y].G = (m[x - 1][y].G + m[x][y + 1].G) / 2;
              a[x][y].B = (m[x - 1][y].B + m[x][y + 1].B) / 2;
            }

            if(x == h - 1 && y == w - 1)
            {
              a[x][y].R = (m[x - 1][y].R + m[x][y - 1].R) / 2;
              a[x][y].G = (m[x - 1][y].G + m[x][y - 1].G) / 2;
              a[x][y].B = (m[x - 1][y].B + m[x][y - 1].B) / 2;
            }

            if(x == 0 && (y != 0 && y != w - 1))
            {
              a[x][y].R = (m[x + 1][y].R + m[x][y - 1].R + m[x][y + 1].R) / 3;
              a[x][y].G = (m[x + 1][y].G + m[x][y - 1].G + m[x][y + 1].G) / 3;
              a[x][y].B = (m[x + 1][y].B + m[x][y - 1].B + m[x][y + 1].B) / 3;
            }

            if(x == h - 1 && (y != 0 && y != w - 1))
            {
              a[x][y].R = (m[x - 1][y].R + m[x][y - 1].R + m[x][y + 1].R) / 3;
              a[x][y].G = (m[x - 1][y].G + m[x][y - 1].G + m[x][y + 1].G) / 3;
              a[x][y].B = (m[x - 1][y].B + m[x][y - 1].B + m[x][y + 1].B) / 3;
            }

            if((x != 0 && x != h - 1) && y == 0)
            {
              a[x][y].R = (m[x + 1][y].R + m[x - 1][y].R + m[x][y + 1].R) / 3;
              a[x][y].G = (m[x + 1][y].G + m[x - 1][y].G + m[x][y + 1].G) / 3;
              a[x][y].B = (m[x + 1][y].B + m[x - 1][y].B + m[x][y + 1].B) / 3;
            }

            if((x != 0 && x != h - 1) && y == w - 1)
            {
              a[x][y].R = (m[x + 1][y].R + m[x - 1][y].R + m[x][y - 1].R) / 3;
              a[x][y].G = (m[x + 1][y].G + m[x - 1][y].G + m[x][y - 1].G) / 3;
              a[x][y].B = (m[x + 1][y].B + m[x - 1][y].B + m[x][y - 1].B) / 3;
            }

            if((x != 0 && x != h - 1) && (y != 0 && y != w - 1))
            {
              a[x][y].R = (m[x][y - 1].R + m[x][y + 1].R + m[x - 1][y].R + m[x + 1][y].R) / 4;
              a[x][y].G = (m[x][y - 1].G + m[x][y + 1].G + m[x - 1][y].G + m[x + 1][y].G) / 4;
              a[x][y].B = (m[x][y - 1].B + m[x][y + 1].B + m[x - 1][y].B + m[x + 1][y].B) / 4;
            }
          }
        }
    }

    for(j = 0; j < h; j++)
      for(k = 0; k < w; k++)
      {
        m[j][k].R = a[j][k].R;
        m[j][k].G = a[j][k].G;
        m[j][k].B = a[j][k].B;
      } //copiem modificarile in matricea imagine
  }

  for(i = 0; i < w; i++)
    free(a[i]);
  free(a);
}

