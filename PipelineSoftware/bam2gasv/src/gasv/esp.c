#include "esp.h"
#include "gasv_logging.h"
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



// CONSTRUCTORS

esp_t *esp(char *name, unsigned short chrx, unsigned short chry, int x, int y, unsigned int xlen, unsigned int ylen) {
  esp_t *new_break_region = malloc(sizeof(esp_t));
  esp_init(new_break_region, name, chrx, chry, x, y, xlen, ylen);
  return new_break_region;
}

void esp_init(esp_t *esp, char *name, unsigned short chrx, unsigned short chry, int x, int y, unsigned int xlen, unsigned int ylen) {
  esp->name = name;
  esp->chrx = chrx;
  esp->chry = chry;
  esp->x = x;
  esp->y = y;
  esp->xlen = xlen;
  esp->ylen = ylen;  
}


void esp_clear(esp_t *pair) {
  free(pair->name);
}

void esp_free(esp_t *pair) {
  esp_clear(pair);
  free(pair);
}


// PREDICATES

unsigned char esp_convergent(esp_t *pair) {
  return 0 <+ pair->x && pair->y < 0;
}

unsigned char esp_triangle(esp_t *pair, unsigned int lmin) {
  //TODO unsign these types
  unsigned int left_side = (2 * pair->xlen * pair->xlen) + (2 * pair->ylen * pair->ylen);
  unsigned int right_side = lmin * lmin;
  return right_side < left_side;
}

int esp_yintercept_cmp(esp_t *a, esp_t *b) {
  static unsigned int lmin, lmax;
  
  // a == NULL is an out-of-band signal that b is a length 2 int array {lmin, lmax}. This is used to prep a pseudo-closure before it is passed to a higher order sort function.
  if (!a) {
    lmin = ((unsigned int *)b)[0];
    lmax = ((unsigned int *)b)[1];
    return -1;
  }
  return (int)esp_yintercept_start(a, lmin, lmax) - (int)esp_yintercept_start(b, lmin, lmax);
}

int break_region_same_orientation(esp_t *a, esp_t *b) {
  return sign(a->x) == sign(b->x) && sign(a->y) == sign(b->y);
}

int break_region_overlap(esp_t *a, esp_t *b, unsigned int lmin, unsigned int lmax) {
  assert(esp_yintercept_start(a, lmin, lmax) <= esp_yintercept_end(b, lmin, lmax));

  // If the scanline completely separates these trapezoids, they certainly cannot overlap.
  if (esp_yintercept_end(a, lmin, lmax) < esp_yintercept_start(b, lmin, lmax)) return 0;
  
  // Two ++ oriented shapes must be checked specially, as they violate the congruence properties that make the other shapes easier to check.
  if (4 == sign(a->x) + sign(a->y) + sign(b->x) + sign(b->y)) {
    // If b is entirely contained within start < end of a, it must be left or beneath a to avoid intersection.
    if (esp_yintercept_end(b, lmin, lmax) <= esp_yintercept_end(a, lmin, lmax)) {
      assert(0 && "My current understanding is that, since lmax is the same for all inversions, yintercept_end - yintercept start should be constant and b should never be entirely within the width of a.");
      if (esp_rightmost_x(b, lmax) < esp_leftmost_x(a, lmax)) return 0;
      if (esp_topmost_y(b, lmax) < esp_bottommost_y(a, lmax)) return 0; 
    }
    // If b extends beyond a's end, it must be above or to the right to avoid intersection.
    else {
      if (esp_topmost_y(a, lmax) < esp_bottommost_y(b, lmax)) return 0;
      if (esp_rightmost_x(a, lmax) < esp_leftmost_x(b, lmax)) return 0;
    }
  }
  else {
    // a whose leftmost segment is vertical can check that b is entirely to the left.
    if (1 == sign(a->x) && esp_rightmost_x(b, lmax) < esp_leftmost_x(a, lmax)) return 0;
    // a whose topmost segment is horizontal can check that b is entirely above.
    if (-1 == sign(a->y) && esp_topmost_y(a, lmax) < esp_bottommost_y(b, lmax)) return 0;
    // a whose bottommost segment is horizontal can check that b is entirely below.
    if (1 == sign(a->x) && 1 == sign(a->y) && esp_topmost_y(b, lmax) < esp_bottommost_y(a, lmax)) return 0; 
    // a whose rightmost segment is vertical can check that b is entirely to the right.
    if (-2 == sign(a->x) + sign(a->y) && esp_rightmost_x(a, lmax) < esp_leftmost_x(b, lmax)) return 0;
  }
  return 1;
}

// ACCESSORS

int esp_lmin(esp_t *pair, unsigned int lmin) {
  return esp_triangle(pair, lmin) ? pair->xlen + pair->ylen : lmin;
}

int break_region_slope(esp_t *br) {
  return -1 * sign(br->x) * sign(br->y);
}

// EVENT POINTS

// These functions return the x and y coordinates of the points making up the vertical and horizontal line segments of the trapezoids. Top and bottom correspond to the greater and lesser y coordinates of the segments as per the paper. Start and end refer to which points are encountered first and last as the scan line proceeds in order through the trapezoids.

unsigned int esp_top_start_x(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->x) + (lmax * sign(pair->x) * (pair->y < 0));
}

unsigned int esp_top_end_x(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->x) + (lmin * sign(pair->x) * (pair->y < 0));
}

unsigned int esp_top_start_y(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->y) + (lmin * sign(pair->y) * (0 <= pair->y));
}

unsigned int esp_top_end_y(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->y) + (lmax * sign(pair->y) * (0 <= pair->y));
}

unsigned int esp_bottom_start_x(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->x) + (lmin * sign(pair->x) * (0 <= pair->y));
}

unsigned int esp_bottom_end_x(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->x) + (lmax * sign(pair->x) * (0 <= pair->y));
}

unsigned int esp_bottom_start_y(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->y) + (lmax * sign(pair->y) * (pair->y < 0));
}

unsigned int esp_bottom_end_y(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return abs(pair->y) + (lmin * sign(pair->y) * (pair->y < 0));
}

int esp_yintercept_start(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return esp_top_start_y(pair, lmin, lmax) + (sign(pair->x) * sign(pair->y) * esp_top_start_x(pair, lmin, lmax));
}

int esp_yintercept_end(esp_t *pair, unsigned int lmin, unsigned int lmax) {
  return esp_top_end_y(pair, lmin, lmax) + (sign(pair->x) * sign(pair->y) * esp_top_end_x(pair, lmin, lmax));
}

unsigned int esp_rightmost_x(esp_t *pair, unsigned int lmax) {
  return abs(pair->x) + (lmax * (0 <= pair->x));
}

unsigned int esp_leftmost_x(esp_t *pair, unsigned int lmax) {
  return abs(pair->x) - (lmax * (pair->x < 0));
}

unsigned int esp_topmost_y(esp_t *pair, unsigned int lmax) {
  return abs(pair->y) + (lmax * (0 <= pair->y));
}

unsigned int esp_bottommost_y(esp_t *pair, unsigned int lmax) {
  return abs(pair->y) - (lmax * (pair->y < 0));
}




// SERIALIZATION

int esp_read(esp_t *pair, FILE *in) {
  char qname[1024];
  int x_end, y_start;
  char x_ori, y_ori;
  //TODO check on possible non int chrnames and what to do
  int bytes_read = fscanf(in, "%s %hd %d %d %c %hd %d %d %c\n", qname, &pair->chrx, &pair->x, &x_end, &x_ori, &pair->chry, &y_start, &pair->y, &y_ori);
  if (-1 == bytes_read) return bytes_read;
  pair->xlen = x_end - pair->x;
  pair->ylen = pair->y - y_start;
  pair->x = pair->x * (x_ori == '+' ? 1 : -1);
  pair->y = pair->y * (y_ori == '+' ? 1 : -1);
  pair->name = malloc(strlen(qname) + 1);
  strcpy(pair->name, qname);

  return bytes_read;
}

list_t *file2break_regions(char *path, unsigned int lmin, unsigned int lmax) {
  FILE *in = fopen(path, "r");
  list_t *break_regions = NULL;
  esp_t *break_region = malloc(sizeof(esp_t));
  while (-1 != esp_read(break_region, in)) {
    break_regions = cons(break_region, break_regions);
    break_region = malloc(sizeof(esp_t));
  }
  free(break_region);
  fclose(in);
  
  //break_regions = list_reverse(break_regions);

  int lmin_lmax[] = {lmin, lmax};
  esp_yintercept_cmp(NULL, (esp_t *)lmin_lmax);
  return list_mergesort(break_regions, (int (*)(void *, void *))esp_yintercept_cmp);
}
