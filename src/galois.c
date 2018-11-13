/* *
 * Copyright (c) 2014, James S. Plank and Kevin Greenan
 * All rights reserved.
 *
 * Jerasure - A C/C++ Library for a Variety of Reed-Solomon and RAID-6 Erasure
 * Coding Techniques
 *
 * Revision 2.0: Galois Field backend now links to GF-Complete
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 *  - Neither the name of the University of Tennessee nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Jerasure's authors:

   Revision 2.x - 2014: James S. Plank and Kevin M. Greenan
   Revision 1.2 - 2008: James S. Plank, Scott Simmerman and Catherine D. Schuman.
   Revision 1.0 - 2007: James S. Plank
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "galois.h"

#define MAX_GF_INSTANCES 64

gf_t *galois_get_field_ptr(gf2_t* g, int w)
{
  if (g->gfp_array[w] != NULL) {
    return g->gfp_array[w];
  }

  return NULL;
}

gf2_t* galois_init_empty()
{
  gf2_t* ptr =  (gf2_t *) malloc(sizeof(gf2_t));
  memset(ptr, sizeof(gf2_t), 0);
  return ptr;
}

void galois_destroy(gf2_t* g)
{
	if (g) {
		free(g->gf);
		for (int i = 0; i < MAX_GF_INSTANCES; i ++)
			free(galois_get_field_ptr(g, i));
		free(g);
	}
}

gf2_t* galois_init_field(int w,
                        int mult_type,
                        int region_type,
                        int divide_type,
                        uint64_t prim_poly,
                        int arg1,
                        int arg2)
{
  int scratch_size;
  void *scratch_memory;
  gf2_t *gf2p;

  if (w <= 0 || w > 32) {
    fprintf(stderr, "ERROR -- cannot init default Galois field for w=%d\n", w);
    assert(0);
  }

  gf2p = (gf2_t *) malloc(sizeof(gf2_t));
  gf2p->gf = (gf_t *) malloc(sizeof(gf_t));
  if (!gf2p || gf2p->gf) {
    fprintf(stderr, "ERROR -- cannot allocate memory for Galois field w=%d\n", w);
    assert(0);
  }

  scratch_size = gf_scratch_size(w, mult_type, region_type, divide_type, arg1, arg2);
  if (!scratch_size) {
    fprintf(stderr, "ERROR -- cannot get scratch size for base field w=%d\n", w);
    assert(0);
  }

  scratch_memory = malloc(scratch_size);
  if (!scratch_memory) {
    fprintf(stderr, "ERROR -- cannot get scratch memory for base field w=%d\n", w);
    assert(0);
  }

  if(!gf_init_hard(gf2p->gf,
                   w, 
                   mult_type, 
                   region_type, 
                   divide_type, 
                   prim_poly, 
                   arg1, 
                   arg2, 
                   NULL, 
                   scratch_memory))
  {
    fprintf(stderr, "ERROR -- cannot init default Galois field for w=%d\n", w);
    assert(0);
  }

  gf2p->gfp_is_composite[w] = 0;
  return gf2p;
}

gf2_t* galois_init_composite_field(int w,
								   int region_type,
								   int divide_type,
								   int degree,
								   gf_t* base_gf)
{
  int scratch_size;
  void *scratch_memory;
  gf2_t *gf2p;
  
  if (w <= 0 || w > 32) {
    fprintf(stderr, "ERROR -- cannot init composite field for w=%d\n", w);
    assert(0);
  }
  
  gf2p = (gf2_t *) malloc(sizeof(gf2_t));
  gf2p->gf = (gf_t *) malloc(sizeof(gf_t));
  if (!gf2p || gf2p->gf) {
    fprintf(stderr, "ERROR -- cannot allocate memory for Galois field w=%d\n", w);
    assert(0);
  }

  scratch_size = gf_scratch_size(w, GF_MULT_COMPOSITE, region_type, divide_type, degree, 0);
  if (!scratch_size) {
    fprintf(stderr, "ERROR -- cannot get scratch size for composite field w=%d\n", w);
    assert(0);
  }

  scratch_memory = malloc(scratch_size);
  if (!scratch_memory) {
    fprintf(stderr, "ERROR -- cannot get scratch memory for composite field w=%d\n", w);
    assert(0);
  }

  if(!gf_init_hard(gf2p->gf,
                   w,
                   GF_MULT_COMPOSITE,
                   region_type,
                   divide_type,
                   0, 
                   degree, 
                   0, 
                   base_gf,
                   scratch_memory))
  {
    fprintf(stderr, "ERROR -- cannot init default composite field for w=%d\n", w);
    assert(0);
  }
  gf2p->gfp_is_composite[w] = 1;
  return gf2p;
}

int galois_init_default_field(gf2_t* g, int w)
{
  if (g->gfp_array[w] == NULL) {
    g->gfp_array[w] = (gf_t*)malloc(sizeof(gf_t));
    if(g->gfp_array[w] == NULL)
      return ENOMEM;
    if (!gf_init_easy(g->gfp_array[w], w))
      return EINVAL;
  }
  return 0;
}

int galois_uninit_field(gf2_t* g, int w)
{
  int ret = 0;
  if (g->gfp_array[w] != NULL) {
    int recursive = 1;
    ret = gf_free(g->gfp_array[w], recursive);
    free(g->gfp_array[w]);
    g->gfp_array[w] = NULL;
  }
  return ret;
}

static void galois_init(gf2_t* g, int w)
{
  if (w <= 0 || w > 32) {
    fprintf(stderr, "ERROR -- cannot init default Galois field for w=%d\n", w);
    assert(0);
  }

  switch (galois_init_default_field(g, w)) {
  case ENOMEM:
    fprintf(stderr, "ERROR -- cannot allocate memory for Galois field w=%d\n", w);
    assert(0);
    break;
  case EINVAL:
    fprintf(stderr, "ERROR -- cannot init default Galois field for w=%d\n", w);
    assert(0);
    break;
  }
}


static int is_valid_gf(gf_t *gf, int w)
{
  // TODO: I assume we may eventually
  // want to do w=64 and 128, so w
  // will be needed to perform this check
  (void)w;

  if (gf == NULL) {
    return 0;
  }
  if (gf->multiply.w32 == NULL) {
    return 0;
  }
  if (gf->multiply_region.w32 == NULL) {
    return 0;
  }
  if (gf->divide.w32 == NULL) {
    return 0;
  }
  if (gf->inverse.w32 == NULL) {
    return 0;
  }
  if (gf->extract_word.w32 == NULL) {
    return 0;
  }

  return 1;
}

void galois_change_technique(gf2_t *g, gf_t* gf, int w)
{
  if (w <= 0 || w > 32) {
    fprintf(stderr, "ERROR -- cannot support Galois field for w=%d\n", w);
    assert(0);
  }

  if (!is_valid_gf(gf, w)) {
    fprintf(stderr, "ERROR -- overriding with invalid Galois field for w=%d\n", w);
    assert(0);
  }

  if (g->gfp_array[w] != NULL) {
    gf_free(g->gfp_array[w], g->gfp_is_composite[w]);
  }

  g->gfp_array[w] = g;
}

int galois_single_multiply(gf2_t* g, int x, int y, int w)
{
  if (x == 0 || y == 0) return 0;
  
  if (g->gfp_array[w] == NULL) {
	  galois_init(g, w);
  }

  if (w <= 32) {
    return g->gfp_array[w]->multiply.w32(g->gfp_array[w], x, y);
  } else {
    fprintf(stderr, "ERROR -- Galois field not implemented for w=%d\n", w);
    return 0;
  }
}

int galois_single_divide(gf2_t* g, int x, int y, int w)
{
  if (x == 0) return 0;
  if (y == 0) return -1;

  if (g->gfp_array[w] == NULL) {
	  galois_init(g, w);
  }

  if (w <= 32) {
    return g->gfp_array[w]->divide.w32(g->gfp_array[w], x, y);
  } else {
    fprintf(stderr, "ERROR -- Galois field not implemented for w=%d\n", w);
    return 0;
  }
}

void galois_w08_region_multiply(gf2_t* g,
								  char *region,      /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,        /* Number of bytes in region */
                                  char *r2,          /* If r2 != NULL, products go here */
                                  int add)
{
  if (g->gfp_array[8] == NULL) {
	  galois_init(g, 8);
  }
  g->gfp_array[8]->multiply_region.w32(g->gfp_array[8], region, r2, multby, nbytes, add);
}

void galois_w16_region_multiply(gf2_t* g,
								char *region,      /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,        /* Number of bytes in region */
                                  char *r2,          /* If r2 != NULL, products go here */
                                  int add)
{
  if (g->gfp_array[16] == NULL) {
	  galois_init(g, 16);
  }
  g->gfp_array[16]->multiply_region.w32(g->gfp_array[16], region, r2, multby, nbytes, add);
}


void galois_w32_region_multiply(gf2_t* g,
								char *region,      /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,        /* Number of bytes in region */
                                  char *r2,          /* If r2 != NULL, products go here */
                                  int add)
{
  if (g->gfp_array[32] == NULL) {
	  galois_init(g, 32);
  }
  g->gfp_array[32]->multiply_region.w32(g->gfp_array[32], region, r2, multby, nbytes, add);
}

void galois_w8_region_xor(gf2_t* g, void *src, void *dest, int nbytes)
{
  if (g->gfp_array[8] == NULL) {
	  galois_init(g, 8);
  }
  g->gfp_array[8]->multiply_region.w32(g->gfp_array[32], src, dest, 1, nbytes, 1);
}

void galois_w16_region_xor(gf2_t* g, void *src, void *dest, int nbytes)
{
  if (g->gfp_array[16] == NULL) {
	  galois_init(g, 16);
  }
  g->gfp_array[16]->multiply_region.w32(g->gfp_array[16], src, dest, 1, nbytes, 1);
}

void galois_w32_region_xor(gf2_t* g, void *src, void *dest, int nbytes)
{
  if (g->gfp_array[32] == NULL) {
	  galois_init(g, 32);
  }
  g->gfp_array[32]->multiply_region.w32(g->gfp_array[32], src, dest, 1, nbytes, 1);
}

void galois_region_xor(gf2_t* g, char *src, char *dest, int nbytes)
{
  if (nbytes >= 16) {
	  galois_w32_region_xor(g, src, dest, nbytes);
  } else {
    int i = 0;
    for (i = 0; i < nbytes; i++) {
      *dest ^= *src;
      dest++;
      src++;
    } 
  }
}

int galois_inverse(gf2_t* g, int y, int w)
{
  if (y == 0) return -1;
  return galois_single_divide(g, 1, y, w);
}
