//
// Copyright (c) 1993-2026 Robert McNeel & Associates. All rights reserved.
// Rhinoceros is a registered trademark of Robert McNeel & Associates.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// ALL IMPLIED WARRANTIES OF FITNESS FOR ANY PARTICULAR PURPOSE AND OF
// MERCHANTABILITY ARE HEREBY DISCLAIMED.
//
// For complete Rhino SDK copyright information see <http://www.rhino3d.com/developer>.
//
////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <unordered_set>
#include <memory>
#include <tuple>
#include <utility>
#include <algorithm>

// The OverlapInternalResults / OverlapProxyIndex / RTreeOverlap classes in this header
// are the core geometry engine behind the Overlap command. They are NOT part of the
// public Rhino C++ SDK - they are scaffolding extracted from commands/cmdMatthew.cpp so
// that the command logic and the geometry computation can evolve independently.
//
// The public C++ SDK surface for overlap detection is the CRhinoOverlapFate class and
// the RhinoGetOverlaps free function, declared at the bottom of this header.

// Granularity at which the overlap engine removes covered geometry.
enum class RhinoOverlapMode : int {
  // An overlapped curve is removed only as a single unit.
  WholeCurves = 0,
  // Curves may be split at their segment boundaries so that only the
  // overlapping segments are affected. Segment coverage is decided on the
  // polyline proxies, so interval ends inherit the approximation's noise.
  Segments = 1,
  // Arbitrary parameter chunks may be removed. Candidate pairs are found with
  // bounding boxes, but the removal intervals themselves come from the exact
  // curve-curve intersector run on projected copies of the curves, so the
  // chunk ends are not subject to polyline-approximation noise. Transverse
  // crossings come back from the intersector as point events and are ignored,
  // so crossing curves are never nicked apart.
  Partial = 2
};

class OverlapInternalResults {
private:
  size_t m_n_curves;
  // A given curve can be dead; in which case it won't be included in the output
  // set at all.
  std::vector<bool> m_dead_curves;
  // Or it can be entirely required - it must be included in the output set, as it
  // has been used to cover up another curve.
  std::vector<bool> m_required_curves;
  typedef struct {
    // Similarly, segments can be required or dead.
    std::vector<bool> m_segment_is_required;
    std::vector<bool> m_segment_is_dead;
    // A curve may also accumulate a bunch of intervals covering it. These
    // are stored as a structure-of-arrays containing the interval and segment data,
    // as well as which other curve participated in this.
    std::vector<int> m_coverage_segment;
    std::vector<uint64_t> m_coverage_twin;
    std::vector<double> m_coverage_start;
    std::vector<double> m_coverage_end;
  } OverlapResult;
  std::vector<OverlapResult*> m_free_results;
  std::vector<OverlapResult*> m_results;

  void free_all(){
    for(OverlapResult* p : m_free_results) delete p;
    for(OverlapResult* p : m_results) delete p;
    m_free_results.clear();
    m_results.clear();
  }

  static std::vector<OverlapResult*> deep_copy_results(const std::vector<OverlapResult*>& src){
    size_t n = src.size();
    std::vector<OverlapResult*> ret(n, nullptr);
    for(size_t i = 0; i < n; i++){
      const OverlapResult* ptr = src[i];
      if(ptr)
        ret[i] = new OverlapResult(*ptr);
    }
    return ret;
  }

  void release_curve_results(int curve_index){
    OverlapResult* results = nullptr;
    std::swap(m_results[curve_index], results);
    if(results){
      // Wipe the vectors out...
      results->m_segment_is_required.clear();
      results->m_segment_is_dead.clear();

      results->m_coverage_segment.clear();
      results->m_coverage_end.clear();
      results->m_coverage_start.clear();
      results->m_coverage_twin.clear();
      //...and then stick it back in the pool
      m_free_results.push_back(results);
    }
  };

  OverlapResult* allocate_curve_result(int curve_index){
    OverlapResult* ret = m_results[curve_index];
    if(ret)
      return ret;
    size_t nfree = m_free_results.size();
    if(nfree){
      ret = m_free_results[nfree - 1];
      m_free_results.pop_back();
    }else{
      ret = new OverlapResult();
      if(!ret)
        return ret;
    }
    m_results[curve_index] = ret;
    ret->m_segment_is_required.clear();
    ret->m_segment_is_dead.clear();
    ret->m_coverage_segment.clear();
    ret->m_coverage_end.clear();
    ret->m_coverage_start.clear();
    ret->m_coverage_twin.clear();
    return ret;
  };

public:
  OverlapInternalResults(size_t n) : m_dead_curves(n, false), m_required_curves(n, false), m_results(n, nullptr) {
    m_n_curves = n;
  };
  ~OverlapInternalResults(){
    free_all();
  };
  OverlapInternalResults(const OverlapInternalResults& other) : m_dead_curves(other.m_dead_curves),
  m_required_curves(other.m_required_curves),
  m_free_results(0),// No need to copy the memory pool over...
  m_results(deep_copy_results(other.m_results)){
    m_n_curves = other.m_n_curves;
  };

  OverlapInternalResults& operator=(OverlapInternalResults other) {
    swap(other.m_dead_curves, m_dead_curves);
    swap(other.m_required_curves, m_required_curves);
    swap(other.m_free_results, m_free_results);
    swap(other.m_results, m_results);
    std::swap(other.m_n_curves, m_n_curves);
    return *this;
  };

  OverlapInternalResults(OverlapInternalResults&& other) noexcept
  : m_dead_curves(std::move(other.m_dead_curves)),
  m_required_curves(std::move(other.m_required_curves)),
  m_free_results(std::move(other.m_free_results)),
  m_results(std::move(other.m_results)),
  m_n_curves(other.m_n_curves)
  {
    other.m_free_results.clear();
    other.m_results.clear();
  };

  OverlapInternalResults& operator=(OverlapInternalResults&& other) noexcept {
    if (this != &other) {
      free_all();
      m_dead_curves     = std::move(other.m_dead_curves);
      m_required_curves = std::move(other.m_required_curves);
      m_free_results    = std::move(other.m_free_results);
      m_results         = std::move(other.m_results);
      m_n_curves = other.m_n_curves;
      other.m_free_results.clear();
      other.m_results.clear();
    }
    return *this;
  };
  // Now that we've gotten all of that silly c++ism out of the way, some actual code.

  // Segments and curves may be either dead or required, both of which are boolean properties that
  // may be get and set, at both the curve level and the segment level (including using a 64-bit id).
  // Locked curves are never dead, and always required, and may not be modified.
  typedef int CurveID;
  typedef int SegmentID;
  typedef uint64_t CombinedID;

  void mark_dead(CurveID);
  void mark_dead(CurveID,SegmentID);
  void mark_dead(CombinedID);

  void mark_required(CurveID);
  void mark_required(CurveID,SegmentID);
  void mark_required(CombinedID);

  bool check_dead(CurveID) const;
  bool check_dead(CurveID,SegmentID) const;
  bool check_dead(CombinedID) const;

  bool check_required(CurveID) const;
  bool check_required(CurveID,SegmentID) const;
  bool check_required(CombinedID) const;

  void prefinalize(int target, int nsegments, bool whole_curves){
    OverlapResult* results = m_results[target];
    if(!results)
      return;

    std::unordered_set<int> used_curves;
    std::unordered_set<uint64_t> used_segments;
    int dead_vector_size = (int) results->m_segment_is_dead.size();
    bool all_covered = true;
    bool none_covered = true;

    // TODO: remove duplication from finalize - big difference is that this doesn't
    // check for required segments.
    size_t all_interval_count = results->m_coverage_segment.size();
    std::vector<int> interval_order;
    interval_order.reserve(all_interval_count);
    for(size_t i = 0; i < all_interval_count; i++)
      interval_order.push_back((int) i);
    std::sort(interval_order.begin(), interval_order.end(), [&results](int a, int b){
      return results->m_coverage_segment[a] < results->m_coverage_segment[b];
    });
    std::vector<int> active_intervals;
    // Walk through each segment and try to cover it
    // Then walk through each segment and see if we can cover each of them.
    for(int i = 0; i < nsegments; i++){
      if(i < dead_vector_size && results->m_segment_is_dead[i]){
        none_covered = false;
        continue;
      }
      // If we don't have any intervals, we can't cover this segment with them...
      if(!all_interval_count){
        all_covered = false;
        if(whole_curves)
          break;
        else
          continue;
      }
      // Now we actually have to consider intervals. Doing this properly is actually an NP-complete problem,
      // so we can't do it properly - it's the "set cover problem". First, figure out which set of intervals
      // cover this segment.
      auto search = std::lower_bound(interval_order.begin(), interval_order.end(), i,[&](int a, int b){
        return results->m_coverage_segment[a] < b;
      });
      if(search == interval_order.end() || results->m_coverage_segment[*search] != i){
        all_covered = false;
        if(whole_curves)
          break;
        else
          continue;
      }
      active_intervals.clear();
      double length = 0;
      while(search != interval_order.end() && results->m_coverage_segment[*search] == i){
        int idx = *search;
        if(!check_dead(results->m_coverage_twin[idx])){
          active_intervals.push_back(idx);
          length += results->m_coverage_end[idx] - results->m_coverage_start[idx];
        }
        search++;
      }
      if(active_intervals.size() == 0 || length < 1 - ON_EPSILON){
        all_covered = false;
        if(whole_curves)
          break;
        else
          continue;
      }
      // We now have a non-empty list of active segments that might cover this segment - they're at the very least long enough!
      // First, try to cover it with only already-required or soon-to-be-required intervals.
      std::vector<std::pair<double, int>> transitions;
      std::vector<int> additional_intervals;
      for(auto idx : active_intervals){
        // Figure out if this interval is required, either in the result set or the set we're newly adding
        int twin_curve, twin_interval;
        unpack_segment_id(results->m_coverage_twin[idx], twin_curve, twin_interval);
        if(check_required(twin_curve, twin_interval) ||
           used_curves.count(twin_curve) || (!whole_curves && used_segments.count(results->m_coverage_twin[idx]))){
          transitions.emplace_back(results->m_coverage_start[idx] - ON_EPSILON, -1);
          transitions.emplace_back(results->m_coverage_end[idx] + ON_EPSILON, 1);
        }else{
          additional_intervals.push_back(idx);
        }
      }
      if(check_coverage(transitions, true)){
        // We've managed to cover this segment without adding anything new! If we're in segment mode,
        // commit the result. Otherwise, don't do anything - but don't fail either.
        if(!whole_curves){
          mark_dead(target, i);
        }
        none_covered = false;
      }else{
        for(auto idx : additional_intervals){
          transitions.emplace_back(results->m_coverage_start[idx] - ON_EPSILON, -1);
          transitions.emplace_back(results->m_coverage_end[idx] + ON_EPSILON, 1);
        }
        if(check_coverage(transitions,false)){
          // We now know that, if we use everything at our disposal, we can cover this segment.
          if(whole_curves){
            for(int idx : additional_intervals){
              int twin_curve, _;
              unpack_segment_id(results->m_coverage_twin[idx], twin_curve, _);
              used_curves.insert(twin_curve);
            }
          }else{
            mark_dead(target, i);
            for(int idx : additional_intervals){
              used_segments.insert(results->m_coverage_twin[idx]);
            }
          }
          none_covered = false;
        }else{
          // We can't cover this segment, regardless of what we've done.
          all_covered = false;
          if(whole_curves)
            break;
        }
      }
    }
    ;
    if(all_covered){
      mark_dead(target);
      release_curve_results(target);
    }
  };

  void finalize(bool& entirely_dead, ON_SimpleArray<ON_COMPONENT_INDEX>& dead_segments, ON_COMPONENT_INDEX::TYPE subtype, int target, int nsegments, bool whole_curves){
    dead_segments.SetCount(0);
    // Look for some definitive cases - all dead, all required, no intersections, etc...
    bool definitive = false;

    if(check_dead(target)){
      entirely_dead = true;
      definitive = true;
    }
    if(!definitive && check_required(target)){
      entirely_dead = false;
      definitive = true;
    }
    OverlapResult* results = m_results[target];
    if(!definitive && !results){
      entirely_dead = false;
      definitive = true;
    }
    if(definitive){
      release_curve_results(target);
      return;
    }
    // We, unfortunately, need to think more about the set of intervals we've accumulated.
    std::unordered_set<int> used_curves;
    std::unordered_set<uint64_t> used_segments;
    bool all_covered = true;
    bool none_covered = true;
    bool any_required = false;

    size_t dead_vector_size = results->m_segment_is_dead.size();
    size_t required_vector_size = results->m_segment_is_required.size();

    // We now need to sort this curve's intervals by segment
    size_t all_interval_count = results->m_coverage_segment.size();
    std::vector<int> interval_order;
    interval_order.reserve(all_interval_count);
    for(size_t i = 0; i < all_interval_count; i++)
      interval_order.push_back((int) i);
    std::sort(interval_order.begin(), interval_order.end(), [&results](int a, int b){
      return results->m_coverage_segment[a] < results->m_coverage_segment[b];
    });
    std::vector<int> active_intervals;
    // Then walk through each segment and see if we can cover each of them.
    for(int i = 0; i < nsegments; i++){
      if(i < dead_vector_size && results->m_segment_is_dead[i]){
        none_covered = false;
        continue;
      }
      if(i < required_vector_size && results->m_segment_is_required[i]){
        any_required = true;
        all_covered = false;
        if(whole_curves)
          break;
        continue;
      }
      // If we don't have any intervals, we can't cover this segment with them...
      if(!all_interval_count){
        all_covered = false;
        if(whole_curves)
          break;
        else
          continue;
      }
      // Now we actually have to consider intervals. Doing this properly is actually an NP-complete problem,
      // so we can't do it properly - it's the "set cover problem". First, figure out which set of intervals
      // cover this segment.
      auto search = std::lower_bound(interval_order.begin(), interval_order.end(), i,[&](int a, int b){
        return results->m_coverage_segment[a] < b;
      });
      if(search == interval_order.end() || results->m_coverage_segment[*search] != i){
        all_covered = false;
        if(whole_curves)
          break;
        else
          continue;
      }
      active_intervals.clear();
      double length = 0;
      while(search != interval_order.end() && results->m_coverage_segment[*search] == i){
        int idx = *search;
        if(!check_dead(results->m_coverage_twin[idx])){
          active_intervals.push_back(idx);
          length += results->m_coverage_end[idx] - results->m_coverage_start[idx];
        }
        search++;
      }
      if(active_intervals.size() == 0 || length < 1 - ON_EPSILON){
        all_covered = false;
        if(whole_curves)
          break;
        else
          continue;
      }
      // We now have a non-empty list of active segments that might cover this segment - they're at the very least long enough!
      // First, try to cover it with only already-required or soon-to-be-required intervals.
      std::vector<std::pair<double, int>> transitions;
      std::vector<int> additional_intervals;
      for(auto idx : active_intervals){
        // Figure out if this interval is required, either in the result set or the set we're newly adding
        int twin_curve, twin_interval;
        unpack_segment_id(results->m_coverage_twin[idx], twin_curve, twin_interval);
        if(check_required(twin_curve, twin_interval) ||
           used_curves.count(twin_curve) || (!whole_curves && used_segments.count(results->m_coverage_twin[idx]))){
          transitions.emplace_back(results->m_coverage_start[idx] - ON_EPSILON, -1);
          transitions.emplace_back(results->m_coverage_end[idx] + ON_EPSILON, 1);
        }else{
          additional_intervals.push_back(idx);
        }
      }
      if(check_coverage(transitions, true)){
        // We've managed to cover this segment without adding anything new! If we're in segment mode,
        // commit the result. Otherwise, don't do anything - but don't fail either.
        if(!whole_curves){
          mark_dead(target, i);
        }
        none_covered = false;
      }else{
        for(auto idx : additional_intervals){
          transitions.emplace_back(results->m_coverage_start[idx] - ON_EPSILON, -1);
          transitions.emplace_back(results->m_coverage_end[idx] + ON_EPSILON, 1);
        }
        if(check_coverage(transitions,false)){
          // We now know that, if we use everything at our disposal, we can cover this segment.
          if(whole_curves){
            for(int idx : additional_intervals){
              int twin_curve, _;
              unpack_segment_id(results->m_coverage_twin[idx], twin_curve, _);
              used_curves.insert(twin_curve);
            }
          }else{
            mark_dead(target, i);
            for(int idx : additional_intervals){
              used_segments.insert(results->m_coverage_twin[idx]);
            }
          }
          none_covered = false;
        }else{
          // We can't cover this segment, regardless of what we've done.
          all_covered = false;
          if(whole_curves)
            break;
        }
      }
    }
    if(whole_curves){
      if(any_required){
        mark_required(target);
      }else if(all_covered){
        // Mark everything we used for this covering as required, at the curve level
        for(int crv : used_curves)
          mark_required(crv);
        // Mark this curve as dead...
        mark_dead(target);
        entirely_dead = true;
      }
      // Either way, we can free up all of this curve's segment-by-segment data - it's all in the
      // required/dead curve bitvectors.
      release_curve_results(target);
    }else{
      // Otherwise, we're using partial segment data!
      if(all_covered){
        // Mark everything we used for this covering as required, at the segment level.
        for(uint64_t seg : used_segments)
          mark_required(seg);
        // Mark this curve as dead...
        mark_dead(target);
        entirely_dead = true;
        release_curve_results(target);
      }else if(none_covered){
        mark_required(target);
        release_curve_results(target);
      }else{
        // Transfer all of the coverage data to the output vector - we need to keep the results around.
        size_t covered_vector = results->m_segment_is_dead.size();
        for(int i = 0; i < covered_vector; i++){
          if(results->m_segment_is_dead[i])
            dead_segments.Append(ON_COMPONENT_INDEX(subtype, i));
        }
      }
    }
  };

  static bool check_coverage(std::vector<std::pair<double,int>>& transitions, bool add_ends){
    if(add_ends){
      transitions.emplace_back(0.0,0);
      transitions.emplace_back(1.0,0);
    }
    size_t count = transitions.size();
    if(count == 2)
      return false;
    std::sort(transitions.begin(), transitions.end());
    bool in_interval = false;
    int active_intervals = 0;

    for(auto& change : transitions){
      if(change.second == 0){
        if(in_interval)
          break;
        in_interval = true;
      }else{
        active_intervals -= change.second;
        if(in_interval && active_intervals <= 0)
          return false;
      }
    }
    return true;
  }

  void record_overlap(int,int,const double*,int,int,const double*);

  static uint64_t pack_segment_id(int curve, int segment){
    return (((uint64_t) segment) << 32) | curve;
  };
  static void unpack_segment_id(uint64_t id, int& curve, int& segment){
    curve = (int) ((id << 32) >> 32);
    segment = (int) (id >> 32);
  };
};

class OverlapProxyIndex {
public:
  typedef void* SegmentIndex;
private:
  double m_tolerance;
  const ON_SimpleArray<const ON_Object*>& m_objects;
  const ON_Xform& m_projection;

  typedef std::vector<std::tuple<int,int,ON_PolylineCurve>> ProxyMesh;
  std::vector<std::unique_ptr<ProxyMesh>> m_proxies;

  std::vector<bool> m_initialized;

  void initialize_proxy(int);

  static SegmentIndex pack_segment(uint64_t object,uint64_t sub){
    return (void*) ((object << 32) + sub);
  };

public:
  std::vector<std::unique_ptr<ON_PolyCurve>> m_flattened;

  OverlapProxyIndex(const ON_SimpleArray<const ON_Object*>& obj, const ON_Xform& proj, double tol) :
  m_objects(obj), m_tolerance(tol), m_projection(proj){
    int nobj = obj.Count();
    m_proxies.reserve(nobj);
    m_flattened.reserve(nobj);
    for(int i = 0; i < nobj; i++){
      m_proxies.emplace_back(nullptr);
      m_flattened.emplace_back(nullptr);
    }
    m_initialized.resize(nobj, false);
  };
  // Return the number of subobjects in the object at this index
  int subobject_count(int) const;
  ON_COMPONENT_INDEX::TYPE subobject_type(int) const;
  // Gets the actual geometry - proxied or not - associated with an index.
  bool get_projected_segment(SegmentIndex, ON_Line& line_out, int& object_out, int& segment_out, double*) const;
  template <typename Callback> void iterate_object(int,Callback);
  ON_BoundingBox get_bounding_box(int) const;

  void free_proxy(int, bool);
};

template <typename Callback> void OverlapProxyIndex::iterate_object(int object_id,Callback f){
  const ON_Object* obj = m_objects[object_id];
  const ON_Point* pt = ON_Point::Cast(obj);
  ON_Line line_out(ON_3dPoint::UnsetPoint,ON_3dPoint::UnsetPoint);
  double full_domain[2] = {0,1};

  if(pt){
    line_out.from = m_projection * pt->point;
    f(0,pack_segment(object_id,0),true,line_out, full_domain);
    return;
  }
  const ON_PointCloud* pts = ON_PointCloud::Cast(obj);
  if(pts){
    int npoints = pts->PointCount();
    for(int i = 0; i < npoints; i++){
      if(pts->PointIsHidden(i))
        continue;
      line_out.from = m_projection * pts->m_P[i];
      f(i,pack_segment(object_id,i),npoints == 1,line_out, full_domain);
    }
    return;
  }
  // Everything else from now on is a curve, fail if not.
  const ON_Curve* crv = ON_Curve::Cast(obj);
  if(!crv)
    return;
  // Line and polylines are simple!
  const ON_LineCurve* line = ON_LineCurve::Cast(obj);
  if(line){
    line_out = line->m_line;
    line_out.Transform(m_projection);
    f(0,pack_segment(object_id,0),true,line_out, full_domain);
    return;
  }
  const ON_PolylineCurve* polyline = ON_PolylineCurve::Cast(obj);
  if(polyline){
    const ON_Polyline& ply = polyline->m_pline;
    int segment_count = ply.SegmentCount();
    ON_3dPoint start = ON_3dPoint::UnsetPoint;
    for(int segment = 0; segment < segment_count; segment++){
      if(start.IsUnset())
        start = m_projection * ply[segment];
      ON_3dPoint end = m_projection * ply[segment + 1];
      f(segment,pack_segment(object_id,segment),segment_count == 1,ON_Line(start, end), full_domain);
      start = end;
    }
    return;
  }
  // Otherwise, we need to consult the meshed geometry...
  initialize_proxy(object_id);
  if(!m_proxies[object_id])
    return;
  const ProxyMesh& proxy = *m_proxies[object_id];
  const ON_PolyCurve* polycurve_ptr = ON_PolyCurve::Cast(obj);

  if(polycurve_ptr){
    const ON_PolyCurve& polycurve = m_flattened[object_id] ? *m_flattened[object_id] : *polycurve_ptr;
    int nsegments = polycurve.Count();
    int j = 0;
    int proxy_size = (int) proxy.size();
    int next_proxy = j < proxy_size ? std::get<0>(proxy[j]) : -1;
    int offset = 0;
    for(int i = 0; i < nsegments; i++){
      if(i == next_proxy){
        const ON_Polyline& ply = std::get<2>(proxy[j]).m_pline;
        int segment_count = ply.SegmentCount();
        ON_3dPoint start = ON_3dPoint::UnsetPoint;
        for(int segment = 0; segment < segment_count; segment++){
          if(start.IsUnset())
            start = m_projection * ply[segment];
          ON_3dPoint end = m_projection * ply[segment + 1];
          double domain[2] = {((double) segment) / segment_count, ((double) 1 + segment) / segment_count};
          f(i,pack_segment(object_id,offset++),false,ON_Line(start, end), domain);
          start = end;
        }
        j++;
        next_proxy = j < proxy_size ? std::get<0>(proxy[j]) : -1;
      }else{
        const ON_Curve* seg = polycurve.SegmentCurve(i);
        line_out.from = m_projection * seg->PointAtStart();
        line_out.to = m_projection * seg->PointAtEnd();
        f(i,pack_segment(object_id,offset++),false,line_out, full_domain);
      }
    }
  }else{
    const ON_Polyline& ply = std::get<2>(proxy[0]).m_pline;
    int segment_count = ply.SegmentCount();
    ON_3dPoint start = ON_3dPoint::UnsetPoint;
    for(int segment = 0; segment < segment_count; segment++){
      if(start.IsUnset())
        start = m_projection * ply[segment];
      ON_3dPoint end = m_projection * ply[segment + 1];
      double domain[2] = {((double) segment) / segment_count, ((double) 1 + segment) / segment_count};
      f(0,pack_segment(object_id,segment),segment_count == 1,ON_Line(start, end), domain);
      start = end;
    }
    return;
  }
}

class RTreeOverlap {
private:
  const ON_SimpleArray<const ON_Object*>& m_objects;
  const ON_SimpleArray<const ON_Object*>* m_locked_objects;

  OverlapProxyIndex m_proxy;
  OverlapProxyIndex m_locked_proxy;

  bool m_exclusive_locked;
  const ON_Xform& m_projection;
  RhinoOverlapMode m_mode;
  bool m_segments;
  double m_tolerance;

  ON_Terminator* m_terminator;

  ON_RTree m_rough_index; // An rtree containing every curve's projected bounding box
  std::vector<int> m_overlapping_box_count;
  std::vector<int> m_processing_order;
  std::vector<int> m_to_be_finalized;;

  // An rtree containing every currently active curve segment, and possibly some
  // inactive ones.
  ON_RTree m_fine_index;
  // and a counter containing the number of currently active curves - when this is 0,
  // we can throw out the rtree.
  int m_active_fine_curves;

  ON_RTree m_locked_index;

  OverlapInternalResults m_results;

  void initialize_rough_index();
  static bool rough_search_callback(void* voidctx, ON__INT_PTR a, ON__INT_PTR b){
    RTreeOverlap* ctx = (RTreeOverlap*) voidctx;
    ctx->m_overlapping_box_count[a]++;
    ctx->m_overlapping_box_count[b]++;
    return true;
  };

  bool initialize_locked_index();

  void finalize(bool& entirely_dead, ON_SimpleArray<ON_COMPONENT_INDEX>& dead_segments, int target){
    dead_segments.SetCount(0);
    entirely_dead = false;
    m_active_fine_curves--;
    m_results.finalize(entirely_dead, dead_segments, m_proxy.subobject_type(target), target, m_proxy.subobject_count(target), !m_segments);
  };

  // Triggers finalization, upon adding curves to the fine index.
  static bool finalize_callback(void* voidctx, ON__INT_PTR a){
    RTreeOverlap* ctx = (RTreeOverlap*) voidctx;
    int overlap = --(ctx->m_overlapping_box_count[a]);
    if(overlap == 0)
      ctx->m_to_be_finalized.push_back((int) a);
    return true;
  };
  static bool set_true(void* voidctx, ON__INT_PTR /*element_id*/){
    *((bool*) voidctx) = true;
    return false; // Don't search anymore
  };

  void fine_index_crv(int target);


  // Which top level object are we currently searching against the index?
  int m_active_object;
  // ...and which subobject is it?
  int m_active_subobject;
  // Is it the only subobject this object has?
  bool m_active_only_subobject;
  double m_active_domain[2];
  ON_3dPoint m_active_segment_point;
  ON_Line m_active_line;

  ON_3dVector  m_active_segment_direction, m_active_segment_normal;
  double m_active_segment_length;
  bool m_active_is_dead;
  bool m_active_locked_search;

  bool segment_callback(OverlapProxyIndex::SegmentIndex);

  static bool segment_callback(void* voidctx, ON__INT_PTR a){
    return ((RTreeOverlap*) voidctx)->segment_callback((OverlapProxyIndex::SegmentIndex) a);
  };

  // ---- Partial (chunk) mode ----
  // Partial mode is not streamed: the whole computation runs on the first call
  // to get_overlap_result and the results are queued. Curves only lose
  // material to earlier inputs (and to locked geometry), so of two mutually
  // overlapping curves the earlier one keeps the shared region; overlap
  // intervals contributed by an earlier curve are clipped against its own
  // surviving spans first, so a chunk that has already been removed covers
  // nothing.
  struct ChunkResult {
    int m_object_index = -1;
    bool m_entire = false;
    ON_SimpleArray<ON_COMPONENT_INDEX> m_dead_segments;
    std::unique_ptr<ON_PolyCurve> m_replacement;
  };
  bool m_chunks_computed = false;
  std::vector<ChunkResult> m_chunk_results;
  size_t m_chunk_cursor = 0;

  void compute_chunk_results();
  int get_chunk_result(bool& entirely_dead, ON_SimpleArray<ON_COMPONENT_INDEX>& dead_segments, std::unique_ptr<ON_PolyCurve>& replacement);

public:
  RTreeOverlap(const ON_SimpleArray<const ON_Object*>& objects, const ON_Xform& projection, RhinoOverlapMode mode, double tolerance,
               const ON_SimpleArray<const ON_Object*>* locked_objects = nullptr, bool exclusive_locked = false, ON_Terminator* terminator = nullptr) :
  m_objects(objects), m_projection(projection),
  m_results((size_t) objects.Count()),
  m_proxy(objects, projection, tolerance),
  m_locked_proxy(locked_objects ? *locked_objects : objects, projection, tolerance),
  m_terminator(terminator){
    m_mode = mode;
    m_segments = (mode == RhinoOverlapMode::Segments);
    m_tolerance = tolerance;
    m_locked_objects = locked_objects;
    m_exclusive_locked = exclusive_locked;
    initialize_rough_index();
    m_active_fine_curves = 0;
  };

  int get_overlap_result(bool& entirely_dead, ON_SimpleArray<ON_COMPONENT_INDEX>& dead_segments, std::unique_ptr<ON_PolyCurve>& replacement);
};

class CRhinoOverlapFate;

/*
Description:
  Detects overlapping regions among a set of input objects after they are
  orthogonally projected onto the given plane, and reports what should happen
  to each affected object so the overlaps can be removed. The objects are
  compared in the projected plane, so this finds curves that lie on top of one
  another when viewed along the plane's normal.
Parameters:
  objects          - [in] The geometry to test for overlaps. Supported types are
                          ON_Curve (curves), ON_Point (single points), and
                          ON_PointCloud (sets of points). Objects of any other
                          type, as well as null or invalid objects, are silently
                          ignored.
  plane            - [in] The plane the geometry is orthogonally projected onto
                          before testing (the projection maps each point P to
                          plane.ClosestPointTo(P)).
  tolerance        - [in] Overlap distance tolerance, in the projected plane.
  mode             - [in] Granularity of the removal; see RhinoOverlapMode.
                          RhinoOverlapMode::Partial removes arbitrary parameter
                          chunks, with the chunk boundaries computed by the
                          exact curve-curve intersector; transverse crossings
                          are never split.
  locked           - [in] Geometry that participates in the test but is never
                          itself reported or modified. May be empty.
  locked_exclusive - [in] True reports overlaps only where an input object
                          overlaps a locked object; overlaps purely among the
                          input objects are ignored.
  fates            - [out] One CRhinoOverlapFate per affected input object.
                          Cleared on entry. Inputs that are not overlapped
                          produce no entry.
  cancel           - [in] Optional terminator used to cancel a long-running
                          computation. May be nullptr.
Returns:
  True if at least one overlap was found (fates is non-empty); false otherwise.
*/
RHINO_SDK_FUNCTION
bool RhinoGetOverlaps(
  const ON_SimpleArray<const ON_Geometry*>& objects,
  const ON_Plane& plane,
  double tolerance,
  RhinoOverlapMode mode,
  const ON_SimpleArray<const ON_Geometry*>& locked,
  bool locked_exclusive,
  ON_ClassArray<CRhinoOverlapFate>& fates,
  ON_Terminator* cancel = nullptr
);

/*
Description:
  Two-mode convenience overload of RhinoGetOverlaps.
Parameters:
  whole_curves - [in] True is RhinoOverlapMode::WholeCurves, false is
                      RhinoOverlapMode::Segments. All other parameters are as
                      in the overload above.
*/
RHINO_SDK_FUNCTION
bool RhinoGetOverlaps(
  const ON_SimpleArray<const ON_Geometry*>& objects,
  const ON_Plane& plane,
  double tolerance,
  bool whole_curves,
  const ON_SimpleArray<const ON_Geometry*>& locked,
  bool locked_exclusive,
  ON_ClassArray<CRhinoOverlapFate>& fates,
  ON_Terminator* cancel = nullptr
);

// Describes what happened to one input object during an overlap computation. One
// CRhinoOverlapFate is produced for each input object that was affected (covered) by the
// overlap; unaffected inputs produce no entry. When m_entire is true the whole object was
// covered and should be removed. Otherwise m_dead_segments lists the covered segments and
// Replacement(), if non-null, is the trimmed remainder the caller should substitute.
class RHINO_SDK_CLASS CRhinoOverlapFate {
public:
  CRhinoOverlapFate() = default;
  ~CRhinoOverlapFate();                                 // deletes m_replacement
  CRhinoOverlapFate(const CRhinoOverlapFate&);          // duplicates m_replacement
  CRhinoOverlapFate& operator=(const CRhinoOverlapFate&);

  int  m_object_index = -1;     // index into the input array
  bool m_entire = false;        // true: whole object overlapped
  ON_SimpleArray<ON_COMPONENT_INDEX> m_dead_segments;  // used when !m_entire

  // The remainder curve the caller should substitute for the input, or nullptr when there
  // is no replacement. Owned by this object.
  const ON_Curve* Replacement() const { return m_replacement; }
private:
  ON_Curve* m_replacement = nullptr;
  friend RHINO_SDK_FUNCTION bool RhinoGetOverlaps(
    const ON_SimpleArray<const ON_Geometry*>&, const ON_Plane&, double, RhinoOverlapMode,
    const ON_SimpleArray<const ON_Geometry*>&, bool,
    ON_ClassArray<CRhinoOverlapFate>&, ON_Terminator*);
};
