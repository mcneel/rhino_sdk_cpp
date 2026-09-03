//
// Internal header - NOT part of the shipped SDK. Included by opennurbs_plus_mtree.cpp, which
// owns the cache's lifetime, and by opennurbs_plus_meshbooleans.cpp, which owns its contents.
// ON_Mesh refers to this type only as an incomplete type (opennurbs_mesh.h), exactly as it does
// for ON_MeshTreeCache, so nothing here is visible to a plug-in and no public layout depends on it.
//

#if !defined(OPENNURBS_PLUS_MESHTESSELLATIONCACHE_INC_)
#define OPENNURBS_PLUS_MESHTESSELLATIONCACHE_INC_

#if defined(OPENNURBS_PLUS)

#include <atomic>
#include <mutex>

// Cached triangulation of one mesh's faces: which diagonal splits each quad, and the mesh's
// naked edge list. Both are pure functions of the mesh, and both cost O(face count) to derive,
// so the intersection routines share one copy per mesh rather than rebuilding it per call.
//
// Held by ON_MeshTreeCache, so it is discarded exactly when the cached trees are. That is the
// correct trigger: the diagonal choice compares the two diagonals' lengths, so it depends on
// vertex POSITIONS as well as on m_F, and ON_Mesh::Transform drops the tree cache.
//
// Created empty. The first consumer fills it under m_build_mutex and sets m_built; every later
// reader only reads. The encoding of m_F_to_T / m_T_to_F belongs to that consumer.
class ON_MeshTessellationCache
{
public:
  ON_MeshTessellationCache() = default;

  // Stored with release once the arrays below are fully written, loaded with acquire on the
  // unlocked fast path, so a reader either sees nothing or sees a completed build.
  std::atomic<bool> m_built{ false };

  ON_SimpleArray<unsigned int> m_F_to_T;
  ON_SimpleArray<unsigned int> m_T_to_F;

  // Derived only once some caller asks for naked edges, then kept. Tracked separately from
  // m_built so a caller that did not ask still sees an empty list, as it did before caching.
  std::atomic<bool> m_naked_built{ false };
  ON_SimpleArray<ON_2udex> m_naked_edges;

  // NOT cached here: the ngon map. ON_Mesh::NgonMap() returns a pointer INTO the mesh's own
  // m_NgonMap, and ON_Mesh::RemoveNgonMap() frees that array without going through the tree
  // cache, so a copy kept across calls could dangle. It stays owned per Tessellation, exactly
  // as it was before this cache existed.

  // Guards the one-time fill only; see the note above.
  std::mutex m_build_mutex;

private:
  // prohibit use - the cache is referred to by pointer and never copied
  ON_MeshTessellationCache(const ON_MeshTessellationCache&);
  ON_MeshTessellationCache& operator=(const ON_MeshTessellationCache&);
};

#endif // OPENNURBS_PLUS

#endif // OPENNURBS_PLUS_MESHTESSELLATIONCACHE_INC_
