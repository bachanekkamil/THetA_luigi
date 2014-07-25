#ifndef GASV_ESP_H
#define GASV_ESP_H

#include "list.h" 
#include <stdio.h>

// DATATYPES

typedef struct {
  char *name;
  int x, y;
  unsigned int xlen, ylen;
  unsigned short chrx;
  unsigned short chry;
} esp_t;


// CONSTRUCTORS 

esp_t *esp(char *, unsigned short , unsigned short , int , int , unsigned int , unsigned int );
void esp_init(esp_t *, char *, unsigned short, unsigned short, int, int, unsigned int, unsigned int);
void esp_clear(esp_t *);
void esp_free(esp_t *);

// PREDICATES 

#define esp_divergent(pair) (!(esp_convergent(pair)))
unsigned char esp_convergent(esp_t *);
unsigned char esp_triangle(esp_t *, unsigned int);
int esp_yintercept_cmp(esp_t *, esp_t *);
int break_region_overlap(esp_t *, esp_t *, unsigned int , unsigned int );
int break_region_same_orientation(esp_t *, esp_t *);

// ACCESSORS

#define esp_lmax(pair) ((pair)->lmax)
int esp_lmin(esp_t *, unsigned int);
int break_region_slope(esp_t *);

// EVENT POINT

unsigned int esp_top_start_x(esp_t *, unsigned int, unsigned int);
unsigned int esp_top_end_x(esp_t *, unsigned int, unsigned int);
unsigned int esp_top_start_y(esp_t *, unsigned int, unsigned int);
unsigned int esp_top_end_y(esp_t *, unsigned int, unsigned int);
unsigned int esp_bottom_start_x(esp_t *, unsigned int, unsigned int);
unsigned int esp_bottom_end_x(esp_t *, unsigned int, unsigned int);
unsigned int esp_bottom_start_y(esp_t *, unsigned int, unsigned int);
unsigned int esp_bottom_end_y(esp_t *, unsigned int, unsigned int);
int esp_yintercept_start(esp_t *, unsigned int, unsigned int);
int esp_yintercept_end(esp_t *, unsigned int, unsigned int);
unsigned int esp_rightmost_x(esp_t *, unsigned int );
unsigned int esp_leftmost_x(esp_t *, unsigned int );
unsigned int esp_topmost_y(esp_t *, unsigned int );
unsigned int esp_bottommost_y(esp_t *, unsigned int );

// SERIALIZATION

int esp_read(esp_t *, FILE *);
list_t *file2break_regions(char *, unsigned int, unsigned int);

// UTILITY

#define sign(x) ((0 <= (x)) - (x < 0)) //Note: we treat 0 as positive. We have no 0-slop scan lines.

#endif
