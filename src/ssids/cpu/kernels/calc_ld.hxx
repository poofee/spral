/* Copyright 2016 The Science and Technology Facilities Council (STFC)
 *
 * Authors: Jonathan Hogg (STFC)
 *
 * IMPORTANT: This file is NOT licenced under the BSD licence. If you wish to
 * licence this code, please contact STFC via hsl@stfc.ac.uk
 * (We are currently deciding what licence to release this code under if it
 * proves to be useful beyond our own academic experiments)
 *
 */
#pragma once

#include <cmath>
#include <stdexcept>

#include "common.hxx"
#include "SimdVec.hxx"

namespace spral { namespace ssids { namespace cpu {

/** Calculates LD from L and D.
 *
 * We assume that both l and ld are 32-bytes aligned, and ldl and ldld are
 * multiples of 32 bytes, so we can use AVX.
 */
template <enum operation op, typename T>
void calcLD(int m, int n, T const* l, int ldl, T const* d, T* ld, int ldld) {
   typedef SimdVec<T> SimdVecT;

   for(int col=0; col<n; ) {
      if(col+1==n || std::isfinite(d[2*col+2])) {
         // 1x1 pivot
         T d11 = d[2*col];
         if(d11 != 0.0) d11 = 1/d11; // Zero pivots just cause zeroes
         if(op==OP_N) {
            int const vlen = SimdVecT::vector_length;
            int const unroll = 4;
            int nvec = m / vlen;
            SimdVecT d11v(d11);
            if(nvec < unroll) {
               for(int row=0; row<nvec*vlen; row+=vlen) {
                  SimdVecT lv = SimdVecT::load_aligned(&l[col*ldl+row]);
                  lv = lv * d11;
                  lv.store_aligned(&ld[col*ldld+row]);
               }
            } else {
               int nunroll = nvec / unroll;
               for(int row=0; row<nunroll*unroll*vlen; row+=unroll*vlen) {
                  SimdVecT lv0 = SimdVecT::load_aligned(&l[col*ldl+row+0*vlen]);
                  SimdVecT lv1 = SimdVecT::load_aligned(&l[col*ldl+row+1*vlen]);
                  SimdVecT lv2 = SimdVecT::load_aligned(&l[col*ldl+row+2*vlen]);
                  SimdVecT lv3 = SimdVecT::load_aligned(&l[col*ldl+row+3*vlen]);
                  lv0 *= d11;
                  lv1 *= d11;
                  lv2 *= d11;
                  lv3 *= d11;
                  lv0.store_aligned(&ld[col*ldld+row+0*vlen]);
                  lv1.store_aligned(&ld[col*ldld+row+1*vlen]);
                  lv2.store_aligned(&ld[col*ldld+row+2*vlen]);
                  lv3.store_aligned(&ld[col*ldld+row+3*vlen]);
               }
               for(int row=nunroll*unroll*vlen; row<nvec*vlen; row+=vlen) {
                  SimdVecT lv = SimdVecT::load_aligned(&l[col*ldl+row]);
                  lv = lv * d11;
                  lv.store_aligned(&ld[col*ldld+row]);
               }
            }
            for(int row=nvec*vlen; row<m; row++)
               ld[col*ldld+row] = d11 * l[col*ldl+row];
         } else { /* op==OP_T */
            for(int row=0; row<m; row++)
               ld[col*ldld+row] = d11 * l[row*ldl+col];
         }
         col++;
      } else {
         // 2x2 pivot
         T d11 = d[2*col];
         T d21 = d[2*col+1];
         T d22 = d[2*col+3];
         T det = d11*d22 - d21*d21;
         d11 = d11/det;
         d21 = d21/det;
         d22 = d22/det;
         for(int row=0; row<m; row++) {
            T a1, a2;
            if(op==OP_N) {
               a1 = l[col*ldl+row];
               a2 = l[(col+1)*ldl+row];
            } else { /* op==OP_T */
               a1 = l[row*ldl+col];
               a2 = l[row*ldl+(col+1)];
            }
            ld[col*ldld+row]     =  d22*a1 - d21*a2;
            ld[(col+1)*ldld+row] = -d21*a1 + d11*a2;
         }
         col += 2;
      }
   }
}

}}} /* namespaces spral::ssids::cpu */