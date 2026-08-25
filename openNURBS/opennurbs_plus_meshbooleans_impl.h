/*
//
// Copyright (c) 1993-2026 Robert McNeel & Associates. All rights reserved.
// OpenNURBS, Rhinoceros, and Rhino3D are registered trademarks of Robert
// McNeel & Associates.
//
// Permission is hereby granted, free of charge, to Giulio
// Piacentino, to deal in this file without restriction,
// including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// Redistributions of this source code file to the public are not
// allowed.
//
// This permission note and above copyright note are not removed.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// THIS FILE IS NOT PART OF OPENNURBS
//
////////////////////////////////////////////////////////////////
*/
#pragma once

#if !defined(OPENNURBS_PLUS) || defined(OPENNURBS_PUBLIC)
#error This file should not be distributed with the public opennurbs source code toolkit.
#endif

#if !defined(ON_RUNTIME_WIN)
#include <signal.h>
#endif

#include <cmath>
#include <cfloat>

#if !defined(OPENNURBS_PLUS_MESHBOOLEANS_INC_)
#define OPENNURBS_PLUS_MESHBOOLEANS_INC_

#define MX_USE_NEW_INTERSECT_DEFAULT true
#define MX_USE_NEW_BOOLEAN_DEFAULT true
#define MX_PRINT_WARNINGS_RELEASE true

#define MX_TRACE_BOTH_OVERLAPS_DEFAULT false

//if this is modified, then the corresponding RhinoCommon const needs to be modified
#define MX_DIMINISH_MAGNITUDE_COEFFICIENT_DEFAULT 0.0001
#define MX_TOLERANCE_WHEN_UNSET_DEFAULT ON_SQRT_EPSILON * 10
#define MX_COMPUTE_TOLERANCE_USED_VALUES false

#define ON_USE_NEW_MESH_VERTEX_ALIGNER

#if defined(ON_RUNTIME_WIN)
#define MX_DEBUG_BREAK __debugbreak()
#else
#define MX_DEBUG_BREAK raise(SIGTRAP)
#endif

enum class MX_LineIntersectionType : char
{
  Unknown, //not computed yet
  RedundantSkipped, //it's a secondary intersection on an edge
  RemovedAtEdge, //was Intersect or Snap, but it's on the end of an edge and across another edge, and the other two lines strictly go out of the triangle

  CollinearDisjoint,
  Parallel,
  SegmentsEndBefore,

  INTERSECTION_BOUNDARY,

  Intersect,
  IntersectForcedByTolerance,
  IntersectForcedByLogic,

  EDGE_BOUNDARY,

  SnapAtEndsOfA,
  SnapAtEndsOfB,

  ContinueEnd,

  SnapContinueEnd,

  CollinearAndOverlap,
};

void _mx_conditionaldebugBreak();

namespace MX
{
  // ============================================================================
  // Exact-sign geometric predicates for the mesh-boolean / intersection engine.
  //
  // This is the provably-EXACT "floor" beneath the fast-double and double-double
  // (MX::DoubleDouble) filters used below. The double-double arithmetic carries ~106
  // bits, but the exact sign of a 3x3 orientation determinant on double-precision
  // inputs can require up to 159+ significand bits (Shewchuk 1997). So double-double
  // is a correct *middle* filter, not an exact floor: near a degeneracy it can still
  // return the wrong sign, which flips a combinatorial decision (which side of an
  // edge, does a segment cross a triangle, in/out) and desynchronizes the two meshes'
  // views of a shared feature.
  //
  // When a determinant is within the rounding horizon of zero, the functions here
  // compute its sign EXACTLY using non-overlapping floating-point expansions:
  //   J. R. Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast
  //   Robust Geometric Predicates", Discrete & Computational Geometry 18:305-363,
  //   1997. (The algorithm is public domain.)
  //
  // Deliberate deviations from the reference predicates.c, and why:
  //
  //  * No exactinit(). Shewchuk computes the error-bound constants at run time;
  //    here they are constexpr, derived from the IEEE-754 double epsilon (2^-53).
  //    => no global mutable state, nothing to initialize, thread-safe by nature.
  //
  //  * Two-product uses fma() (Kahan's error-free transformation) instead of
  //    Dekker's splitting. Given a correctly rounded fma (guaranteed by C++11
  //    <cmath>), p = a*b; e = fma(a,b,-p) yields p + e == a*b EXACTLY, needs no
  //    "splitter" constant, and -- critically -- is immune to compiler FMA
  //    contraction: each rounded product is written to a named temporary before
  //    fma recovers its residual, so /fp:precise (MSVC default for opennurbs) and
  //    default contraction on clang/gcc cannot fuse away the error term. The
  //    two-sum steps are add/sub only and are likewise contraction-immune. This
  //    matches the MX::DoubleDouble arithmetic (opennurbs_plus_meshbooleans.cpp
  //    _product_error), which already relies on the same guarantee.
  //
  // The result of every public function is a SIGN in {-1, 0, +1}. It is exact:
  // +1/-1 iff the true determinant is strictly positive/negative, 0 iff the true
  // determinant is exactly zero. There is no scale dependence and no tolerance.
  // ============================================================================
  namespace Exact
  {
    // Half-ulp machine epsilon for IEEE-754 binary64: 2^-53. DBL_EPSILON is 2^-52
    // (the gap above 1.0), so Shewchuk's "epsilon" is half of it.
    constexpr double kEps = DBL_EPSILON * 0.5;

    // Static a-priori error-bound coefficients (Shewchuk, exactinit()). Applied to
    // the "permanent" (sum of absolute values of the determinant's terms) they bound
    // the round-off of the fast double estimate; clearing the bound proves the sign.
    constexpr double kOrient2dErrBoundA = (3.0 + 16.0 * kEps) * kEps;
    constexpr double kOrient3dErrBoundA = (7.0 + 56.0 * kEps) * kEps;

    // ---- error-free transformations (exact: the returned pair sums to the true value) ----

    // x = fl(a + b), returns x; tail = (a + b) - x exactly.  (Knuth / Shewchuk Two_Sum)
    inline double ee_two_sum(double a, double b, double& tail)
    {
      const double x = a + b;
      const double bvirt = x - a;
      const double avirt = x - bvirt;
      tail = (a - avirt) + (b - bvirt);
      return x;
    }

    // x = fl(a - b), returns x; tail = (a - b) - x exactly.  (Shewchuk Two_Diff)
    inline double ee_two_diff(double a, double b, double& tail)
    {
      const double x = a - b;
      const double bvirt = a - x;
      const double avirt = x + bvirt;
      tail = (a - avirt) + (bvirt - b);
      return x;
    }

    // x = fl(a + b) where |a| >= |b| is assumed; tail = (a + b) - x exactly.  (Fast_Two_Sum)
    inline double ee_fast_two_sum(double a, double b, double& tail)
    {
      const double x = a + b;
      tail = b - (x - a);
      return x;
    }

    // x = fl(a * b), returns x; tail = (a * b) - x exactly, via fma. (Kahan Two_Product)
    inline double ee_two_product(double a, double b, double& tail)
    {
      const double x = a * b;
      tail = std::fma(a, b, -x);
      return x;
    }

    // ---- expansion arithmetic (an "expansion" is a length-sorted, non-overlapping
    //      sequence of doubles whose exact sum is the represented value) ----

    // (a1,a0) - b  ->  (x2,x1,x0)
    inline void ee_two_one_diff(double a1, double a0, double b, double& x2, double& x1, double& x0)
    {
      double i;
      i = ee_two_diff(a0, b, x0);
      x2 = ee_two_sum(a1, i, x1);
    }

    // (a1,a0) - (b1,b0)  ->  (x3,x2,x1,x0)   (the exact difference of two two-doubles)
    inline void ee_two_two_diff(double a1, double a0, double b1, double b0,
                                double& x3, double& x2, double& x1, double& x0)
    {
      double j, z;
      ee_two_one_diff(a1, a0, b0, j, z, x0);
      ee_two_one_diff(j, z, b1, x3, x2, x1);
    }

    // h = e + f (both expansions). Returns length of h. Discards zero components
    // ("zeroelim"). h must hold up to elen+flen doubles. (Shewchuk fast_expansion_sum_zeroelim.)
    // Index reads are guarded so no element one-past-the-end is ever dereferenced.
    inline int ee_fast_expansion_sum(int elen, const double* e, int flen, const double* f, double* h)
    {
      double Q, Qnew, hh;
      int eindex = 0, findex = 0, hindex = 0;
      double enow = e[0], fnow = f[0];

      if ((fnow > enow) == (fnow > -enow)) { Q = enow; if (++eindex < elen) enow = e[eindex]; }
      else                                 { Q = fnow; if (++findex < flen) fnow = f[findex]; }

      if ((eindex < elen) && (findex < flen))
      {
        if ((fnow > enow) == (fnow > -enow)) { Qnew = ee_fast_two_sum(enow, Q, hh); if (++eindex < elen) enow = e[eindex]; }
        else                                 { Qnew = ee_fast_two_sum(fnow, Q, hh); if (++findex < flen) fnow = f[findex]; }
        Q = Qnew;
        if (hh != 0.0) h[hindex++] = hh;
        while ((eindex < elen) && (findex < flen))
        {
          if ((fnow > enow) == (fnow > -enow)) { Qnew = ee_two_sum(Q, enow, hh); if (++eindex < elen) enow = e[eindex]; }
          else                                 { Qnew = ee_two_sum(Q, fnow, hh); if (++findex < flen) fnow = f[findex]; }
          Q = Qnew;
          if (hh != 0.0) h[hindex++] = hh;
        }
      }
      while (eindex < elen) { Qnew = ee_two_sum(Q, enow, hh); if (++eindex < elen) enow = e[eindex]; Q = Qnew; if (hh != 0.0) h[hindex++] = hh; }
      while (findex < flen) { Qnew = ee_two_sum(Q, fnow, hh); if (++findex < flen) fnow = f[findex]; Q = Qnew; if (hh != 0.0) h[hindex++] = hh; }
      if ((Q != 0.0) || (hindex == 0)) h[hindex++] = Q;
      return hindex;
    }

    // h = e * b (expansion times a scalar). Returns length of h; h must hold up to
    // 2*elen doubles. (Shewchuk scale_expansion_zeroelim, fma variant -- no presplit.)
    inline int ee_scale_expansion(int elen, const double* e, double b, double* h)
    {
      double Q, sum, hh, product1, product0;
      int hindex = 0;

      Q = ee_two_product(e[0], b, hh);
      if (hh != 0.0) h[hindex++] = hh;
      for (int eindex = 1; eindex < elen; eindex++)
      {
        product1 = ee_two_product(e[eindex], b, product0);
        sum = ee_two_sum(Q, product0, hh);
        if (hh != 0.0) h[hindex++] = hh;
        Q = ee_fast_two_sum(product1, sum, hh);
        if (hh != 0.0) h[hindex++] = hh;
      }
      if ((Q != 0.0) || (hindex == 0)) h[hindex++] = Q;
      return hindex;
    }

    // The sign of an expansion equals the sign of its largest (last) component,
    // because the components are non-overlapping and length-sorted.
    inline int ee_sign(double top)
    {
      return (top > 0.0) ? 1 : (top < 0.0 ? -1 : 0);
    }

    // ---- exact predicates (always compute the exact sign; no filter) ----

    // Exact sign of the 2D orientation determinant of points (ax,ay),(bx,by),(cx,cy):
    //   det = (ax-cx)*(by-cy) - (ay-cy)*(bx-cx)
    // > 0 : c-a-b is a left turn (counter-clockwise);  < 0 : right turn;  0 : collinear.
    inline int orient2d_exact(double ax, double ay, double bx, double by, double cx, double cy)
    {
      // det = ax*(by-cy) + bx*(cy-ay) + cx*(ay-by), expanded exactly.
      double axby1, axby0, axcy1, axcy0;
      double bxcy1, bxcy0, bxay1, bxay0;
      double cxay1, cxay0, cxby1, cxby0;
      double aterms[4], bterms[4], cterms[4];

      axby1 = ee_two_product(ax, by, axby0);
      axcy1 = ee_two_product(ax, cy, axcy0);
      ee_two_two_diff(axby1, axby0, axcy1, axcy0, aterms[3], aterms[2], aterms[1], aterms[0]);

      bxcy1 = ee_two_product(bx, cy, bxcy0);
      bxay1 = ee_two_product(bx, ay, bxay0);
      ee_two_two_diff(bxcy1, bxcy0, bxay1, bxay0, bterms[3], bterms[2], bterms[1], bterms[0]);

      cxay1 = ee_two_product(cx, ay, cxay0);
      cxby1 = ee_two_product(cx, by, cxby0);
      ee_two_two_diff(cxay1, cxay0, cxby1, cxby0, cterms[3], cterms[2], cterms[1], cterms[0]);

      double v[8], w[12];
      const int vlen = ee_fast_expansion_sum(4, aterms, 4, bterms, v);
      const int wlen = ee_fast_expansion_sum(vlen, v, 4, cterms, w);
      return ee_sign(w[wlen - 1]);
    }

    // Exact sign of the 3x3 determinant whose rows are the vectors a, b, c:
    //   det = | ax ay az ; bx by bz ; cx cy cz | = a . (b x c)   (the scalar triple product)
    // This is orient3d of (a, b, c, origin); i.e. the signed volume of the tetrahedron
    // (0, a, b, c) times 6. Inputs are treated as exact doubles, which is what the
    // caller wants when a/b/c are already-formed vectors (e.g. V-p in a winding number).
    inline int det3x3_exact(double ax, double ay, double az,
                            double bx, double by, double bz,
                            double cx, double cy, double cz)
    {
      // Exact 2x2 minors of b,c: mx = by*cz-bz*cy, my = bz*cx-bx*cz, mz = bx*cy-by*cx.
      double mx[4], my[4], mz[4];
      double p1, p0, q1, q0;

      p1 = ee_two_product(by, cz, p0); q1 = ee_two_product(bz, cy, q0);
      ee_two_two_diff(p1, p0, q1, q0, mx[3], mx[2], mx[1], mx[0]);

      p1 = ee_two_product(bz, cx, p0); q1 = ee_two_product(bx, cz, q0);
      ee_two_two_diff(p1, p0, q1, q0, my[3], my[2], my[1], my[0]);

      p1 = ee_two_product(bx, cy, p0); q1 = ee_two_product(by, cx, q0);
      ee_two_two_diff(p1, p0, q1, q0, mz[3], mz[2], mz[1], mz[0]);

      // det = ax*mx + ay*my + az*mz, summed exactly.
      double sx[8], sy[8], sz[8];
      const int lx = ee_scale_expansion(4, mx, ax, sx);
      const int ly = ee_scale_expansion(4, my, ay, sy);
      const int lz = ee_scale_expansion(4, mz, az, sz);

      double t[16], u[24];
      const int lt = ee_fast_expansion_sum(lx, sx, ly, sy, t);
      const int lu = ee_fast_expansion_sum(lt, t, lz, sz, u);
      return ee_sign(u[lu - 1]);
    }

    // ---- filtered public entry points (fast double estimate, exact only on the knife-edge) ----

    // Sign of orient2d(a,b,c). Fast path is a few flops; the exact expansion runs only
    // when the estimate is within its proven round-off bound of zero.
    inline int orient2d(double ax, double ay, double bx, double by, double cx, double cy)
    {
      const double detleft  = (ax - cx) * (by - cy);
      const double detright = (ay - cy) * (bx - cx);
      const double det = detleft - detright;

      double detsum;
      if (detleft > 0.0)
      {
        if (detright <= 0.0) return ee_sign(det);
        detsum = detleft + detright;
      }
      else if (detleft < 0.0)
      {
        if (detright >= 0.0) return ee_sign(det);
        detsum = -detleft - detright;
      }
      else
      {
        return ee_sign(det);
      }

      const double errbound = kOrient2dErrBoundA * detsum;
      if (det >= errbound || -det >= errbound) return ee_sign(det);
      return orient2d_exact(ax, ay, bx, by, cx, cy);
    }

    // Sign of the 3x3 determinant (rows a,b,c) with the same fast-then-exact discipline.
    inline int det3x3(double ax, double ay, double az,
                      double bx, double by, double bz,
                      double cx, double cy, double cz)
    {
      const double bycz = by * cz, bzcy = bz * cy;
      const double bzcx = bz * cx, bxcz = bx * cz;
      const double bxcy = bx * cy, bycx = by * cx;
      const double det = ax * (bycz - bzcy) + ay * (bzcx - bxcz) + az * (bxcy - bycx);

      const double perm = std::fabs(ax) * (std::fabs(bycz) + std::fabs(bzcy))
                        + std::fabs(ay) * (std::fabs(bzcx) + std::fabs(bxcz))
                        + std::fabs(az) * (std::fabs(bxcy) + std::fabs(bycx));
      const double errbound = kOrient3dErrBoundA * perm;
      if (det > errbound || det < -errbound) return ee_sign(det);
      if (perm == 0.0) return 0;
      return det3x3_exact(ax, ay, az, bx, by, bz, cx, cy, cz);
    }
  } // namespace Exact

  template <typename T> inline int cmp(T val0, T val1)
  {
    return (val1 < val0) - (val0 < val1);
  }

  template <typename T> inline int cmp_decr(T val0, T val1)
  {
    return  (val0 < val1) - (val1 < val0);
  }

  template <typename T> inline bool eqls(T val0, T val1)
  {
    return *val0 == *val1;
  }

  /// UINT64 max size
  template <typename T> inline bool enum_has_flags(T enum_instance, T enum_flags)
  {
    return ((ON__UINT64)enum_instance & (ON__UINT64)enum_flags) == (ON__UINT64)enum_flags; //mac compiler
  }

  template <typename T> inline void rotate(T& o0, T& o1, T& o2)
  {
    T tmp = std::move(o0);
    o0 = std::move(o2);
    o2 = std::move(o1);
    o1 = std::move(tmp);
  }

  namespace template_impl_details {

    template <typename TKey, typename TValue, typename TValue2> void sort_key_values(
      ON_SimpleArray<TKey>& keys, TValue* values, TValue2* values2, int compareObjects(const TKey*, const TKey*), int compareObjectsContext(const TKey*, const TKey*, void*), void* context)
    {
      ON_SimpleArray<int> indices(keys.Count());
      for (int i = 0; i < keys.Count(); i++) indices.Append(i);

      if (compareObjects != nullptr)
      {
        keys.Sort(ON::sort_algorithm::quick_sort, indices.Array(), compareObjects);
      }
      else
      {
        keys.Sort(ON::sort_algorithm::quick_sort, indices.Array(), compareObjectsContext, context);
      }

      ON_SimpleArray<ON__UINT_PTR> back_indices(keys.Count()); back_indices.SetCount(keys.Count());
      ON_SimpleArray<bool> done(keys.Count());

      for (ON__UINT_PTR i = 0; i < keys.Count(); i++)
      {
        back_indices[indices[i]] = i;
        done.Append(false);
      }

      ON__UINT64 i = 0;
      while (i < keys.Count())
      {
        if (done[i])
        {
          i++;
        }
        else if (i != back_indices[i])
        {
          const ON__UINT_PTR& back_target = back_indices[i];
          keys.Swap((int)i, (int)back_target);
          if (values) std::swap(values[i], values[back_target]);
          if (values2) std::swap(values2[i], values2[back_target]);

          done[back_indices[i]] = true;
          back_indices.Swap((int)i, (int)back_indices[i]);
        }
        else i++;
      }
    }

    template <typename TKey> void sort_key_values(
      ON_SimpleArray<TKey>& keys, int compareObjectsContext(const TKey*, const TKey*, void*), void swap(unsigned, unsigned, void* context), void* context)
    {
      ON_SimpleArray<int> indices(keys.Count());
      for (int i = 0; i < keys.Count(); i++) indices.Append(i);

      keys.Sort(ON::sort_algorithm::quick_sort, indices.Array(), compareObjectsContext, context);

      ON_SimpleArray<ON__UINT_PTR> back_indices(keys.Count()); back_indices.SetCount(keys.Count());
      ON_SimpleArray<bool> done(keys.Count());

      for (ON__UINT_PTR i = 0; i < keys.Count(); i++)
      {
        back_indices[indices[i]] = i;
        done.Append(false);
      }

      ON__UINT64 i = 0;
      while (i < keys.Count())
      {
        if (done[i])
        {
          i++;
        }
        else if (i != back_indices[i])
        {
          const ON__UINT_PTR& back_target = back_indices[i];
          keys.Swap((int)i, (int)back_target);
          swap((unsigned)i, (unsigned)back_target, context);

          done[back_indices[i]] = true;
          back_indices.Swap((int)i, (int)back_indices[i]);
        }
        else i++;
      }
    }

  }

  template <typename TKey, typename TValue, typename TValue2> void sort_key_values(
    TKey* keys, int count, TValue* values, TValue2* values2, int compareObjects(const TKey*, const TKey*))
  {
    if (count == 0) return;
    ON_SimpleArray<TKey> array;
    array.SetArray(keys, count, count);
    template_impl_details::sort_key_values(array, values, values2, compareObjects, (int(*)(const TKey*, const TKey*, void*))nullptr, (void*)nullptr);
    array.KeepArray();
  }

  template <typename TKey, typename TValue, typename TValue2> void sort_key_values(
    ON_SimpleArray<TKey>& keys, TValue* values, TValue2* values2, int compareObjects(const TKey*, const TKey*))
  {
    template_impl_details::sort_key_values(keys, values, values2, compareObjects, (int(*)(const TKey*, const TKey*, void*))nullptr, (void*)nullptr);
  }

  template <typename TKey, typename TValue, typename TValue2> void sort_key_values(
    ON_SimpleArray<TKey>& keys, TValue* values, TValue2* values2, int compareObjects(const TKey*, const TKey*, void*), void* context)
  {
    template_impl_details::sort_key_values(keys, values, values2, (int(*)(const TKey*, const TKey*))nullptr, compareObjects, context);
  }

  template <typename TKey> void sort_key_values(
    ON_SimpleArray<TKey>& keys, int compareObjects(const TKey*, const TKey*, void*), void swap(unsigned, unsigned, void*), void* context)
  {
    template_impl_details::sort_key_values(keys, compareObjects, swap, context);
  }


  template <typename TKey, typename TValue> void sort_key_values(ON_SimpleArray<TKey>& keys, TValue* values, int compareObjects(const TKey*, const TKey*))
  {
    sort_key_values(keys, values, (int*)nullptr, compareObjects);
  }

  /*template <typename TKey, typename TValue, typename TValue2> void sort_key_values(
    ON_SimpleArray<TKey>& keys, int compareObjects(const TKey*, const TKey*, void*), void* context)
  {
    template_impl_details::sort_key_values(keys, (int(*)(const TKey*, const TKey*))nullptr, compareObjects, context);
  }*/


  template<class ForwardIt>
  ForwardIt unique(ForwardIt first, ForwardIt last)
  {
    if (first == last) return last;

    ForwardIt result = first;
    while (++first != last) {
      if (!(*result == *first) && ++result != first) {
        *result = std::move(*first);
      }
    }
    return ++result;
  }

  //indexing of last is as in std library. Returns new count
  template<class TKey, class TContext>
  ON__UINT64 unique_indirect(TKey key_first, TKey key_last, bool areEqual(TKey, TKey, TContext context), TContext context)
  {
    if (key_first == key_last) return 0;

    ON__UINT64 r_i = 0;
    ON__UINT64 f_i = 0;
    while ((key_first + (++f_i)) != key_last) {
      if (!areEqual(key_first + r_i, key_first + f_i, context) && key_first + (++r_i) != key_first + f_i) {
        *(key_first + r_i) = std::move(*(key_first + f_i));
      }
    }
    return ++r_i;
  }

  //indexing of last is as in std library. Returns new count
  template<class TKey, class TValue>
  ON__UINT64 unique_key_values(TKey key_first, TKey key_last, TValue values, bool areEqual(TKey, TKey))
  {
    if (key_first == key_last) return 0;

    ON__UINT64 r_i = 0;
    ON__UINT64 f_i = 0;
    while ((key_first + (++f_i)) != key_last) {
      if (!areEqual(key_first + r_i, key_first + f_i) && key_first + (++r_i) != key_first + f_i) {
        *(key_first + r_i) = std::move(*(key_first + f_i));
        if (values) *(values + r_i) = std::move(*(values + f_i));
      }
    }
    return ++r_i;
  }

  //indexing of last is as in std library. Returns new count
  template<class TKey, class TValue>
  ON__UINT64 unique_key_values(TKey key_first, TKey key_last, TValue values, bool areEqual(TKey, TKey, double), double threashold)
  {
    if (key_first == key_last) return 0;

    ON__UINT64 r_i = 0;
    ON__UINT64 f_i = 0;
    while ((key_first + (++f_i)) != key_last) {
      if (!areEqual(key_first + r_i, key_first + f_i, threashold) && key_first + (++r_i) != key_first + f_i) {
        *(key_first + r_i) = std::move(*(key_first + f_i));
        if (values) *(values + r_i) = std::move(*(values + f_i));
      }
    }
    return ++r_i;
  }

  template <typename T> inline unsigned char min_of_3(T val0, T val1, T val2)
  {
    unsigned char res;

    if (val1 < val0)
    {
      if (val2 < val1) res = 2u;
      else res = 1u;
    }
    else if (val2 < val1) res = ((val2 < val0) ? 2u : 0u);
    else res = 0u;

    return res;
  }


  template<class ForwardIt, class T>
  ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value, bool less_than(const T&, const T&, void*), void* context)
  {
    ForwardIt it;
    ON__INT_PTR count;
    count = last - first;

    while (count > 0)
    {
      it = first;
      ON__INT_PTR step = count >> 1;
      it += step;
      if (less_than(*it, value, context))
      {
        first = ++it;
        count -= step + 1;
      }
      else count = step;
    }
    return first;
  }

  template<class ForwardIt, class T>
  ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T& value, bool less_than(const T&, const T&, void*), void* context)
  {
    ForwardIt it;
    ON__INT_PTR count;
    count = last - first;

    while (count > 0)
    {
      it = first;
      ON__INT_PTR step = count >> 1;
      it += step;
      if (!less_than(value, *it, context))
      {
        first = ++it;
        count -= step + 1;
      }
      else count = step;
    }
    return first;
  }

  template<class ForwardIt, class T>
  inline std::pair<ForwardIt, ForwardIt> equal_range(
    ForwardIt first, ForwardIt last, const T& value, bool less_than(const T&, const T&, void*), void* context)
  {
    ON__INT_PTR count = last - first;

    while (true)
    {
      if (count <= 0) { last = first; break; }

      ON__INT_PTR count2 = count >> 1;
      ForwardIt middle = first + count2;
      if (less_than(*middle, value, context))
      {
        first = (middle + 1);
        count -= count2 + 1;
      }
      else if (less_than(value, *middle, context))
      {
        count = count2;
      }
      else
      {
        auto first2 = lower_bound(first, middle, value, less_than, context);
        auto last2 = upper_bound(middle + 1, first + count, value, less_than, context);
        last = last2;
        first = first2;
        break;
      }
    }

    return { first, last };
  }


  class DynBitArray {
    using Element = unsigned int;

    static const ON__UINT64 element_bits = CHAR_BIT * sizeof(Element);
    static inline ON__UINT64 size_from_element_bits(ON__UINT64 bigCount)
    {
      return (bigCount + DynBitArray::element_bits - 1) / element_bits;
    }
    static unsigned number_of_set_bits(unsigned i) //popcount or sideways addition
    {
#if defined(ON_RUNTIME_WIN)
      i = i - ((i >> 1u) & 0x55555555u);
      i = (i & 0x33333333u) + ((i >> 2u) & 0x33333333u);
      return (((i + (i >> 4u)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24u;
#else
      return (unsigned)__builtin_popcount(i);
#endif
    }

    ON__UINT64 m_count;
    ON_SimpleArray<Element> m_array;
    int PrivateResetCount(ON__UINT64 count);

  public:
    DynBitArray() ON_NOEXCEPT : m_count(0), m_array() {};
    DynBitArray& operator=(const DynBitArray&);
    ~DynBitArray();

    void Init(ON__UINT64 count, bool value);
    
    void MemSet(bool value);
    void Destroy();
    bool operator[](ON__UINT64 index) const;
    void Set(ON__UINT64 index, bool value);
    void Resize(ON__UINT64 index, bool value);
    ON__UINT64 Count() const;
    unsigned TruesCount() const;
  };

  class ON_2udex_hash
  {
  public:
    std::size_t operator()(const ON_2udex& item) const
    {
      return std::hash<ON__UINT64>()(
        ((((ON__UINT64)item.i) << 24U | ((ON__UINT64)item.i) >> 40U))
          ^ (ON__UINT64)item.j);
    }
  };

  class ON_4udex_hash
  {
  public:
    std::size_t operator()(const ON_4udex& item) const
    {
      return std::hash<ON__UINT64>()(
        (((ON__UINT64)item.i) << 24U | ((ON__UINT64)item.i) >> 40U) ^
        (((ON__UINT64)item.j) << 16U | ((ON__UINT64)item.j) >> 32U) ^
        (((ON__UINT64)item.k) << 8U | ((ON__UINT64)item.k) >> 56U) ^
        ((ON__UINT64)item.l));
    }
  };

  ///<summary>
  /// Marks mesh intersection events for consideration if they happen within the same mesh, between different meshes, or in any case.
  ///</summary>
  enum class SetsCombinations : signed char
  {
    ///<summary>
  /// Nothing is requested explicitly.
  ///</summary>
    None = 0,

    ///<summary>
    /// Events happen within the mesh only. This means that each mesh is computed separately and only references itself.
    ///</summary>
    Self = 1,

    ///<summary>
    /// Events happen only between different meshes. This means that self-intersections within a single mesh are invisible.
    ///</summary>
    Cross = 2,

    ///<summary>
    /// All events. This set includes all geometric intersections, irrespectively of their origin.
    ///</summary>
    All = Self | Cross,

    ///<summary>Selective: only the first mesh is intersected with all others, so its self-intersections are invisible.</summary>
    FirstMeshWithRest = 4,
  };

  inline SetsCombinations operator|(MX::SetsCombinations a, MX::SetsCombinations b) { return static_cast<MX::SetsCombinations>(static_cast<int>(a) | static_cast<int>(b)); }

  class ManagedMeshes
  {
    ON_SimpleArray<const ON_Mesh*> inputs;

  public:
    ON_SimpleArray<const ON_Mesh*> intertwined_list;
    ON_SimpleArray<ON_Mesh*> worked_objects;

   explicit ManagedMeshes(const ON_SimpleArray<const ON_Mesh*>* inputs)
    {
      unsigned count = 0U;
      if (inputs) count = inputs->UnsignedCount();
      worked_objects.SetCapacity((int)count); worked_objects.SetCount((int)count); worked_objects.MemSet(0);
      if (inputs) { intertwined_list = *inputs; this->inputs = *inputs; }
    }
    void SetIfExistsAndThenMaintain(unsigned index, ON_Mesh* mesh) {
      if (mesh)
      {
        if (worked_objects[index] != nullptr) delete worked_objects[index];
        worked_objects[index] = mesh;
        intertwined_list[index] = mesh;
      }
    }
    const ON_Mesh* Get(unsigned index)const {
      return intertwined_list[index];
    }
    unsigned Count() { return intertwined_list.UnsignedCount(); }
    void AppendThirdPartyOwned(const ON_Mesh* item)
    {
      inputs.Append(item);
      worked_objects.Append(nullptr);
      intertwined_list.Append(item);
    }
    void AcceptOrfans(ON_SimpleArray<ON_Mesh*>* new_meshes)
    {
      if (!new_meshes) return;
      if (new_meshes->UnsignedCount() == 0) return;
      if (new_meshes->UnsignedCount() != Count()) ON_ERROR("Unexpected different array length");
      for (unsigned i = 0; i < new_meshes->UnsignedCount(); i++)
      {
        SetIfExistsAndThenMaintain(i, new_meshes->operator[](i));
      }
    }
    void Clear()
    {
      for (unsigned i = 0; i < worked_objects.UnsignedCount(); i++) { if (worked_objects[i] != nullptr) { delete worked_objects[i]; worked_objects[i] = nullptr; intertwined_list[i] = nullptr; } }
    }
    void Empty()
    {
      Clear();
      inputs.SetCount(0);
      worked_objects.SetCount(0);
      intertwined_list.SetCount(0);
    }
    void GiveAllInAdoption(ON_SimpleArray<ON_Mesh*>& to_this)
    {
      to_this = worked_objects;
      worked_objects.MemSet(0);
    }
    ~ManagedMeshes() { Clear(); }
  };

  // Allows to transparently see any ON_Mesh faces list as a list of ON_MeshTriangles, without copying any face.
  class ON_CLASS Tessellation
  {
  public:
    Tessellation(const ON_Mesh* mesh, unsigned original_index, bool computeNakedEdges, bool computeNakedVertices);
    ~Tessellation();
    Tessellation& operator=(const Tessellation&) = delete;
    void Destroy();

    ON_MeshTriangle TriangleAt(unsigned index) const;
    unsigned char NakedAt(const ON_MeshTriangle& triangle) const;
    ON_Triangle TrianglePointsAt(unsigned index) const;
    unsigned FaceIndexForTriangleAt(unsigned index) const;
    unsigned FaceIndexForTriangleAt(unsigned index, bool& isSecondQuadTriangle) const;
    unsigned TriangleIndexForFaceAt(unsigned index, bool secondQuadTriangle) const;
    bool IsSecondTriangleInQuadIndex(unsigned ti0, unsigned ti1) const;
    bool IsTrianglePartOfQuad(unsigned ti) const;
    unsigned GetTriangleCount() const;
    unsigned GetFaceCount() const;
    const ON_Mesh* GetMesh() const;
    ON_3dPoint Vertex(unsigned index) const;
    ON_3dPoint Vertex(int index) const;
    unsigned GetVertexCount() const;

    // m_F indices, then mesh_ngon indices
    unsigned GetFaceNgonIndexForFaceIndex(unsigned fi) const;

    //if the triangle index references a triangulized quad, returns the index of the other quad triangle. Otherwise, returns -1. Index must exist
    unsigned OtherPartOfQuad(unsigned ti) const;
    unsigned m_original_index;
    ON_SimpleArray<ON_2udex> m_naked_edges;
    DynBitArray* m_naked_vertices;

  private:
    // Required: the constructor returns early when mesh is nullptr.
    const ON_SimpleArray<ON_MeshFace>* m_F = nullptr;
    const ON_Mesh* m_mesh = nullptr;

    ON_SimpleArray<unsigned int> m_F_to_T;
    ON_SimpleArray<unsigned int> m_T_to_F;

    const unsigned int* m_ngon_map = nullptr;
    bool m_ngon_map_needs_destruction = false;

#if defined(ON_DEBUG)
    ON_SimpleArray<ON_MeshTriangle>* m_tris_for_debug = nullptr;
#endif
  };

  class LazyPlane
  {
  public:
    mutable ON_Plane m_plane;
    mutable char m_state;

  private:
    inline bool Upgrade() const;
    static constexpr double one_third = 1.0 / 3.0;

  public:
    const static LazyPlane UnsetPlane;
    inline bool IsValid() const;
    inline ON_3dVector Normal() const;
    inline bool CreateFromPoints(const ON_3dPoint& p, const ON_3dPoint& q, const ON_3dPoint& r);
    inline bool CreateFromPoints(const ON_Triangle& tri);
    inline double DistanceTo(const ON_3dPoint& p) const;
    inline bool ClosestPointTo(ON_3dPoint p, double* s, double* t) const;
  };
  
  class ON_CLASS Tolerance
  {
    double m_intersect_tolerance;
    double m_sq_intersect_tolerance;
    double m_overlap_tolerance;
    double m_sq_overlap_tolerance;

#if MX_COMPUTE_TOLERANCE_USED_VALUES
    double m_intersect_tolerance_used;
    double m_sq_intersect_tolerance_used;
    double m_overlap_tolerance_used;
    double m_sq_overlap_tolerance_used;
#endif

  public:
    void Setup(double intersect_tolerance, double overlap_tolerance)
    {
      if (ON_IS_NAN(intersect_tolerance) || !ON_IS_VALID(intersect_tolerance))
        intersect_tolerance = MX_TOLERANCE_WHEN_UNSET_DEFAULT;

      m_intersect_tolerance = abs(intersect_tolerance); m_sq_intersect_tolerance = intersect_tolerance * intersect_tolerance;

      if (ON_IS_NAN(overlap_tolerance) || !ON_IS_VALID(overlap_tolerance))
        overlap_tolerance = MX_TOLERANCE_WHEN_UNSET_DEFAULT;

      m_overlap_tolerance = abs(overlap_tolerance); m_sq_overlap_tolerance = overlap_tolerance * overlap_tolerance;
#if MX_COMPUTE_TOLERANCE_USED_VALUES
      m_intersect_tolerance_used = 0.0;
      m_sq_intersect_tolerance_used = 0.0;
      m_overlap_tolerance_used = 0.0;
      m_sq_overlap_tolerance_used = 0.0;
#endif
    }

    bool IsInIntersectTolerance(double detectedPositive);
    bool IsInIntersectSquareTolerance(double detectedPositive);
    bool IsInOverlapTolerance(double detectedPositive);
    bool IsInOverlapSquareTolerance(double detectedPositive);

    double IntersectTolerance() const;
    double OverlapTolerance() const;
    double OverlapToleranceSquared() const;
    double LargerTolerance() const;

    void SignalIntersectToleranceUsed(double detected);
    void SignalSquaredIntersectToleranceUsed(double detected);
    void SignalOverlapToleranceUsed(double detected);
    void SignalSquaredOverlapToleranceUsed(double detected);

    static Tolerance& ZeroTolerance()
    {
      static Tolerance instance(ON_ZERO_TOLERANCE / (2 << 9), ON_ZERO_TOLERANCE / (2 << 9));
      return instance;
    }

    static Tolerance& OnZeroTolerance()
    {
      static Tolerance instance(ON_ZERO_TOLERANCE, ON_ZERO_TOLERANCE);
      return instance;
    }

    static Tolerance CreateTolerance(double intersect, double overlap = ON_UNSET_VALUE)
    {
      Tolerance instance(intersect, ON_IS_VALID(overlap) ? (overlap >= intersect ? overlap : intersect): intersect);
      return instance;
    }

    static bool TolerancePredicate(double* a, double* b, double tolerance)
    {
      return (fabs(*a - *b) <= tolerance);
    }

    explicit Tolerance(double intersectAndOverlap) { Setup(intersectAndOverlap, intersectAndOverlap); }

  private:
    Tolerance() = default;
    Tolerance(double intersect, double overlap) { Setup(intersect, overlap); }
    //Tolerance(Tolerance& other) = default;
    //Tolerance& operator=(Tolerance& other) = delete;
  };

  //This class needs to have the size of ON__INT_PTR, as it's used in ON_RTree on the ON__INT_PTR part.
  class GroupItemPair
  {
  public:
    unsigned int m_GroupIndex;
    unsigned int m_InnerItemIndex;

    GroupItemPair() = default;
    explicit GroupItemPair(ON__INT_PTR ptr);
    explicit GroupItemPair(ON__UINT_PTR ptr);
    GroupItemPair(unsigned groupIndex, unsigned itemIndex);
    static GroupItemPair CreateGroupItemPair(ON__UINT64 specialIndex);
    operator void*() const;
    friend bool operator>(const GroupItemPair& one, const GroupItemPair& other);
    friend bool operator>=(const GroupItemPair& one, const GroupItemPair& other);
    friend bool operator==(const GroupItemPair& one, const GroupItemPair& other);
    bool IsSpecialIndex() const;
    ON__UINT64 SpecialIndexValue() const;
    ON__UINT64 RoughValue() const;
    void MakeIncreasing();
    static int Compare(const GroupItemPair* a, const GroupItemPair* b);

    static GroupItemPair Unset;
  };

  struct TwoGroupItemPairs
  {
    GroupItemPair A;
    GroupItemPair B;
  };

  using MX_Opts = ON_MeshIntersectionOptions::ON_MeshIntersectionOptionsImplementation;
  class ScalableReporter;

  /// <summary>
  /// Don't export this class to the SDK. It's an internal class.
  /// </summary>
  class ON_CLASS InnerSettings
  {
  public:
    MX_Opts* m_options;
    SetsCombinations m_sets;
    bool m_dike;
    bool m_compute_geometry;
    bool m_final_target_is_split;

    /// <summary>
    /// possibly expose this to the SDK user
    /// </summary>
    bool m_parallel;

    InnerSettings();
    InnerSettings(MX_Opts* m_options, SetsCombinations sets);
    InnerSettings(MX_Opts* m_options, SetsCombinations sets, bool dam);
    InnerSettings(MX_Opts* m_options, SetsCombinations sets, bool dam, bool computeGeometry, bool finalTargetIsSplit, bool parallel);

    void ReportProgress(double value) const;
    MX::ScalableReporter* Reporter() const;
  };

  class TessellationArray : public ON_SimpleArray<const ON_Mesh*>
  {
  public:
    TessellationArray();
    explicit TessellationArray(const ON_SimpleArray<const ON_Mesh*>& meshes, bool nakedEdgeList, bool nakedVerticesList, bool sort);

    void SetContent(const ON_SimpleArray<const ON_Mesh*>& meshes, bool nakedEdgeList, bool nakedVerticesList = false, bool sort = true);
    void SetContent(const ON_SimpleArray<ON_Mesh*>& meshes, bool nakedEdgeList, bool nakedVerticesList) { SetContent((const ON_SimpleArray<const ON_Mesh*>&)meshes, nakedEdgeList, nakedVerticesList); };
    void SetContent(const ON_Mesh& mesh, bool nakedEdgeList, bool nakedVerticesList = false, bool sort = true);

    void Empty();

    bool CleanUpWithTolerance(double tolerance, ON_SimpleArray<ON_Mesh*>* results, bool mendDoubleToSingleJump, bool onlyNaked, bool A, bool B, bool C, InnerSettings& settings) const;

    unsigned TotalTriangles() const;
    unsigned TotalVertices() const;
    const Tessellation* At(unsigned int) const;
    const Tessellation* At(int) const;

    ON_3dPoint Vertex(const GroupItemPair& pair) const;

    ON_SimpleArray<class MX_3dPointSparseEnumerator*>* ToSimpleArrayOfWraps() const;
    
    //const MeshTessellation* CollectionForTriangleAt(unsigned int ti, unsigned int& out_index) const;
    //const MeshTessellation* CollectionForTriangleAt(unsigned int) const;

    //base class has a virtual Remove(int) method, so dynamic_cast will work.

    template<class TT> void ReorderWithInput(TT* ptrs) const
    {
      ON_SimpleArray<TT> intermediate{ UnsignedCount() };
      intermediate.SetCount(Count());
      intermediate.MemSet(0);
      for (unsigned i = 0; i < UnsignedCount(); i++)
      {
        intermediate[i] = ptrs[i];
      }

      for (unsigned i = 0; i < UnsignedCount(); i++)
      {
        ptrs[At(i)->m_original_index] = intermediate[i];
      }
    }

    ~TessellationArray();

  private:
    ON_SimpleArray<Tessellation*>* m_coll;
    //ON_SimpleArray<unsigned int>* m_ti_to_coll_map;
    //ON_SimpleArray<unsigned int>* m_coll_to_t_map;
    unsigned m_total_triangles;
    unsigned m_total_vertices;
  };

  //Contains a simple doubledouble class that performs ONLY multiplication and subtraction
  //Subset of implementation of "Library for Double-Double and Quad-Double Arithmetic" paper and
  //on "Quad Double computation package" by Yozo Hida, Xiaoye S. Li, and David H. Bailey
  class DoubleDouble
  {
  public:
    DoubleDouble() : High(0.0), Low(0.0) {}
    DoubleDouble(double High, double Low) : High(High), Low(Low) {}
    DoubleDouble(double h) : High(h), Low(0.0) {}

    static DoubleDouble multiplyinto(double a, double b);
    static DoubleDouble subtractinto(double a, double b);
    friend DoubleDouble operator-(const DoubleDouble &a, const DoubleDouble &b);
    friend DoubleDouble operator/(const DoubleDouble &a, const DoubleDouble &b);
    friend DoubleDouble operator*(const DoubleDouble &a, double b);
    friend DoubleDouble operator*(double a, const DoubleDouble& b);
    friend DoubleDouble operator*(const DoubleDouble& a, const DoubleDouble& b);
    explicit operator double() const;

    double High, Low;
  };

  enum class Orientation : signed char
  {
    Negative = -1,
    Equal = 0,
    Positive = 1,
    Unknown = 2,
  };

  class Triangle2d
  {
  public:
    Orientation FindOrientation() const;
    void Flip();
    static Triangle2d CreateFromProjection(const LazyPlane& pl, const ON_Triangle& tri);

  public:
    ON_2dPoint m_p[3];
  };

  enum class BarycentricSide : unsigned char
  {
    Unset = 0U,
    Valid = 1U,
    EdgeOppositeOfU = 1U << 1,
    EdgeOppositeOfV = 1U << 2,
    EdgeOppositeOfW = 1U << 3,
    VertexU = EdgeOppositeOfV | EdgeOppositeOfW,
    VertexV = EdgeOppositeOfU | EdgeOppositeOfW,
    VertexW = EdgeOppositeOfU | EdgeOppositeOfV,
    AllVertices = EdgeOppositeOfU | EdgeOppositeOfV | EdgeOppositeOfW,
    WasMovedByToleranceFromInside = 1U << 6,
    WasMovedByToleranceFromOutside = 1U << 7,
  };

  BarycentricSide operator |(const BarycentricSide& left, const BarycentricSide& right);
  BarycentricSide& operator |=(BarycentricSide& left, const BarycentricSide& right);

  class BarycentricCoords
  {
  public:
    double m_U, m_V, m_W;
    BarycentricSide m_triangle_side;

    template<class T> T Evaluate(const T& v0, const T& v1, const T& v2) const
    {
      return (v0 * m_U) + (v1 * m_V) + (v2 * m_W);
    }

    // [Giulio] Steve reported RH-67967: GCC has a bug that prohibits template specialization here. Going with overloading
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
    ON_4dPoint Evaluate(const ON_4dPoint& v0, const ON_4dPoint& v1, const ON_4dPoint& v2) const
    {
      return ON_4dPoint { 
        v0.x * m_U + v1.x * m_V + v2.x * m_W,
        v0.y * m_U + v1.y * m_V + v2.y * m_W,
        v0.z * m_U + v1.z * m_V + v2.z * m_W,
        v0.w * m_U + v1.w * m_V + v2.w * m_W,
      };
    }

    template<class T> T EvaluateFloat(const T& v0, const T& v1, const T& v2) const
    {
      return (v0 * (float)m_U + v1 * (float)m_V + v2 * (float)m_W);
    }
    ON_Color EvaluateColor(const ON_Color& v0, const ON_Color& v1, const ON_Color& v2) const;
    template<class T> T EvaluateArray(const ON_SimpleArray<T>& coords, const ON_MeshTriangle& triangle) const
    {
      return Evaluate<T>(coords[triangle.m_vi[0]], coords[triangle.m_vi[1]], coords[triangle.m_vi[2]]);
    }
    template<class T> T EvaluateArrayFloat(const ON_SimpleArray<T>& coords, const ON_MeshTriangle& triangle) const
    {
      return EvaluateFloat<T>(coords[triangle.m_vi[0]], coords[triangle.m_vi[1]], coords[triangle.m_vi[2]]);
    }

    static BarycentricCoords FromVW(double v, double w, const ON_3dPoint& pt, const ON_Triangle& triangle, Tolerance& tolerance);
    static BarycentricCoords FromVWNoPoint(double v, double w, const ON_Triangle& triangle, Tolerance& tolerance);
    static BarycentricCoords FromVWNoSides(double v, double w);

    bool operator==(const BarycentricCoords &other) const;
    double operator[] (int x) const;

    bool IsValid() const;
    ON_3dPoint EvaluateTriangle(const ON_Triangle& triangle) const;


    bool IsStrictlyOutsideTriangle() const;
    bool IsStrictlyOutsideTriangleOrAtBorderBecauseOfTolerance() const;
    unsigned char AtWhichSideOutsideOrAtBorderOfTriangle() const;
    bool IsAtBorderOfTriangle() const;
    signed char AtWhichEdgeOfTriangle() const;
    signed char AtWhichVertexOfTriangle() const;
    bool IsAtVertexOfTriangle() const;

    double DistanceToSquared(const BarycentricCoords& other) const;

    bool IsInsideOrAtBorderOfTriangle() const;
    bool IsInsideOrAtBorderOfTriangle(const ON_Triangle& triangle, const ON_3dPoint& pt, Tolerance& tolerance) const;
    bool IsInsideOrAtBorderOfTriangle(const ON_Triangle& triangle, const ON_3dPoint& pt, Tolerance& tolerance, BarycentricSide& out_side) const;
    BarycentricCoords ClosestTriangleEdgeCoords(const ON_Triangle& tri) const;
    BarycentricCoords ClosestTriangleEdgeCoords(const ON_Triangle& tri, const ON_3dPoint& pt, ON_3dPoint& pt_found) const;

    double ToEdgeParameter(signed char edge) const;

    static const BarycentricCoords Unset;

    static BarycentricCoords FromVertexIndex(unsigned char index);
    static BarycentricCoords OnEdgeOppositeOf(unsigned char index, double t, bool flipped);
    static BarycentricCoords CreateFromTriangle(const ON_Triangle& triangle, const ON_3dPoint& pt, double* out_signedHeight, Tolerance& tolerance);

    bool IsBarycentricSideOppositeOfVertexIndex(unsigned char vi);
    bool IsBarycentricSideNarrowlyOppositeOfVertexIndex(unsigned char vi);
  };


  class MeshVertexInfo
  {
  public:
    ON_Color m_C;
    ON_2fPoint m_T;
    ON_2dPoint m_S;
    ON_3fVector m_N;
    ON_SurfaceCurvature m_K;
  };

  enum class StartEndBoth : signed char
  {
    None = 0,
    Start = 1,
    End = 2,
    Both = Start | End,
  };
  inline StartEndBoth operator|(StartEndBoth a, StartEndBoth b) { return static_cast<StartEndBoth>(static_cast<signed char>(a) | static_cast<signed char>(b)); }

  class TriangleAndPolylinePair
  {
  public:
    GroupItemPair m_triangle;
    GroupItemPair m_polyline;
    BarycentricCoords m_segment_from;
    BarycentricCoords m_segment_to;
    StartEndBoth m_segment_side;

    static int Compare(const TriangleAndPolylinePair& a, const TriangleAndPolylinePair& b)
    {
      int rc = cmp(a.m_triangle.RoughValue(), b.m_triangle.RoughValue());

      if (rc == 0) rc = cmp(a.m_polyline.RoughValue(), b.m_polyline.RoughValue());

      return rc;
    }

    static int ComparePtr(const TriangleAndPolylinePair* a, const TriangleAndPolylinePair* b)
    {
      return Compare(*a, *b);
    }
  };

  class PolylineSegmentPair : public GroupItemPair
  {
  public:
    BarycentricCoords m_from;
    BarycentricCoords m_to;
    StartEndBoth m_side;
  };


  //even numbers are errors, odd are valid. Values over 100 are hits. Do not rely on the value itself
  enum class TriangleHitMode : unsigned char
  {
    Unset = 0,
    ErrorAssumeMiss = 2,

    Apart = 3, //nothing in common
    TippingYetMissed = 5, //have some edge point in common but do not intersect
    OnlySingleVertexTouch = 7, //have some edge point in common but do not intersect
    AsideYetMissed = 9, //coplanar that do not intersect

    Lateral1Into0_FirstStep = 21, //skew triangles that have one edge that perforates

    BOUNDARY_OF_MISSED = 100, //only use for comparison!

    Crossing1Into0 = 101, //skew triangles that have two edges that perforate

    Lateral1Into0 = 121, //skew triangles that have one edge that perforates

    AsideEdgeHitting = 123, //triangles that have two edges that overlap in 3D. Only determined if on different meshes

    AsideHitting = 131, //coplanar triangles that overlap in an area

    ExactMatch = 141, //exactly matching triangles. No tolerance allowed

    ErrorAssumeHit = 200,
  };

  class TriangleHitModeParser
  {
  public:
    static bool IsExplicitError(TriangleHitMode mode)
    {
      return (((int)mode) & 1) == 0 && (mode != TriangleHitMode::Unset); //is even
    }

    static bool IsValid(TriangleHitMode mode)
    {
      return (((int)mode) & 1); //is odd
    }

    static bool IsOverlapHit(TriangleHitMode mode)
    {
      return mode == TriangleHitMode::AsideHitting ||
        mode == TriangleHitMode::ExactMatch;
    }

    static bool IsPerforatingHit(TriangleHitMode mode)
    {
      return mode == TriangleHitMode::Lateral1Into0 ||
        mode == TriangleHitMode::Crossing1Into0;
    }

    static bool IsHit(TriangleHitMode mode)
    {
      return mode > TriangleHitMode::BOUNDARY_OF_MISSED;
    }

    static bool IsMiss(TriangleHitMode mode)
    {
      return mode < TriangleHitMode::BOUNDARY_OF_MISSED;
    }

    static bool IsPerfectSide(TriangleHitMode mode)
    {
      return mode == TriangleHitMode::TippingYetMissed || mode == TriangleHitMode::AsideYetMissed || mode == TriangleHitMode::ExactMatch;
    }

  private:
    TriangleHitModeParser() = delete;
  };

  enum class CoplanarMeshIndexItem : char
  {
    Unset,
    TriQuad,
    Ngon,
  };

  enum class GroupStates : unsigned char
  {
    Unset,
    ThereAreNoGroups,
    Mesh0IsFromFirstGroup,
    Mesh0IsFromSecondGroup,
  };

  enum class InfinitesimalDike : unsigned char
  {
    Unset,
    Intersect,
    Apart,
  };

  //Defines the event of an intersection of two triangles. 
  class TriangleHit
  {
  public:
    unsigned m_Mesh0;
    unsigned m_Mesh1;

    unsigned m_TI0;
    unsigned m_TI1;

    BarycentricCoords m_Coord_T0_From;
    BarycentricCoords m_Coord_T0_To;

    TriangleHitMode m_HitMode;
    GroupStates m_M0_from_first_group;
    InfinitesimalDike m_dike;

    CoplanarMeshIndexItem m_coplanar_mesh_result;
    unsigned m_coplanar_mesh_index;

    unsigned char m_top_naked_status;
    Orientation m_BtoA;

    unsigned m_creation_age;

    void Swap0And1References();

    bool IsValid() const;
    bool IsCoherent() const;
    void Reset();

    unsigned FirstGroupMesh() const;
    unsigned SecondGroupMesh() const;
    unsigned FirstGroupTri() const;
    unsigned SecondGroupTri() const;

    void Compute(Tolerance& tolerance, const TessellationArray* inputA, const TessellationArray* inputB, class HitsCollection* result_shared_data, InnerSettings& settings);
    ON_Line ToLine(ON_Triangle& t) const;

    static int Compare(const TriangleHit* a, const TriangleHit* b);

    static const TriangleHit Unset;
  };

  class HitsCollection
  {
  public:
    ON_Mesh* m_coplanar_intersects = nullptr;
    ON_SimpleArray<unsigned>* m_coplanar_intersects_faces_ages = nullptr;
    ON_SimpleArray<ON_2udex>* m_coplanar_intersects_mesh_origin = nullptr;
    ON_SimpleArray<TriangleHit> m_coll;
    void* m_lock = nullptr;

    //reverts scrambling made by threading
    void Sort();
  };

  enum class RayHitMode : unsigned int
  {
    InconclusiveError,
    CoplanarUndetermined,

    MissBySide,
    MissByTOnly,
    MissByTAndSide,
    MissByLogic,

    Hit,
  };


  //Defines the event of an intersection of a ray and a triangle. 
  class RayHit
  {
  public:
    static RayHit FromValues(RayHitMode mode, double t, const BarycentricCoords& coords);

    // represents the barycentric coordinates of the ray hit, where U==1 is the B point, V==1 is the C point, and both==0 is the A point.
    BarycentricCoords m_coord;

    // represents the location of the ray hit, from 0, ray start, to 1, ray end.
    double m_T;

    // represents the signed height of the hit.
    double m_signedHeight;
    RayHitMode m_Mode;

    static RayHit Inconclusive;

    bool CompromiseAllWithTolerance(const ON_Line& line, const ON_Triangle& tri, Tolerance& tolerance);

    bool CompromiseCloseSideMiss(const ON_Line& line, const ON_Triangle& tri, Tolerance& tolerance);
    bool CompromiseCloseDepthMiss(const ON_Line& line, Tolerance& tolerance);

    bool IsStartOrEndHit() const;
  };

  //allows composition of ON_ProgressReporter
  class ScalableReporter
  {
    ON_ProgressReporter* m_reporter;
    double m_from;
    double m_scale;

#if defined(ON_DEBUG)
    double m_last_value;
#endif

  public:
    /*__declspec(noinline)*/ void ReportProgress(double fractionalComplete);

    ScalableReporter(ON_ProgressReporter* reporter) : ScalableReporter(reporter, 0, 1)
    {
    }

    ScalableReporter(ON_ProgressReporter* reporter, double from, double scale)
    {
      m_reporter = reporter;
      m_from = from;
      m_scale = scale;

#if defined(ON_DEBUG)
      m_last_value = 0;
#endif
    }

    static ScalableReporter Create(ON_ProgressReporter* reporter);
    static ScalableReporter CreateScaled(ON_ProgressReporter* reporter, double fromFractional, double toFractional);
    ScalableReporter CreateScaled(double fromFractional, double toFractional) const;

    ScalableReporter CreatePartitioned(unsigned int maxExcluded) const;
    ScalableReporter CreatePartitioned(int maxExcluded) const { return CreatePartitioned((unsigned)maxExcluded); }

    ScalableReporter CreateScaledPartitioned(double fromFractional, double toFractional, unsigned int maxExcluded) const;
    ScalableReporter CreateScaledPartitioned(double fromFractional, double toFractional, int maxExcluded) const
      { return CreateScaledPartitioned(fromFractional, toFractional, (unsigned)maxExcluded); };

    ScalableReporter InsidePartitionedStep(unsigned int step) const;
    ScalableReporter InsidePartitionedStep(int step) const { return InsidePartitionedStep((unsigned)step); }

    ON_ProgressReporter* GetSystemReporter() const { return m_reporter; }

    static ScalableReporter Empty;
  };


  //Defines the search context for in-function call when triangles are searched. 
  class RTreeTriangleSearchContext
  {
  public:
    static bool WhenFoundCallbackParallel(RTreeTriangleSearchContext* context, ON__INT_PTR id0, ON__INT_PTR id1);
    static bool WhenFoundCallbackAwithBParallel(RTreeTriangleSearchContext* context, ON__INT_PTR id0, ON__INT_PTR id1);
    static bool WhenFoundCallbackInline(RTreeTriangleSearchContext* context, ON__INT_PTR id0, ON__INT_PTR id1);
    static bool WhenFoundCallbackAwithBInline(RTreeTriangleSearchContext* context, ON__INT_PTR id0, ON__INT_PTR id1);

    const TessellationArray* m_InputsA;
    const TessellationArray* m_InputsB;

    HitsCollection* m_Events;
    Tolerance* m_Tolerance;
    InnerSettings* m_Settings;
    ScalableReporter* m_Reporter;

    //ON_SimpleArray<unsigned char>* m_KeepFirst;
    unsigned m_creation_age;

    void* m_Futures;

    bool* m_anyTypeOfIntersection;
    void* m_meshmeshpairs;
    void* m_meshfacemeshfacepairs;
  };

  //Defines the search context for in-function calls when polylines segments are searched. 
  class RTreePolylinesSearchTriangleContext
  {
  public:
    static bool WhenSegmentFoundCallbackForTriangle(void* context, ON__INT_PTR id);

    const ON_RTree* m_tree;
    const ON_SimpleArray<ON_Polyline*>* m_polylines;

    bool m_found = false;
    Tolerance* m_tolerance;

    GroupItemPair m_searched_index;
    ON_Triangle m_searched_triangle;
    ON_BoundingBox m_searched_triangle_boundingbox;

    ON_SimpleArray<TriangleAndPolylinePair>* m_hits;
  };


  //Defines the search context for in-function calls when polylines segments are searched. 
  class RTreePolylinesSearchContext
  {
  public:
    static bool WhenSegmentFoundCallbackDoubleStackMoveToFirst(void* context, ON__INT_PTR id);

    const ON_RTree* m_tree;
    const ON_SimpleArray<ON_Polyline*>* m_polylines;
    const ON_SimpleArray<const ON_Line*>* m_lines;

    bool m_equal_found = false;
    GroupItemPair m_index = { UINT_MAX, UINT_MAX };
    Tolerance* m_tolerance;

    ON_Line m_searched_line_preincreased;
    ON_BoundingBox m_searched_line_boundingbox;
  };


  //Contains functions that logically belong with ON_RTree or are associated with it. This class only contains static methods.
  class RTreeExtensions
  {
  public:
    static void FindTriangleBoundingBox(const ON_3dPoint& p0, const ON_3dPoint& p1, const ON_3dPoint& p2, double out_min[3], double out_max[3]);
    static void FindTriangleBoundingBox(const ON_MeshTriangle& triangle, const ON_Mesh& mesh, double out_min[3], double out_max[3]);
    static bool AddMeshTrianglesToTree(ON_RTree* tree, unsigned meshIndex, const Tessellation& tris, ON_Terminator* cancel, const ScalableReporter reporter);
    static bool AddTriangleFace(ON_RTree* tree, const ON_3fPoint* meshFloatVs, const ON_MeshTriangle tri, GroupItemPair indices);
    static bool AddTriangleFace(ON_RTree* tree, const ON_3dPoint* meshFloatVs, const ON_MeshTriangle tri, GroupItemPair indices);
    static bool AddPolyline(ON_RTree* tree, const ON_Polyline& polyline, unsigned int polyline_index);
    static bool AddBoundingBox(ON_RTree* tree, const ON_BoundingBox& box, GroupItemPair indices);
    static bool TreeFindTriangleInPolyline(RTreePolylinesSearchTriangleContext& context);
    static void FillTreeWithMeshPoints(const TessellationArray& tessellations, ON_RTree &tree, bool insertSinglePrecision,
      const ON_SimpleArray<const ON_SimpleArray<bool>*>* vertexFlags, bool onlyNaked);
    static void FillTreeWithMeshPoints(const ON_SimpleArray<class MX_3dPointSparseEnumerator*>& tessellations, ON_RTree& tree, bool insertSinglePrecision,
      const ON_SimpleArray<const ON_SimpleArray<bool>*>* vertexFlags, bool onlyNaked);

    static ON_RTree* CreateTreeWithTriangles(const TessellationArray& inputMeshes, InnerSettings& settings, ScalableReporter reporter, SetsCombinations sets, 
      const ON_RTree* firstTree, const ON_BoundingBox* firstTreeBox);
    static ON_RTree* CreateTreeWithPolylines(const ON_SimpleArray<const ON_Polyline*>& polys);
    static ON_RTree* CreateTreeWithPoints(const ON_Mesh& vertices);

  private: RTreeExtensions() = delete; //prohibit instantiation
  };

  class ON_CLASS LinesIntersection
  {
  public:
    MX_LineIntersectionType Type;

    // intersection point
    ON_3dPoint Pt0;

    // filled in case Type == CollinearAndOverlap,
    // or if computed Pt0 is in bit-sparkle tolerance with Pt0, then this value represents the actual computed value (t0_in_end_tolerance or t1_in_end_tolerance)
    ON_3dPoint Pt1;

    double t0, t1;
    bool e0_in_end_tolerance, e1_in_end_tolerance;
    Orientation e0_flip, e1_flip;
    double dist_sq;

    static const LinesIntersection Empty;

    bool DefinesIntersection() const;
    bool DefinesIntersectionNotAtEdge() const;

    bool IsIntersectionBecauseOfSnaps() const;
    bool IsIntersectionOnEdge() const;
    void SwapOrder();
    double T0_directed() const { return e0_flip == Orientation::Negative ? 1.0 - t0 : t0; }
    double T1_directed() const { return e1_flip == Orientation::Negative ? 1.0 - t1 : t1; }
  };

  //Performs intersection operations. This class only contains static methods.
  class IntersectionOps
  {
  public:

    //given a0==[0,1,2] and a1==[0,1,2], where a0!=a1, returns the missing item from both sets. Otherwise, -1 is returned.
    static unsigned char MissingTriangleVertex(unsigned char a0, unsigned char a1);
    //given a0==[0,1,2], returns the other two missing items.
    static void OtherTriangleVertices(unsigned char a0, unsigned char& a1, unsigned char& a2);

    static void FaceVertices(unsigned char v, unsigned char& a1, unsigned char& a2);
    static void FaceEdge(const ON_Mesh& m, unsigned int fi, unsigned char ei, unsigned int& a1, unsigned int& a2);

    static unsigned char VertexThatJoinsEdges(unsigned char e0, unsigned char e1);

    static BarycentricCoords BarycentricCoordinatesOf(const ON_Triangle& tri, const ON_3dPoint& pt, Tolerance& tolerance);
    static BarycentricCoords BarycentricCoordinates(const ON_Triangle& tri, const ON_3dPoint& pt, double* out_signedHeight, Tolerance& tolerance);

    static bool IsPointProjectionInOrAtTriangle(const ON_Triangle& tri, const ON_3dPoint& pt, Tolerance& tolerance);
    static bool IsPointProjectionInOrAtTriangle(const ON_Triangle& tri, const ON_3dPoint& pt, BarycentricCoords& out_coords, Tolerance& tolerance);
    static bool IsPointInOrAtTriangle(const ON_Triangle& tri, const ON_3dPoint& pt, BarycentricCoords& out_coords, double* out_signedHeight, Tolerance& tolerance);
    static bool IsPointInOrAtTriangle(const ON_Triangle& tri, const ON_3dPoint& pt, BarycentricCoords& out_coords, double* out_signedHeight,
      Tolerance& tolerance, BarycentricSide& out_side);

    //tree2 only used for FirstWithRest
    static void OriginalGatherIntersectionEvents(
      const TessellationArray& inputMeshes, const TessellationArray* inputMeshesB, const ON_RTree* tree, const ON_RTree* tree2,
      HitsCollection* intersectionEventsToFill, bool* anyTypeOfIntersection, ON_SimpleArray<ON_2udex>* meshmeshpairs, ON_SimpleArray<ON_4udex>* meshfacepairs,
      InnerSettings& settings, ScalableReporter reporter);

    static ON_3dRay TriangleToRay(const ON_Triangle& t, const int edgeIndex);
    static ON_3dRay TriangleToRay(const ON_Triangle& t, const int edgeIndex, bool& out_swapped);
    static ON_Line TriangleToLine(const ON_Triangle& t, const int edgeIndex, bool& out_swapped);
    static ON_3dRay TriangleToRay(const ON_Triangle& t, const int vertex0, const int vertex1);
    static ON_3dRay TriangleToRay(const ON_Triangle& t, const int vertex0, const int vertex1, bool& out_swapped);
    static ON_Polyline TriangleToPolyline(const ON_Triangle& t0);

    static void OriginalTriangleTriangleIntersect(const TessellationArray* inputA,const TessellationArray* inputB, Tolerance& tolerance, TriangleHit& hitToFill, HitsCollection* resultSharedData);

    static bool OriginalTriangleTriangleIntersectBoundingBoxes(const TessellationArray* inputA, const TessellationArray* inputB, Tolerance& tolerance, TriangleHit& hitToFill);

    static bool FillTrianglePairFromTriangleHit(const TessellationArray* inputA, const TessellationArray* inputB, const TriangleHit& hit, ON_Triangle& t0, ON_Triangle& t1, unsigned char* naked);
    static bool FillTrianglePairFromTriangleHitChecked(const TessellationArray* inputA, const TessellationArray* inputB, const TriangleHit& hit, ON_Triangle& t0, ON_Triangle& t1, unsigned char* naked);

    //beware! This is similar to RH_CompareGeometry but it DOES NOT round vertices to floats. Therefore gives different results
    static int CompareMeshes(const ON_Mesh* m0, const ON_Mesh* m1);
    static int CompareMeshes(const Tessellation* m0, const Tessellation* m1);
    static int CompareMeshes(Tessellation* const* m0, Tessellation* const* m1);

    static Orientation MakeLineIncreasing(ON_Line& l);
    static Orientation MakeEdgeIncreasing(ON_2udex& edge);
    static Orientation MakeEdgeIncreasing(ON_2udex& edge, const ON_SimpleArray<int>& top_v_map);
    static void MakeLineOriginal(ON_Line& l, Orientation orient);
    static Orientation Make2udexIncreasing(ON_2udex& dex);

    //0 is already triangle; 1 is 012+023, 2 is 013+123
    static int ChooseMeshFaceIntoMeshTriangles(const ON_Mesh& mesh, const ON_MeshFace& face);
    //returns false if face was already a triangle
    static bool SplitMeshFaceIntoMeshTriangles(const ON_Mesh& mesh, const ON_MeshFace& face, ON_MeshTriangle& tr0, ON_MeshTriangle& tr1);

    //simply closed means that start and end point are the same
    static bool IsPolylineSimplyClosed(const ON_Polyline& polyline);
    static bool IsPolylineSimplyClosed(const ON_Polyline& polyline, Tolerance& tolerance, double& sq_len);

    static ON_2dPoint Project3dPointToPlane(const ON_3dPoint& point, const LazyPlane& plane);

    static double TriangleQuadrupleSquaredArea(const ON_Triangle& triangle);
    static unsigned char TriangleLongestEdge(const ON_Triangle& triangle, double& squaredLength);

    static bool MeshSplitHitsArray(const TessellationArray& inputs, ON_SimpleArray<TriangleAndPolylinePair>& hits, ON_SimpleArray<ON_Mesh*>& results,
      ON_SimpleArray<ON_SimpleArray<unsigned int> *> * splitOriginFaces, bool createNgons,
      ON_SimpleArray<ON_SimpleArray<ON_2udex> *>* boundaries, ON_SimpleArray<ON_SimpleArray<unsigned>*>* boundaries_source_ids,
      const ON_SimpleArray<ON_Polyline*>& polylines, InnerSettings& settings, ScalableReporter reporter);

    static double MinimalFaceDistanceSquared(const ON_Triangle& t0, const ON_Triangle& t1);
    static double MinimalFaceDistanceSquared(const ON_MeshFace& f0, const ON_Mesh& m0, const ON_MeshFace& f1, const ON_Mesh& m1);
    static double MinimalFaceDistanceSquared(const ON_MeshFace& f0, const ON_MeshFace& f1, const ON_Mesh& mesh);
    static double MinimalFaceDistanceSquared(const ON_MeshTriangle& t0, const ON_Mesh& m0, const ON_MeshTriangle& t1, const ON_Mesh& m1);
    static double MinimalFaceDistanceSquared(const ON_MeshTriangle& t0, const ON_MeshTriangle& t1, const ON_Mesh& mesh);

    static bool RemoveNearlyEqualSubsequentPointsInPolyline(ON_Polyline& pl, Tolerance& tolerance);

  private: IntersectionOps() = delete; //prohibit instantiation
           const static unsigned char missing_triangle_indices[];
  };
}

class ON_MeshIntersectionCache::ON_MeshIntersectionCacheImplementation
{
public:
  ON_MeshIntersectionCacheImplementation();
  ~ON_MeshIntersectionCacheImplementation();
  std::mutex* m_mutex;
  const MX::TessellationArray* m_inputB;
  const ON_RTree* m_inputBTree;
  const ON_BoundingBox* m_inputBTreeBBox;
  ON_BoundingBox GetBoundingBox(const ON_SimpleArray<const ON_Mesh*>& meshes);
};

class ON_MeshIntersectionOptions::ON_MeshIntersectionOptionsImplementation
{
public:
  ON_MeshIntersectionOptionsImplementation();
  ON_MeshIntersectionOptionsImplementation(double);
  ON_MeshIntersectionOptionsImplementation(double, ON_TextLog*, ON_Terminator*, ON_ProgressReporter*);
  ON_MeshIntersectionOptionsImplementation(ON_MeshIntersectionCache*, Accuracy, bool, double, ON_TextLog*, ON_Terminator*, ON_ProgressReporter*);
  ~ON_MeshIntersectionOptionsImplementation();

  ON_MeshIntersectionCache* m_cache = nullptr;
  Accuracy m_accuracy = Accuracy::Standard;
  bool m_multithreading = false;
  MX::Tolerance m_tolerance = MX::Tolerance::OnZeroTolerance();
  ON_TextLog* m_log = nullptr;
  ON_Terminator* m_cancel = nullptr;
  MX::ScalableReporter* m_reporter = nullptr;
  // Tiny-model scale-normalization exponent p (s = 2^p) set by _MX_MeshMeshBoolean: m_tolerance is
  // already divided by 2^p, so any ABSOLUTE length floor guarding a downstream tolerance must be too.
  int m_scale_shift = 0;
};

class ON_MeshSplitOptions::ON_MeshSplitOptionsImplementation
{
public:
  ON_MeshSplitOptionsImplementation();
  ~ON_MeshSplitOptionsImplementation();

  bool m_createNgons = true;
  bool m_splitAtCoplanars = true;
};

namespace MX
{
  class EdgeSplitEnd
  {
  public:
    signed char m_edge;
    double m_t;
    unsigned m_point;
  };

  class ON_CLASS PublicIntersectionOps
  {
  public:
    static RayHit TriangleRayIntersect(const ON_Triangle& triangle, const ON_3dRay& ray, Tolerance& tolerance);

    static bool ComputeMeshIntersectionEvents(const TessellationArray& inputs, HitsCollection* hitsToFill, 
      InnerSettings& settings, ScalableReporter reporter);
    static bool ComputeMeshIntersectionEvents(const TessellationArray& inputsA, const TessellationArray& inputsB, const ON_RTree* treeB, const ON_BoundingBox* treeBBox,
      HitsCollection* hitsToFill, InnerSettings& settings, ScalableReporter reporter);

    static RayHit TriangleRayIntersectWatertight(const ON_Triangle& triangle, const ON_3dRay& ray, bool ray_start_is_considered_on_triangle, Tolerance& tolerance);

    static bool OriginalComputeMeshIntersections(
      const TessellationArray& inputsA, const TessellationArray* inputsB, const ON_RTree* treeB, const ON_BoundingBox* treeBBox,
      ON_SimpleArray<ON_Polyline*>* intersectionResult, ON_SimpleArray<unsigned>* out_intersectionResultInvolvedCnt, ON_SimpleArray<unsigned>* out_intersectionResultInvolvedMeshes,
      ON_SimpleArray<ON_Polyline*>* overlapResult, ON_SimpleArray<unsigned>* out_overlapResultInvolvedCnt, ON_SimpleArray<unsigned>* out_overlapResultInvolvedMeshes,
      ON_Mesh* mesh, ON_SimpleArray<ON_SimpleArray<bool>*>* out_inputs_fi_are_overlaps, InnerSettings& settings, ScalableReporter reporter,
      bool removeTinyGapsForRhinoDocument);

    // Exact analog of OriginalComputeMeshIntersections, same signature. No fallback to Original: when the
    // exact engine is selected the caller observes its strengths AND weaknesses
    // (see MX::UseTentativeIntersector, which gates this and is OFF by default).
    static bool TentativeComputeMeshIntersections(
      const TessellationArray& inputsA, const TessellationArray* inputsB, const ON_RTree* treeB, const ON_BoundingBox* treeBBox,
      ON_SimpleArray<ON_Polyline*>* intersectionResult, ON_SimpleArray<unsigned>* out_intersectionResultInvolvedCnt, ON_SimpleArray<unsigned>* out_intersectionResultInvolvedMeshes,
      ON_SimpleArray<ON_Polyline*>* overlapResult, ON_SimpleArray<unsigned>* out_overlapResultInvolvedCnt, ON_SimpleArray<unsigned>* out_overlapResultInvolvedMeshes,
      ON_Mesh* mesh, ON_SimpleArray<ON_SimpleArray<bool>*>* out_inputs_fi_are_overlaps, InnerSettings& settings, ScalableReporter reporter,
      bool removeTinyGapsForRhinoDocument);

    // Engine dispatcher for the NON-boolean entries (MX_MeshMeshIntersect, MX_MeshMeshSplit_Helper).
    // Gated by MX::UseTentativeIntersector() -- NOT UseTentative() -- so the boolean A/B gate keeps
    // measuring only the booleans while the exact intersector reaches parity on these paths.
    // _MX_MeshMeshBoolean must keep calling OriginalComputeMeshIntersections directly: it has already
    // evaluated UseTentative() for its own supported class, so dispatching again would re-enter the exact
    // engine on the branch it deliberately routed to Original.
    static bool ComputeMeshIntersections(
      const TessellationArray& inputsA, const TessellationArray* inputsB, const ON_RTree* treeB, const ON_BoundingBox* treeBBox,
      ON_SimpleArray<ON_Polyline*>* intersectionResult, ON_SimpleArray<unsigned>* out_intersectionResultInvolvedCnt, ON_SimpleArray<unsigned>* out_intersectionResultInvolvedMeshes,
      ON_SimpleArray<ON_Polyline*>* overlapResult, ON_SimpleArray<unsigned>* out_overlapResultInvolvedCnt, ON_SimpleArray<unsigned>* out_overlapResultInvolvedMeshes,
      ON_Mesh* mesh, ON_SimpleArray<ON_SimpleArray<bool>*>* out_inputs_fi_are_overlaps, InnerSettings& settings, ScalableReporter reporter,
      bool removeTinyGapsForRhinoDocument);

    static bool ComputeMeshIntersectionPredicates(
      const TessellationArray& inputsA, const TessellationArray* inputsB, const ON_RTree* treeB, const ON_BoundingBox* treeBBox, 
      bool* anyTypeOfIntersection, ON_SimpleArray<ON_2udex>* out_meshpairs, ON_SimpleArray<ON_4udex>* out_meshfacepairs, InnerSettings& settings, ScalableReporter reporter);

    //if result is empty, it will be filled by the same amount of items as in inputs. If it's non-empty, it must contain the same meshes as inputs.
    static bool MeshIncorporateProjectedPolylines(const TessellationArray& inputs, const ON_SimpleArray<const ON_Polyline*>& polys,
      bool* somethingHappened, ON_SimpleArray<ON_Mesh*>& result, InnerSettings& settings, ScalableReporter reporter, bool createNgons,
      ON_SimpleArray< ON_SimpleArray < ON_2udex >*>* boundary = nullptr, ON_SimpleArray<ON_SimpleArray<unsigned>*>* boundaries_source_ids = nullptr,
      ON_SimpleArray< ON_SimpleArray< unsigned int >* >* splitOriginFaces = nullptr
      );

    static bool MeshSplitWithProjectedPolylines(const TessellationArray& inputs, const ON_SimpleArray<const ON_Polyline*>& polys,
      bool* somethingHappened, ON_SimpleArray<ON_Mesh*>& result, InnerSettings& settings, ScalableReporter reporter,
      bool createNgons, const ON_SimpleArray<const ON_Mesh*>* splitters_for_disjoint_volumetrics = nullptr,
      ON_SimpleArray< ON_SimpleArray < ON_2udex >*>* boundary = nullptr, ON_SimpleArray<ON_SimpleArray<unsigned>*>* boundaries_source_ids = nullptr, ON_SimpleArray<ON_2udex>* out_boundary_to_color = nullptr,
      ON_SimpleArray<ON_SimpleArray<bool>*>* inputs_fi_are_overlaps = nullptr, ON_SimpleArray<bool>* out_overlaps = nullptr,
      // Per-piece fraction of the piece's own area descending from an upstream overlap-flagged face
      // (-1 if undeterminable). Same indexing/lifetime as out_overlaps.
      ON_SimpleArray<double>* out_overlap_area_fraction = nullptr);

    // New mesh from the faces fi with faceValues[fi] == keepValue; copies only the vertices those faces
    // use (ascending) plus all per-vertex/face/texture/ngon arrays. nullptr on malformed input; else caller owns.
    static ON_Mesh* ExtractSubmesh(const ON_Mesh& mesh, const ON_SimpleArray<unsigned int>& faceValues, unsigned int keepValue);

    // Note: There's similar code in: rhino4\commands\cmdDuplicate.cpp but it uses RhinoMergeCurves
    // faceOrigins [in] optional: two indices per m_F, tagging values for the next args. out_faceOriginsCnt
    // / out_faceOriginsIndices [out] required iff faceOrigins given: per-polyline index count, then the indices.
    static bool MeshDupBorder(const ON_Mesh* pMesh, ON_SimpleArray<ON_Polyline*>& out_polylines, bool joinOverUnweldedVertices = true, ON_SimpleArray<ON_2udex>* faceOrigins = nullptr, ON_SimpleArray<unsigned>* out_faceOriginsCnt = nullptr, ON_SimpleArray<unsigned>* out_faceOriginsIndices = nullptr);

    static LinesIntersection LineLineIntersect(const ON_Line& a, const ON_Line& b, Tolerance& tolerance);

    // endedup: for each line, an index of the output polyline where it ended up
    // endedup: for each line, the segment in the output polyline where it ended up. Several lines can end up in the same segment of the same polyline
    static bool JoinLines(const ON_SimpleArray<const ON_Line*>& lines, double tolerance, bool split_at_intersections, 
      ON_SimpleArray<ON_Polyline*>* results, ON_SimpleArray<unsigned>* endedup = 0, ON_SimpleArray<unsigned>* endeduppart = 0, ON_SimpleArray<Orientation>* dirs = 0,
      ON_Terminator* cancel = nullptr);

    // Exact-expansion test hooks: thin forwards to the file-static helpers, exercised only by
    // rhinocore_tests (rhtest_tentative_predicates.cpp).
    static int TestHookExpansionSign(int len, const double* h);
    static double TestHookExpansionEstimate(int len, const double* h);

    //Returns -inf if nothing was found. Tolerance is not currently meant to be tolerance for geometric proximity, but for calculation accuracy
    static double MeshRayIntersect(const ON_Mesh& mesh, const ON_3dRay& ray, ON_SimpleArray<int>* face_indices, ON_SimpleArray<double>* t_params,
      bool remove_hidden_hits, double tolerance);
    static double MeshLineIntersect(const ON_Mesh& mesh, const ON_Line& line, ON_SimpleArray<int>* face_indices, ON_SimpleArray<double>* t_params,
      bool infinite, bool sort, double tolerance);
    static double MeshPolylineIntersect(const ON_Mesh& mesh, const ON_Polyline& pline, ON_SimpleArray<int>* face_indices, ON_SimpleArray<double>* t_params,
      bool sort, double tolerance);

    static bool RemoveNearlyEqualSubsequentPointsInPolyline(ON_Polyline& pl, double tolerance);

    static ON_Mesh* FlipSquashedMeshFaces(const ON_Mesh* mesh, double min_height, double min_edge_length_for_consideration);

    static bool g_use_new_mesh_intersect_hint; // init as true
    static bool g_use_new_mesh_boolean_hint; // init as false, otherwise tests fail
    // MX::UseTentative path selection when env MX_BOOLEAN_TENTATIVE is unset: true = Original (the
    // default), false = Tentative. Toggled by TestMxDebug option 19.
    static bool g_original_path_not_tentative;
    static bool g_use_dike; // init as false in V7, true in V8
    static int g_print_level; // init as 1: 0 (silent, no reporting), 1 (minimal - bad errors only), 2 (verbose)
    static bool g_debugbreak; // init as false
    static bool g_color_spread_debug; // init as false
    static bool g_ngon_debug; // init as false
    static bool g_boolean_debug; // init as false
    static bool g_dike_debug; // init as false
    static bool g_dir_debug; // init as false
    static bool g_speed_trace; // init as false
    static bool g_parallel; // init as true
    static bool g_rejoin_islands; // init as true
    static bool g_overlaps; // init as true
    static bool g_new_meshmeshfast; // init as false
    static bool g_untangle_vertices; // init as false
    static bool g_untangle_boxes; // init as false

  private: PublicIntersectionOps() = delete; //prohibit instantiation 
  };

  class ON_CLASS DebugHelp
  {
  protected:
    virtual void BakeDot(const ON_3dPoint& pt, const wchar_t* text, ON_Color color) = 0;
    virtual void BakeLine(const ON_Line& line, ON_Color color) = 0;
    virtual void BakePolyline(const ON_Polyline& pl, ON_Color color) = 0;
    virtual void BakeMesh(const ON_Mesh& mesh, ON_Color color) = 0;
    virtual void ScreenRedraw() = 0;

  public:
    static ON_String TrianglePairToText(const ON_Triangle& t0, const ON_Triangle& t1);

    //static bool AddTrianglePair(const ON_Triangle& t0, const ON_Triangle& t1);

    static bool AnnotateTriangle(const ON_Triangle& t);
    static bool AnnotatePolyline(const ON_SimpleArray<ON_3dPoint>& p);
    static bool AnnotatePolyline(const ON_SimpleArray<ON_2dPoint>& p);
    static bool AnnotateMeshIndex(const ON_Mesh& m, int number);
    static bool AnnotateMeshIndex(const ON_Mesh& m, const wchar_t* text, const ON_Color& color);
    static bool AnnotateMeshFace(const ON_Mesh& m, int fi, int color);
    static bool AnnotateMeshFace(const ON_Mesh& m, int fi, const ON_Color& color, const ON_wString* prefix = 0);
    static bool AnnotateLine(const ON_Line& line);
    static bool AnnotateVector(const ON_3dVector& vector, const ON_3dPoint& start, const ON_Color& color);
    static bool AnnotateLineIndex(const ON_Line& line, const wchar_t* text, const ON_Color& color);
    static bool AnnotateLines(const ON_SimpleArray<const ON_Line*>& lines);

    static bool AnnotateDot(const ON_3dPoint& pt);
    static bool AnnotateDot(const ON_Triangle& t, const BarycentricCoords& coords);
    static bool AnnotateDot(const ON_3dPoint& pt, const wchar_t* text);
    static bool AnnotateDot(const ON_3dPoint& pt, const wchar_t* text, ON_Color color);
    static bool AnnotateDot(const ON_3dPoint& pt, const wchar_t* text, int color);

    static bool AddPolyline(const ON_Polyline& pl, const ON_Color& color);
    static bool AddPolyline(const ON_Polyline& pl);

    static bool AddLine(const ON_Line& line, const ON_Color& color);
    static bool AddLine(const ON_Line& line);

    static bool AddMesh(const ON_Mesh& mesh, const ON_Color& color);
    static bool AddMesh(const ON_Mesh& mesh);

    static bool Redraw();

    static DebugHelp* theOneAndOnlyDebugHelper;

    virtual ~DebugHelp() {};
  };

  class ON_CLASS MX_3dPointSparseEnumerator
  {
  public:
    virtual ~MX_3dPointSparseEnumerator() {}
    virtual ON_3dPoint Get(int index) const = 0;
    virtual void Set(int index, const ON_3dPoint& pt) = 0;
    virtual int NextIndex() const = 0;
    virtual void Reset() const = 0;
    virtual int GetCount() const = 0;
    virtual int GetHighestIndex() const = 0;
    virtual MX_3dPointSparseEnumerator* Clone() const = 0;
    virtual bool IsNaked(int index) const = 0;
    virtual void EnsureInternalIntegrity() {}
    static const MX_3dPointSparseEnumerator* CreateFromObject(const ON_Geometry* geom);
    static MX_3dPointSparseEnumerator* CreateFromObject(ON_Geometry* geom, bool onlyNaked);
    virtual ON_Geometry* InternalGeometry() = 0;
    virtual const ON_Geometry* InternalConstGeometry() const = 0;
    const ON_RTree* m_tree;
  };

  class ON_CLASS MX_3dPointSparseSubD : public MX_3dPointSparseEnumerator
  {
  public:
    MX_3dPointSparseSubD(ON_SubD& subd, bool naked);
    static const MX_3dPointSparseSubD* CreateReadOnly(const ON_SubD& subd);
    virtual ~MX_3dPointSparseSubD() {}
    virtual ON_3dPoint Get(int index) const;
    virtual void Set(int index, const ON_3dPoint& pt);
    virtual int NextIndex() const;
    virtual void Reset() const;
    virtual int GetCount() const;
    virtual int GetHighestIndex() const;
    virtual MX_3dPointSparseEnumerator* Clone() const;
    virtual bool IsNaked(int index) const;
    virtual ON_Geometry* InternalGeometry();
    virtual const ON_Geometry* InternalConstGeometry() const;
    virtual void EnsureInternalIntegrity();
  protected:
    MX_3dPointSparseSubD(const ON_SubD& subd);
    ON_SubD* m_subd;
    mutable ON_SubDVertexIterator* m_iter;
  };

  class ON_CLASS MX_3dPointSparseMesh : public MX_3dPointSparseEnumerator
  {
  public:
    MX_3dPointSparseMesh(ON_Mesh& mesh, bool naked);
    MX_3dPointSparseMesh(const MX_3dPointSparseMesh& other);
    MX_3dPointSparseMesh() = default;
    static const MX_3dPointSparseMesh* CreateReadOnly(const ON_Mesh& mesh);
    static const MX_3dPointSparseMesh* CreateReadOnly(Tessellation& tess);
    virtual ~MX_3dPointSparseMesh();
    virtual ON_3dPoint Get(int index) const;
    virtual void Set(int index, const ON_3dPoint& pt);
    virtual int NextIndex() const;
    virtual void Reset() const;
    virtual int GetCount() const;
    virtual int GetHighestIndex() const;
    unsigned GetTriangleCount() const;
    const ON_Mesh* GetMesh() const;
    ON_Mesh* GetNonConstMesh();
    ON_MeshTriangle TriangleAt(unsigned ti) const;
    virtual MX_3dPointSparseEnumerator* Clone() const;
    virtual bool IsNaked(int index) const;
    virtual void EnsureInternalIntegrity();
    virtual ON_Geometry* InternalGeometry();
    virtual const ON_Geometry* InternalConstGeometry() const;
    Tessellation* m_tess;
  protected:
    MX_3dPointSparseMesh(const ON_Mesh& mesh, Tessellation* tess);
    ON_Mesh* m_mesh = nullptr;
    mutable int m_vi = -1;
    bool m_external_tess = false;
  };

  class ON_CLASS MX_3dPointSparseCloud : public MX_3dPointSparseEnumerator
  {
  public:
    MX_3dPointSparseCloud(ON_PointCloud& cloud, bool naked);
    static const MX_3dPointSparseCloud* CreateReadOnly(const ON_PointCloud& cloud);
    virtual ~MX_3dPointSparseCloud() {}
    virtual ON_3dPoint Get(int index) const;
    virtual void Set(int index, const ON_3dPoint& pt);
    virtual int NextIndex() const;
    virtual void Reset() const;
    virtual int GetCount() const;
    virtual int GetHighestIndex() const;
    virtual MX_3dPointSparseEnumerator* Clone() const;
    virtual bool IsNaked(int index) const;
    virtual ON_Geometry* InternalGeometry();
    virtual const ON_Geometry* InternalConstGeometry() const;
  private:
    MX_3dPointSparseCloud(const ON_PointCloud& cloud);
    ON_PointCloud* m_cloud;
    mutable int m_vi;
  };
}


bool MX_MeshMeshIntersect(const ON_SimpleArray<const ON_Mesh*>* meshesA, const ON_SimpleArray<const ON_Mesh*>* meshesBOrNull,
  MX::SetsCombinations sets, ON_SimpleArray<ON_Polyline*>* intersectionResults,
  ON_SimpleArray<ON_Polyline*>* overlapResults, ON_Mesh* overlapResult,
  MX::MX_Opts* options, bool removeTinyGapsForRhinoDocument = false);

bool MX_MeshMeshIntersectPredicate(const ON_SimpleArray<const ON_Mesh*>* meshesA, const ON_SimpleArray<const ON_Mesh*>* meshesBOrNull,
  bool* anyTypeOfIntersection,
  ON_SimpleArray<ON_2udex>* meshpairs, ON_SimpleArray<ON_4udex>* meshfacepairs, MX::InnerSettings& settings);


bool MX_MeshMeshSplit(
  const ON_Mesh& meshToSplit, const ON_SimpleArray<const ON_Mesh*>& meshesThatSplit, MX::InnerSettings& settings,
  bool splitAtCoplanar, bool createNgons, bool* somethingHappened, ON_SimpleArray<ON_Mesh*>& results);

ON_DECL
bool MX_MeshSplit_Array_ForTL(const ON_SimpleArray<const ON_Mesh*>& MeshesToSplit,
  const ON_SimpleArray<const ON_Mesh*>& MeshSplitters,
  double intersectionTolerance,
  double overlapTolerance,
  bool splitAtCoplanar,
  bool* something_happened,
  ON_SimpleArray<ON_Mesh*>& OutMeshes,
  const ON_SimpleArray<const ON_3dmObjectAttributes*>* InputAttributeArray,
  ON_SimpleArray<const ON_3dmObjectAttributes*>* OuputAttributeArray,
  ON_TextLog* log, ON_Terminator* cancel, ON_ProgressReporter* reporter, bool createNgons);

ON_DECL
bool MX_MeshIncorporateProjectedPolylines(const ON_SimpleArray<const ON_Mesh*>& inputs, double tolerance, const ON_SimpleArray<const ON_Polyline*>& polys,
  bool* somethingHappened, ON_SimpleArray<ON_Mesh*>& result, bool createNgons,
  ON_SimpleArray< ON_SimpleArray < ON_2udex >*>* boundary, ON_SimpleArray< ON_SimpleArray< unsigned int >* >* splitOriginFaces,
  ON_TextLog* log = 0, ON_Terminator* cancel = 0, ON_ProgressReporter* reporter = 0);

ON_DECL
bool MX_MeshSplitWithProjectedPolylines(const ON_SimpleArray<const ON_Mesh*>& inputs, double tolerance, const ON_SimpleArray<const ON_Polyline*>& polys,
  bool* somethingHappened, ON_SimpleArray<ON_Mesh*>& result,
  ON_SimpleArray< ON_SimpleArray < ON_2udex >*>* boundary, ON_SimpleArray< ON_SimpleArray < unsigned >*>* boundary_src_ids, ON_SimpleArray<ON_2udex>* out_boundary_to_color,
  const ON_SimpleArray<const ON_Mesh*>* splitters_for_disjoint_volumetrics, bool createNgons,
  ON_TextLog* log, ON_Terminator* cancel, ON_ProgressReporter* reporter
);

ON_DECL
int MX_AlignVertices(ON_SimpleArray<ON_Geometry*>& meshes, ON_SimpleArray<ON_Geometry*>* results, double distance, const ON_SimpleArray<const ON_SimpleArray<bool>*>* vertexFlags, bool onlyNaked, bool averagePoints);

bool MX_CleanUpMeshesWithTolerance(const ON_SimpleArray<const ON_Mesh*>& meshes, double tolerance, ON_SimpleArray<ON_Mesh*>* results, bool mendDoubleToSingleJump,
  bool onlyNaked, bool A, bool B, bool C);

ON_DECL
ON_2dPoint MX_ClosestTriangleEdgeCoords(const ON_Triangle& t, const ON_3dPoint& point, double* signedHeight);

ON_DECL
bool MX_LineLineIntersect(const ON_Line& a, const ON_Line& b, double tolerance,
  ON_3dPoint* out_p0, ON_3dPoint* out_p1, MX_LineIntersectionType* out_intersect_type);

enum class MX_BooleanOps : unsigned char
{
  Unset,
  Union,
  Intersection,
  Difference,
  BothSplit
};

ON_DECL
int MX_MeshMeshFast(const ON_Mesh* meshA, const ON_Mesh* meshB, ON_SimpleArray<ON_Line>& lines, double tolerance);

ON_DECL
bool MX_MeshMeshBooleanUnion(const ON_SimpleArray<const ON_Mesh*>& inMeshes,
  double intersectionTolerance, double overlapTolerance,
  bool* somethingHappened,
  ON_SimpleArray<ON_Mesh*>& outMeshes,
  const ON_SimpleArray<const ON_3dmObjectAttributes*>* inputAttributeArray = 0,
  ON_SimpleArray<const ON_3dmObjectAttributes*>* ouputAttributeArray = 0,
  ON_TextLog* log = 0, ON_Terminator* cancel = 0, ON_ProgressReporter* reporter = 0,
  // RH-94152: one index set per output mesh (parallel to outMeshes), listing the input-mesh
  // indices that contributed to it. For two-set ops the indices are flat-concatenated:
  // inMeshes0 occupies 0..n0-1, inMeshes1 occupies n0..n0+n1-1.
  ON_ClassArray<ON_SimpleArray<int>>* outInputMap = 0);

ON_DECL
bool MX_MeshMeshBooleanIntersection(const ON_SimpleArray<const ON_Mesh*>& inMeshes0,
  const ON_SimpleArray<const ON_Mesh*>& inMeshes1,
  double intersectionTolerance, double overlapTolerance,
  bool* somethingHappened,
  ON_SimpleArray<ON_Mesh*>& outMeshes,
  const ON_SimpleArray<const ON_3dmObjectAttributes*>* inputAttributeArray = 0,
  ON_SimpleArray<const ON_3dmObjectAttributes*>* ouputAttributeArray = 0,
  ON_TextLog* log = 0, ON_Terminator* cancel = 0, ON_ProgressReporter* reporter = 0,
  // RH-94152: one index set per output mesh (parallel to outMeshes), listing the input-mesh
  // indices that contributed to it. For two-set ops the indices are flat-concatenated:
  // inMeshes0 occupies 0..n0-1, inMeshes1 occupies n0..n0+n1-1.
  ON_ClassArray<ON_SimpleArray<int>>* outInputMap = 0);

ON_DECL
bool MX_MeshMeshBooleanDifference(const ON_SimpleArray<const ON_Mesh*>& inMeshes0,
  const ON_SimpleArray<const ON_Mesh*>& inMeshes1,
  double intersectionTolerance, double overlapTolerance,
  bool* somethingHappened,
  ON_SimpleArray<ON_Mesh*>& outMeshes,
  const ON_SimpleArray<const ON_3dmObjectAttributes*>* inputAttributeArray = 0,
  ON_SimpleArray<const ON_3dmObjectAttributes*>* ouputAttributeArray = 0,
  ON_TextLog* log = 0, ON_Terminator* cancel = 0, ON_ProgressReporter* reporter = 0,
  // RH-94152: one index set per output mesh (parallel to outMeshes), listing the input-mesh
  // indices that contributed to it. For two-set ops the indices are flat-concatenated:
  // inMeshes0 occupies 0..n0-1, inMeshes1 occupies n0..n0+n1-1.
  ON_ClassArray<ON_SimpleArray<int>>* outInputMap = 0);

ON_DECL
bool MX_MeshMeshBooleanSplit(const ON_SimpleArray<const ON_Mesh*>& inMeshes0,
  const ON_SimpleArray<const ON_Mesh*>& inMeshes1,
  double intersectionTolerance, double overlapTolerance,
  bool* somethingHappened,
  ON_SimpleArray<ON_Mesh*>& outMeshes,
  const ON_SimpleArray<const ON_3dmObjectAttributes*>* inputAttributeArray = 0,
  ON_SimpleArray<const ON_3dmObjectAttributes*>* ouputAttributeArray = 0,
  ON_TextLog* log = 0, ON_Terminator* cancel = 0, ON_ProgressReporter* reporter = 0,
  // RH-94152: one index set per output mesh (parallel to outMeshes), listing the input-mesh
  // indices that contributed to it. For two-set ops the indices are flat-concatenated:
  // inMeshes0 occupies 0..n0-1, inMeshes1 occupies n0..n0+n1-1.
  ON_ClassArray<ON_SimpleArray<int>>* outInputMap = 0);




// ==== Tentative exact mesh-boolean path ====

// Tentative exact mesh-boolean path: exact predicates/implicit points, exact tri-tri intersection,
// shared-refinement corefinement, classification + extraction. Consumed by _MX_MeshMeshBoolean.
namespace MX
{
  // Boolean-path selection: true = Tentative (in dev), false = Original (shipping, the DEFAULT).
  // Chosen by env MX_BOOLEAN_TENTATIVE if set ("0" = Original, else Tentative), else the TestMxDebug
  // global g_original_path_not_tentative. No fallback.
  ON_DECL bool UseTentative();

  // Opt-in for the exact intersector on the NON-boolean entries (MX_MeshMeshIntersect,
  // MX_MeshMeshSplit_Helper), via ComputeMeshIntersections. Separate from UseTentative() and OFF by
  // default: the exact curves are not yet at parity with the split's projected-polyline incorporation.
  // env MX_SPLIT_TENTATIVE (anything but "0") enables it.
  ON_DECL bool UseTentativeIntersector();

  // TentativeLPI: a line/plane intersection point, never rounded -- its 5 generating EXPLICIT points
  // (line p,q; plane a,b,c), recovered exactly on demand as homogeneous coords (nx/den, ny/den, nz/den).
  struct TentativeLPI
  {
    ON_3dPoint p, q;    // line
    ON_3dPoint a, b, c; // plane
  };

  // Output-buffer capacity contracts for TentativeLPI_ExactCoords (den <= 192, nx/ny/nz <= 1152 doubles).
  // Callers MUST size their buffers to at least these many doubles.
  const int MX_TentativeLPI_DEN_CAPACITY = 192;
  const int MX_TentativeLPI_NUM_CAPACITY = 1152;

  // Exact homogeneous coords of the LPI point; each out buffer gets a Shewchuk expansion (*_len =
  // component count). Read via PublicIntersectionOps::TestHookExpansion*. den==0 => check TentativeLPI_DenSign.
  ON_DECL void TentativeLPI_ExactCoords(const TentativeLPI& lpi,
    double* nx, int& nx_len,
    double* ny, int& ny_len,
    double* nz, int& nz_len,
    double* den, int& den_len);

  // Exact sign of den (line direction dotted with plane normal): +1/-1 transverse, 0 when the line is
  // parallel to/within the plane -- the LPI point is then undefined, don't use its nx/ny/nz.
  ON_DECL int TentativeLPI_DenSign(const TentativeLPI& lpi);

  // The core geometric predicate (which side of a triangle an intersection point lies), on TentativePoint
  // args -- an original mesh vertex (Explicit) or an implicit point (LPI = edge x face).

  enum class TentativePointType { Explicit, LPI };

  // A tagged point argument: Explicit = a verbatim coordinate; LPI = an implicit point recovered
  // exactly on demand. Fill exactly ONE payload matching `type` (use the MakeTentativePoint* helpers below).
  struct TentativePoint
  {
    TentativePointType type;
    ON_3dPoint e;   // valid iff type == Explicit
    TentativeLPI lpi;  // valid iff type == LPI
  };

  inline TentativePoint MakeExplicitPoint(const ON_3dPoint& p)
  {
    TentativePoint k; k.type = TentativePointType::Explicit; k.e = p; return k;
  }
  inline TentativePoint MakeLPIPoint(const TentativeLPI& l)
  {
    TentativePoint k; k.type = TentativePointType::LPI; k.lpi = l; return k;
  }

  // TentativeOrient3D sentinels (outside -1/0/+1): UNDEFINED = an LPI arg's denominator is 0 (point
  // does not exist); UNSUPPORTED = out-of-scope arg mix (more than two LPI args).
  const int MX_Tentative_Orient3D_UNDEFINED = -2;
  const int MX_Tentative_Orient3D_UNSUPPORTED = -3;

  // Exact orientation of (a,b,c,d): sign of the signed tet volume det[a-d; b-d; c-d]. Returns +1/0/-1
  // or a sentinel above; filter-first, but always the exact sign.
  ON_DECL int TentativeOrient3D(const TentativePoint& a, const TentativePoint& b,
    const TentativePoint& c, const TentativePoint& d);

  // Exact path alone (no filter); same contract as TentativeOrient3D. Lets tests pin the exact result.
  ON_DECL int TentativeOrient3D_Exact(const TentativePoint& a, const TentativePoint& b,
    const TentativePoint& c, const TentativePoint& d);

  // FILTER alone: +1/0/-1 only when the FP interval certifies it, else FILTER_UNCERTAIN (or
  // UNDEFINED/UNSUPPORTED). A definite sign MUST equal TentativeOrient3D_Exact.
  const int MX_Tentative_Orient3D_FILTER_UNCERTAIN = -4;
  ON_DECL int TentativeOrient3D_FilterSign(const TentativePoint& a, const TentativePoint& b,
    const TentativePoint& c, const TentativePoint& d);

  // Tri-tri sign classification: the six vertex-vs-opposite-plane signs, before any point is constructed.

  struct TentativeVertexId { int mesh; int vert; };

  // A triangle from an input mesh: explicit coords + each vertex's global id (the tentative never moves/
  // rounds an input vertex; ids let later phases recognize a shared vertex by provenance).
  struct TentativeTriangle { ON_3dPoint v[3]; TentativeVertexId id[3]; };

  // sA[i] = B.v[i]'s side of A's plane; sB[j] = A.v[j]'s side of B's plane (six TentativeOrient3D signs).
  struct TentativeTriPairClass
  {
    int sA[3];
    int sB[3];
    bool coplanar;  // all sA[i]==0 (equivalently all sB[j]==0): the two planes coincide
    bool separated; // A entirely one strict side of B's plane, or B of A's -- no intersection
  };

  ON_DECL TentativeTriPairClass Tentative_ClassifyTriPair(const TentativeTriangle& A, const TentativeTriangle& B);

  // Returns the FIRST axis (x,y,z order) whose common-line direction (dir = nA x nB) component is
  // EXACTLY nonzero, or -1 if the planes are parallel. Fixed-order (NOT largest-magnitude) => deterministic.
  // outDirSign, when given, receives the EXACT sign of dir on the returned axis (0 iff the result is -1);
  // since a span is stored by increasing coordinate on that axis, that sign is the span's orientation
  // relative to nA x nB.
  ON_DECL int Tentative_CommonLineAxis(const TentativeTriangle& A, const TentativeTriangle& B,
    int* outDirSign = nullptr);

  // Compares p, q by coordinate on `axis` along the common line (from Tentative_CommonLineAxis). Returns
  // -1/0/+1; 0 = SAME POINT (exact). UNDEFINED for a zero-den LPI arg.
  ON_DECL int Tentative_CompareOnLine(const TentativePoint& p, const TentativePoint& q, int axis);

  // Transverse segment construction. KEY INVARIANT: in every non-coplanar config, each segment
  // endpoint is a reused input vertex or an LPI(edge, plane).

  // Coplanar is tagged only; Degenerate is a defensive fallback for ill-defined input (e.g. a
  // collinear triangle), never a normal outcome.
  enum class TentativeIsectKind { Empty, Point, Segment, Coplanar, Degenerate };

  // Provenance key welded by KEY EQUALITY (never coordinate/tolerance); every kind is canonical
  // (SORTED ids), so any triangle/vertex order yields the identical key.
  enum class TentativeKeyKind { InputVertex, EdgeFace, EdgeEdge, PlanarGrid };
  struct TentativeIsectKey
  {
    TentativeKeyKind kind;
    TentativeVertexId vertex;      // valid iff kind == InputVertex
    TentativeVertexId edge[2];     // valid iff kind == EdgeFace or EdgeEdge (sorted)
    TentativeVertexId face[3];     // valid iff kind == EdgeFace, or EdgeEdge (materialization only; sorted)
    TentativeVertexId edgeB[2];    // valid iff kind == EdgeEdge: the OTHER mesh's edge (sorted); identity is {edge, edgeB} unordered
    int planarCluster;          // valid iff kind == PlanarGrid: deterministic cluster ordinal
    long long planarGx;         // valid iff kind == PlanarGrid: exact grid x (identity)
    long long planarGy;         // valid iff kind == PlanarGrid: exact grid y (identity)
    ON_3dPoint planarPt;        // valid iff kind == PlanarGrid: cached materialization (NOT identity)
  };

  // Canonical PlanarGrid key (zero-fills every non-PlanarGrid field for deterministic byte compares).
  // `pt` is the node's frame lift; MaterializeKey returns it verbatim (fails on an unset point).
  ON_DECL TentativeIsectKey MakePlanarGridKey(int cluster, long long gx, long long gy,
    const ON_3dPoint& pt);

  // Canonical InputVertex key (every non-InputVertex field zero-filled).
  ON_DECL TentativeIsectKey MakeInputVertexKey(TentativeVertexId v);

  // Materializes any key to the EXACT double coordinate the weld assigns it; same-key calls return
  // bit-identical points by construction.
  ON_DECL bool Tentative_MaterializeKeyPoint(const TentativeIsectKey& key,
    const ON_Mesh& m0, int id0, const ON_Mesh& m1, int id1, ON_3dPoint& pt);

  // Deterministic TOTAL order over keys (kind, then identity fields lexicographically): the canonical-
  // pick rule when several keys land on one node. KeyLess(a,b)==KeyLess(b,a)==false iff KeyEqual-equal.
  ON_DECL bool Tentative_KeyLess(const TentativeIsectKey& a, const TentativeIsectKey& b);

  // Weld KEY EQUALITY (never coordinate/tolerance) -- used to apply key remaps to constraint endpoints.
  ON_DECL bool Tentative_KeyEqual(const TentativeIsectKey& a, const TentativeIsectKey& b);

  // One intersection endpoint: the implicit point (Explicit or LPI, see KEY INVARIANT
  // above) plus its provenance key.
  struct TentativeIsectEndpoint
  {
    TentativePoint pt;
    TentativeIsectKey key;
  };

  // Tri-tri intersection result. Segment: [a,b] ordered along the common line; Point: `a` only;
  // Empty/Coplanar/Degenerate: neither endpoint meaningful.
  struct TentativeTriIsect
  {
    TentativeIsectKind kind;
    TentativeIsectEndpoint a, b;
    // Segment only: +1 when a->b runs ALONG nA x nB, -1 when against it, 0 for every other kind.
    // [a,b] is ordered by increasing coordinate on the common-line axis, which is a storage
    // convention, not an orientation; this carries the geometric one (the same nA x nB rule
    // OriginalTriangleTriangleIntersect records in TriangleHit::m_BtoA).
    int abAlongNormalCross = 0;
  };

  // Runs Tentative_ClassifyTriPair, then builds points only for the sign pattern: separated->Empty,
  // coplanar->Coplanar (tag), same-mesh shared simplex->Empty, else the common-line span overlap.
  ON_DECL TentativeTriIsect Tentative_IntersectTriangles(const TentativeTriangle& A, const TentativeTriangle& B);

  // Mesh-pair driver: ON_RTree bbox broad phase + exact narrow phase; deterministic conservative cull.

  // One tri-tri result plus its SOURCE face on each side: face0/face1 = the mesh0/mesh1 triangle index
  // (same per-mesh enumeration as TentativeIntersectMeshPair; a quad splits into two).
  struct TentativeTriIsectTagged
  {
    TentativeTriIsect isect;
    int face0;
    int face1;
  };

  // Collected result of intersecting every candidate triangle pair between two meshes.
  struct TentativeIsectSoup
  {
    ON_SimpleArray<TentativeTriIsectTagged> items; // Point/Segment results only (kind is never
                                           // Empty/Coplanar/Degenerate for an entry here)
    int degenerateCount = 0;              // defensive: stays 0 for well-formed inputs
    ON_SimpleArray<ON_2dex> coplanarPairs; // (tri0,tri1) per pair classified Coplanar -- tagged, not
                                           // expanded (same index space as face0/1)
    bool cancelled = false;               // narrow phase aborted: `items` is partial
  };

  // Builds one mesh's triangle list EXACTLY as TentativeIntersectMeshPair does (same enumeration, same
  // coords, same CANONICAL vertex ids: each id -> the FIRST bitwise-identical coord).
  ON_DECL void Tentative_BuildTriangles(const ON_Mesh& m, int meshId,
    const ON_SimpleArray<ON_MeshTriangle>* explicitTris, ON_SimpleArray<TentativeTriangle>& out);

  // Intersects every candidate triangle pair; meshId0/1 are STABLE provenance ids. Triangle meshes
  // only (quads split on diagonal); tris0/1 override enumeration; nearCoplanarTol>0 gates near-coplanar pairs uncut.
  ON_DECL TentativeIsectSoup TentativeIntersectMeshPair(const ON_Mesh& m0, int meshId0,
    const ON_Mesh& m1, int meshId1,
    const ON_SimpleArray<ON_MeshTriangle>* tris0 = nullptr,
    const ON_SimpleArray<ON_MeshTriangle>* tris1 = nullptr,
    double nearCoplanarTol = 0.0, ON_Terminator* cancel = nullptr);

  // One segment to embed in a face's refinement (endpoints carry provenance keys). A degenerate
  // constraint (e0.key == e1.key) represents a Point result.
  struct TentativeFaceConstraint
  {
    TentativeIsectEndpoint e0, e1;
  };

  struct TentativeFaceRefineInput
  {
    int mesh;
    int faceIndex;
    TentativeTriangle face;
    ON_SimpleArray<TentativeFaceConstraint> constraints;
  };

  // Groups each result onto its source face(s): a Segment adds a constraint to both face0/face1, a
  // Point a degenerate (e0==e1) one. Args must match those `soup` was built from; output order deterministic.
  ON_DECL void Tentative_CollectFaceConstraints(const TentativeIsectSoup& soup,
    const ON_Mesh& m0, int id0, const ON_Mesh& m1, int id1,
    ON_ClassArray<TentativeFaceRefineInput>& perFace,
    const ON_SimpleArray<ON_MeshTriangle>* explicitTris0 = nullptr,
    const ON_SimpleArray<ON_MeshTriangle>* explicitTris1 = nullptr);

  // EdgeFace/EdgeEdge key canonicalization (Tentative_CollectFaceConstraints calls it once). Re-run on
  // `perFace` after any later key mutation, else a fresh divergence -> spurious InteriorCrossing.
  ON_DECL void Tentative_CanonicalizeCoincidentEdgeFaceKeys(ON_ClassArray<TentativeFaceRefineInput>& perFace);

  // Axis (0/1/2) to DROP projecting `face` to 2D: the plane-normal component of EXACTLY largest
  // magnitude (tie-broken x<y<z), keeping the 2D projection farthest from edge-on degeneracy.
  ON_DECL int Tentative_FaceProjectionAxis(const TentativeTriangle& face);

  // Exact 2D orientation sign (CCW +, CW -, collinear 0) of p,q,r projected dropping `dropAxis`; p,q,r
  // must lie on `face`'s plane. UNDEFINED for a zero-den LPI arg.
  ON_DECL int Tentative_Orient2D_OnFace(const TentativePoint& p, const TentativePoint& q,
    const TentativePoint& r, int dropAxis);

  // Exact path alone (no filter); same contract as Tentative_Orient2D_OnFace. Lets tests pin the exact result.
  ON_DECL int Tentative_Orient2D_OnFace_Exact(const TentativePoint& p, const TentativePoint& q,
    const TentativePoint& r, int dropAxis);

  // FILTER alone: a definite +1/0/-1 only when the FP interval certifies it, else FILTER_UNCERTAIN
  // (or UNDEFINED/UNSUPPORTED). A definite sign MUST equal Tentative_Orient2D_OnFace_Exact.
  ON_DECL int Tentative_Orient2D_OnFace_FilterSign(const TentativePoint& p, const TentativePoint& q,
    const TentativePoint& r, int dropAxis);

  // Per-face constrained retriangulation: each constraint becomes an EDGE, welded by key (zero-
  // tolerance conformance). Chain-only; an interior endpoint/crossing -> InteriorCrossing.
  enum class TentativeRefineStatus
  {
    Ok,                // tris holds a conforming triangulation of the face.
    InteriorCrossing,  // a constraint endpoint is interior to the face, or two chords meet away
                       // from a shared endpoint -- a 2D arrangement is needed; tris left empty.
    Degenerate,        // defensive fallback: the face is degenerate under this projection, or an
                       // implicit-point argument was undefined mid-triangulation; tris left empty.
  };

  // One sub-triangle of a face's refinement; vertices identified by KEY (never coordinate) so the weld
  // unifies identical keys from both sides. sourceMesh = origin mesh (driver stamps it before welding).
  struct TentativeRefinedTri
  {
    TentativeIsectKey v[3];
    int sourceMesh = -1;
  };

  // ---- Coplanar-band (CopBand) shared surface. Helper implementations live at the end of
  // opennurbs_plus_meshbooleans.cpp; only members referenced outside that block are surfaced here. ----
  constexpr int COPB_SCALE_GRANULARITY = 3; // coplanar-band grid scale granularity

  struct CopBandKeyRemap
  {
    TentativeIsectKey from;
    TentativeIsectKey to;
    TentativePoint toExact;
  };

  struct CopBandCluster
  {
    ON_Plane plane;                  // canonical member plane: the largest-area member tri's
    double step = 0.0;               // grid step: max(0.5*overlapTolerance, maxAbsUV / (COP2_GRID_BOUND/4))
    ON_SimpleArray<ON_2dex> members; // (mesh, tessTri), sorted ascending (mesh, then tri)
    bool valid = false;              // plane fit + deviation gate passed
  };

  struct CopBandResult
  {
    ON_ClassArray<TentativeRefinedTri> tris;                  // kept, FINAL winding
    ON_ClassArray<TentativeFaceRefineInput> pointConstraints; // T-junction injections, per (mesh, tessTri)
    ON_SimpleArray<ON_2dex> bandedMembers;                 // (mesh, tessTri) consumed
    bool sourceUsed[2] = { false, false };
    ON_ClassArray<CopBandKeyRemap> keyRemaps;              // apply to ALL constraint endpoints
  };

  // ---- Coplanar-band 2D arrangement primitives (shared with rhtest_tentative_coplanar). ----
  // Grid coordinate bound. This used to be 2^26, forced by the int64 products in the predicates below,
  // and that made the band's RESOLUTION the binding constraint rather than the tolerance: the cluster step
  // is at least maxAbsUV/(bound/4), so a cluster 55 units across at a 1e-7 tolerance got a step of 3.3e-6
  // -- 33x COARSER than tolerance -- and half a step of off-surface error was measured in the output
  // (1.807e-06 on discourse_mesh_boolean_difference, 1.616e-06 on SliderForMeshIntersect, both ~16x tol).
  // The predicates are now exact-signed through Exact::orient2d instead of int64, so the only remaining
  // requirement is that a coordinate (and its x4 form, used by the ray test) be exactly representable as a
  // double: 2^40 leaves 11 bits of headroom under 2^53 - 2.
  static const long long COP2_GRID_BOUND = 1LL << 40;

  struct Cop2Pt { long long x, y; };
  static inline bool operator==(const Cop2Pt& a, const Cop2Pt& b) { return a.x == b.x && a.y == b.y; }
  static inline bool Cop2Less(const Cop2Pt& a, const Cop2Pt& b) { return a.x != b.x ? a.x < b.x : a.y < b.y; }
  // ON_SimpleArray::BinarySearch comparator; MUST stay consistent with Cop2Less (the sort order).
  static inline int Cop2PtCompare(const Cop2Pt* a, const Cop2Pt* b)
  {
    if (a->x != b->x) return a->x < b->x ? -1 : 1;
    if (a->y != b->y) return a->y < b->y ? -1 : 1;
    return 0;
  }
  static inline long long Cop2Cross(const Cop2Pt& a, const Cop2Pt& b, const Cop2Pt& c)
  {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }

  // EXACT SIGN of Cop2Cross for any coordinate the grid can hold -- which the int64 form above is not once
  // COP2_GRID_BOUND exceeds 2^28 (its products reach 2^82 at 2^40). Grid coordinates are integers, so
  // converting to double is LOSSLESS up to 2^53 and Exact::orient2d then decides the sign exactly, taking
  // its expansion path only when the double estimate is inside its own round-off bound. Argument order maps
  // orient2d's det = (ax-cx)(by-cy) - (ay-cy)(bx-cx) onto (b.x-a.x)(c.y-a.y) - (b.y-a.y)(c.x-a.x).
  static inline int Cop2CrossSign(const Cop2Pt& a, const Cop2Pt& b, const Cop2Pt& c)
  {
    return Exact::orient2d((double)b.x, (double)b.y, (double)c.x, (double)c.y, (double)a.x, (double)a.y);
  }

  struct Cop2Frame
  {
    ON_Plane plane;
    double step = 0.0;
    long long bound = COP2_GRID_BOUND;

    bool ToGrid(const ON_3dPoint& p, Cop2Pt& out) const
    {
      double u = 0.0, v = 0.0;
      if (!plane.ClosestPointTo(p, &u, &v) || !ON_IS_FINITE(u) || !ON_IS_FINITE(v) || !(step > 0.0))
        return false;
      const double gu = u / step, gv = v / step;
      if (fabs(gu) >= (double)bound || fabs(gv) >= (double)bound)
        return false;
      out.x = llround(gu);
      out.y = llround(gv);
      return true;
    }

    ON_3dPoint FromGrid(const Cop2Pt& g) const
    {
      return plane.PointAt((double)g.x * step, (double)g.y * step);
    }
  };

  struct Cop2Seg
  {
    Cop2Pt a, b;
    unsigned mesh;
    unsigned poly;
    int dir;
  };

  struct Cop2Tri { Cop2Pt v[3]; };

  static inline bool Cop2OnSegment(const Cop2Pt& p, const Cop2Pt& a, const Cop2Pt& b)
  {
    if (Cop2CrossSign(a, b, p) != 0) return false;
    return p.x >= (a.x < b.x ? a.x : b.x) && p.x <= (a.x < b.x ? b.x : a.x)
        && p.y >= (a.y < b.y ? a.y : b.y) && p.y <= (a.y < b.y ? b.y : a.y);
  }

  struct Cop2Graph
  {
    ON_SimpleArray<Cop2Pt> nodes;
    ON_SimpleArray<int> he_from, he_to, he_next;
    ON_SimpleArray<int> he_face;
    ON_SimpleArray<int> he_seg;
    int region_count = 0;
    int outer_region = -1;
  };

  struct MX_ExternalEdgeSplit { int mesh; int vlo; int vhi; TentativeIsectEndpoint ep; };

  struct CopBandArrangement
  {
    Cop2Frame frame;
    ON_SimpleArray<Cop2Seg> segs;
    ON_SimpleArray<Cop2Pt> nodePts;
    ON_ClassArray<TentativeIsectKey> nodeKeys;
    ON_SimpleArray<ON_3dPoint> nodeCoords;
    ON_ClassArray<TentativePoint> nodeExact;
    ON_SimpleArray<int> polyMesh;
    ON_SimpleArray<int> polySign;
    int memberCount = 0;
    struct EdgeSplit { int mesh; int vlo; int vhi; int node; bool near_miss; };
    struct ChordSplit { int item; int node; bool near_miss; };
    ON_SimpleArray<EdgeSplit> edgeSplits;
    ON_SimpleArray<ChordSplit> chordSplits;
    ON_ClassArray<CopBandKeyRemap> keyRemaps;

    // nodePts is sorted (Cop2Less) and deduped, so any bsearch match is THE index.
    int NodeIndex(const Cop2Pt& p) const
    {
      return nodePts.BinarySearch(&p, Cop2PtCompare);
    }
  };

  struct CopBandCoverage
  {
    ON_SimpleArray<int> coverPlus[2];
    ON_SimpleArray<int> coverMinus[2];
    // Per region: how many DISTINCT member faces cover it. Needed because the per-side counts cannot tell
    // "two faces overlap here" from "one face was registered on both sides", which is exactly the
    // difference between a real self-overlap and the self-mode artifact. Filled only when the members are
    // supplied to _mx_copband_coverage.
    ON_SimpleArray<int> distinctFaces;
  };

  struct CopBandCycle
  {
    int region = -1;
    long long area2 = 0;
    ON_SimpleArray<int> nodes;
    bool hasTwinPair = false;
  };

  // Coplanar-band 2D arrangement helpers (implementations at end of the .cpp).
  // outCrossNode / outCrossPair (optional, both or neither): for every node the arrangement INVENTS -- a
  // 2D crossing that no input feature registered -- the node and the two `raw` indices whose crossing it is.
  // Lets the caller place such a node at the true crossing of those two segments instead of reconstructing it
  // from the grid, which is only accurate to half a grid step. Deduplicated per grid cell, first mint wins.
  ON_DECL void Cop2BuildArrangementSegments(const ON_SimpleArray<Cop2Seg>& raw, ON_SimpleArray<Cop2Seg>& out,
    const ON_SimpleArray<Cop2Pt>* extraNodes = nullptr, bool bendNearAnchors = false,
    ON_Terminator* cancel = nullptr,
    ON_SimpleArray<Cop2Pt>* outCrossNode = nullptr, ON_SimpleArray<ON_2dex>* outCrossPair = nullptr);
  ON_DECL void Cop2BuildGraph(const ON_SimpleArray<Cop2Seg>& segs, Cop2Graph& g);
  ON_DECL bool _mx_copband_build_arrangement(
    const CopBandCluster& cluster, const TentativeIsectSoup& soup,
    const ON_Mesh& m0, const ON_Mesh& m1,
    const ON_SimpleArray<TentativeTriangle>& tris0,
    const ON_SimpleArray<TentativeTriangle>& tris1,
    int clusterOrdinal, CopBandArrangement& out,
    const ON_SimpleArray<MX_ExternalEdgeSplit>* externalSplits = nullptr, ON_Terminator* cancel = nullptr,
    // exactCrossingCoords: place the nodes the 2D arrangement INVENTS at the true crossing of the two
    // segments that made them, instead of reconstructing them from the grid cell (accurate only to half a
    // grid step). ONLY the CURVE consumer asks for this. The boolean band deliberately does not: its
    // watertightness currently rests on the coarse grid's snap-rounding, and moving those coordinates breaks
    // MeshBooleanUnion/lands_terrain_path_spline_step1 exactly as the fine grid does -- measured, 6 -> 7.
    bool exactCrossingCoords = false);
  ON_DECL void _mx_copband_coverage(const ON_SimpleArray<Cop2Seg>& segs, const Cop2Graph& g,
    const ON_SimpleArray<int>& polyMesh, const ON_SimpleArray<int>& polySign, CopBandCoverage& out,
    ON_Terminator* cancel = nullptr,
    // Cluster members indexed BY POLYGON, so out.distinctFaces can be filled: two polygons naming the same
    // face are ONE face, however many side slots they occupy.
    const ON_SimpleArray<ON_2dex>* polyMember = nullptr);
  ON_DECL void _mx_copband_collect_cycles(const Cop2Graph& g, ON_ClassArray<CopBandCycle>& out);
  ON_DECL void _mx_copband_pair_holes(const Cop2Graph& g, const ON_ClassArray<CopBandCycle>& cycles,
    ON_SimpleArray<int>& parentCell);
  ON_DECL bool _mx_copband_triangulate_cell(const Cop2Graph& g, const ON_ClassArray<CopBandCycle>& cycles,
    int cell, const ON_SimpleArray<int>& holes, ON_SimpleArray<Cop2Tri>& outTris,
    const CopBandArrangement* arr = nullptr, ON_Terminator* cancel = nullptr);

  // Exact-arithmetic expansion helpers (implementations at end of the .cpp).
  double _mx_expansion_estimate(int len, const double* h);
  double _mx_cross2_exact(double a, double b, double c, double d, int& sign_out);
  int _mx_expansion_cross(const double u[2], const double x[2], const double v[2], const double w[2], double* h16);
  int _mx_expansion_scale2(int elen, const double* e, double s1, double s0, double* h);
  double _mx_orient3d_exact(const ON_3dPoint& A, const ON_3dPoint& B, const ON_3dPoint& C, const ON_3dPoint& p, int& sign_out);

  // Coplanar-band pass entry points (implementations at end of the .cpp).
  ON_DECL bool _mx_copband_form_clusters(
    const TentativeIsectSoup& soup,
    const ON_SimpleArray<TentativeTriangle>& tris0,
    const ON_SimpleArray<TentativeTriangle>& tris1,
    double overlapTolerance, double epsData,
    ON_ClassArray<CopBandCluster>& outClusters, ON_Terminator* cancel = nullptr);
  ON_DECL bool _mx_copband_run(
    const ON_ClassArray<CopBandCluster>& clusters, const TentativeIsectSoup& soup,
    const ON_Mesh& m0, const ON_Mesh& m1,
    const ON_SimpleArray<TentativeTriangle>& tris0,
    const ON_SimpleArray<TentativeTriangle>& tris1,
    MX_BooleanOps op, CopBandResult& out,
    const ON_ClassArray<TentativeFaceRefineInput>* perFace = nullptr,
    const Tessellation* tess0 = nullptr, const Tessellation* tess1 = nullptr,
    double edgeSplitTol = 0.0, ON_Terminator* cancel = nullptr);
  bool _mx_copband_overlap_loops(
    const ON_ClassArray<CopBandCluster>& clusters, const TentativeIsectSoup& soup,
    const ON_Mesh& m0, const ON_Mesh& m1,
    const ON_SimpleArray<TentativeTriangle>& tris0,
    const ON_SimpleArray<TentativeTriangle>& tris1,
    ON_SimpleArray<ON_Polyline*>& outLoops,
    ON_SimpleArray<unsigned>& outCnts, ON_SimpleArray<unsigned>& outMeshes, ON_Terminator* cancel = nullptr,
    // exactCrossingCoords: see _mx_copband_build_arrangement. Only the SPLIT/INTERSECT dispatcher asks for
    // it. The BOOLEAN path reaches this function too -- as the fallback route when the band itself bails --
    // and there these loops feed a watertightness check that the coarse grid's snap-rounding is holding up:
    // MeshBooleanUnion/lands_terrain_path_spline_step1 fails "coplanar-band result not watertight" with exact
    // coordinates and passes without them. Measured both ways, which is why this is per-caller and not global.
    bool exactCrossingCoords = false,
    // Self mode hands the SAME triangle list in as both sides, so every member face is registered on both
    // -- which makes the cross-mode overlap test (covered by side 0 AND side 1) true for any singly covered
    // region. In self mode an overlap needs TWO DISTINCT faces, so the test becomes "covered twice".
    bool selfMode = false);

  // Retriangulates `in.face` so every constraint becomes an output edge (Ok), else reports why not
  // (InteriorCrossing/Degenerate). PRECONDITION: dropAxis == Tentative_FaceProjectionAxis(in.face).
  // `tris` is cleared first; left empty unless the return is Ok.
  ON_DECL TentativeRefineStatus Tentative_RefineFace(const TentativeFaceRefineInput& in, int dropAxis,
    ON_SimpleArray<TentativeRefinedTri>& tris, ON_Terminator* cancel = nullptr);

  // THE WELD: sub-triangles reference vertices by TentativeIsectKey; same key = same vertex, materialized
  // once (the sole rounding step), output indices by first appearance. false on a bad/degenerate key.
  ON_DECL bool TentativeBuildCorefinedMesh(const ON_ClassArray<TentativeRefinedTri>& allTris,
    const ON_Mesh& m0, int id0, const ON_Mesh& m1, int id1, ON_Mesh& out,
    ON_SimpleArray<int>* outFaceSourceMesh = nullptr, ON_Terminator* cancel = nullptr);

  // Ray-cast parity test; casts along canonical fixed directions, retrying the next on any grazing.
  // PRECONDITION: `p` not ON the surface. faceTree = optional broad-phase cull; outExhausted set when all rays ambiguous.
  ON_DECL bool TentativePointInsideMesh(const ON_3dPoint& p, const ON_Mesh& mesh,
    const ON_RTree* faceTree = nullptr, bool* outExhausted = nullptr);

  struct TentativeTriLabel
  {
    int sourceMesh;   // idA or idB (whichever the caller passed as this face's faceSrc entry)
    bool insideOther; // true iff this triangle lies inside the OTHER operand's mesh
  };

  // Per face: sourceMesh = faceSrc[i], insideOther via centroid ray-cast into the OTHER operand.
  // outUnclassifiableCount/outExhaustedFaces (optional) report faces whose centroid lay on the surface.
  ON_DECL void TentativeClassifyCorefined(const ON_Mesh& corefined, const ON_SimpleArray<int>& faceSrc,
    const ON_Mesh& meshA, int idA, const ON_Mesh& meshB, int idB,
    ON_SimpleArray<TentativeTriLabel>& labels, int* outUnclassifiableCount = nullptr,
    ON_SimpleArray<int>* outExhaustedFaces = nullptr, ON_Terminator* cancel = nullptr);

  enum class TentativeBoolOp { Union, Difference /*A minus B*/, Intersection };

  // Union keeps outside tris, Intersection inside, Difference (A minus B) A-outside plus B-inside FLIPPED;
  // `out` is rebuilt compacted. False on mis-sized/bad-sourceMesh input.
  ON_DECL bool TentativeExtractBoolean(const ON_Mesh& corefined, const ON_SimpleArray<int>& faceSrc,
    const ON_SimpleArray<TentativeTriLabel>& labels, int idA, int idB, TentativeBoolOp op, ON_Mesh& out);

  // Exact tri-tri Segment results graph-walked into OriginalComputeMeshIntersections-compatible polylines
  // (one chain/loop per component; caller owns outPolylines). outHadCoplanarPairs signals Original fallback.
  // outSoup (optional): when non-null AND coplanar pairs exist, receives a copy of the TentativeIsectSoup
  // this function builds internally -- lets the caller's coplanar band reuse it without recomputing.
  // With no coplanar pairs *outSoup is left untouched (the copy is skipped; nothing consumes it then).
  ON_DECL bool TentativeExactMeshIntersectionPolylines(const ON_Mesh& m0, int id0,
    const ON_Mesh& m1, int id1, ON_SimpleArray<ON_Polyline*>& outPolylines,
    ON_SimpleArray<unsigned>& outInvolvedCnt, ON_SimpleArray<unsigned>& outInvolvedMeshes,
    bool* outHadCoplanarPairs = nullptr,
    const ON_SimpleArray<ON_MeshTriangle>* tris0 = nullptr,
    const ON_SimpleArray<ON_MeshTriangle>* tris1 = nullptr,
    double nearCoplanarTol = 0.0,
    TentativeIsectSoup* outSoup = nullptr, ON_Terminator* cancel = nullptr,
    bool selfMode = false);

  // TEST-ONLY: forces one ORIGINAL face's retriangulation to fail during a split (definition in
  // opennurbs_plus_meshbooleans.cpp; sole consumer rhtest_mesh_split_area_accounting.cpp).
  // ON_UNSET_UINT_INDEX = off.
  ON_DECL unsigned& MX_TestHook_SplitForceFailFace();

  // TEST-ONLY: captures the polyline set MX_MeshMeshSplit_Helper hands to the projected-polyline split.
  // That set is NOT what ON_Mesh::IntersectArray returns -- IntersectArray post-processes for document
  // output, and on dissect-twig it drops 8 of the 330 points the split is actually fed -- so any
  // downstream investigation has to compare THIS. Set the pointer to an array you own and the helper
  // appends deep copies (caller deletes); nullptr = off, which is a single pointer test.
  ON_DECL ON_SimpleArray<ON_Polyline*>*& MX_TestHook_SplitFeedCapture();

  // TEST-ONLY: suppresses the exact route's sub-tolerance neck weld for ONE operation, so an A/B audit can
  // print with-and-without in a single run. The weld is unconditional in the product -- it is what makes the
  // exact route's output agree with the tolerance engine on contact below tolerance -- so this is an
  // instrument, NOT a mode: never set it from product code. false (the default) = the shipping behaviour.
  ON_DECL bool& MX_TestHook_DisableNeckWeld();

  // One mesh against ITSELF. Same broad/narrow phase as TentativeIntersectMeshPair, but visits only
  // j > i and skips any triangle pair sharing a canonical vertex: adjacent triangles always meet at
  // their shared edge/corner, and counting that would make every mesh self-intersecting everywhere.
  // face0/face1 index the SAME triangle list.
  ON_DECL TentativeIsectSoup TentativeIntersectMeshSelf(const ON_Mesh& m, int meshId,
    const ON_SimpleArray<ON_MeshTriangle>* explicitTris = nullptr,
    double nearCoplanarTol = 0.0, ON_Terminator* cancel = nullptr);
}

#endif
