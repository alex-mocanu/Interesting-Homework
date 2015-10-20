#ifndef NAME
#define NAME #include "structuri.h"
#include "structuri.h"
#endif

void dim_clus(struct param_clus x);

void blur(struct pixel** m, unsigned int h, unsigned int w, struct cluster* c, unsigned int nr, unsigned int** m_c);

void crop(struct pixel** m, struct cluster* c, unsigned int nr, char* name);
