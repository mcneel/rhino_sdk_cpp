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

/*
Description:
  Options for RhinoSdkBlend::SurfaceBlend. The defaults give the result of
  the BlendSrf command when its dialog is accepted unchanged.
  Side 0 is the first edge, side 1 the second.
*/
class RHINO_SDK_CLASS CArgsRhinoSurfaceBlend
{
public:
  CArgsRhinoSurfaceBlend();
  ~CArgsRhinoSurfaceBlend();
  CArgsRhinoSurfaceBlend(const CArgsRhinoSurfaceBlend&);
  CArgsRhinoSurfaceBlend& operator=(const CArgsRhinoSurfaceBlend&);

  /*
  Returns:
    Continuity of the blend at the side's edge.
      0: position
      1: tangency
      2: curvature (default)
      3: G3
      4: G4
  */
  int Continuity(int side) const;

  /*
  Parameters:
    continuity - [in] 0 to 4. Other values are clamped to that range.
  */
  void SetContinuity(int side, int continuity);

  /*
  Returns:
    Scale factor for the blend's tangent lengths at the side's edge,
    the Bulge setting of BlendSrf. The default is 1.0.
  */
  double Bulge(int side) const;

  /*
  Parameters:
    bulge - [in] Must be greater than zero. Other values leave the
                 setting unchanged.
  */
  void SetBulge(int side, double bulge);

  /*
  Returns:
    True to give every blend shape the height of a shape spanning the
    average of the gaps at the two ends of the blend, the Same height
    setting of BlendSrf. The default is false.
  */
  bool SameHeight() const;
  void SetSameHeight(bool bSameHeight);

  /*
  Returns:
    True to add blend shapes at the edit points of the edges when the edges
    are compatible, the Interior shapes setting of BlendSrf. Has no effect when
    Refine() is true. The default is true.
  */
  bool InteriorShapes() const;
  void SetInteriorShapes(bool bInteriorShapes);

  /*
  Returns:
    True to fit the blend to tolerance instead of making the simple blend
    that compatible edges allow, the Refine setting of BlendSrf.
    The default is false.
  */
  bool Refine() const;
  void SetRefine(bool bRefine);

  /*
  Returns:
    True if every blend shape lies in a plane parallel to
    PlanarSectionDirection(), the Planar sections setting of BlendSrf.
    The default is false.
  */
  bool PlanarSections() const;
  void SetPlanarSections(bool bPlanarSections);

  /*
  Returns:
    Direction the planar sections are parallel to. Used only when
    PlanarSections() is true. The default is the world z axis.
  */
  ON_3dVector PlanarSectionDirection() const;

  /*
  Parameters:
    direction - [in] A zero or invalid vector leaves the setting unchanged.
  */
  void SetPlanarSectionDirection(const ON_3dVector& direction);

private:
  int m_continuity[2] = { 2, 2 };
  double m_bulge[2] = { 1.0, 1.0 };
  bool m_bSameHeight = false;
  bool m_bInteriorShapes = true;
  bool m_bRefine = false;
  bool m_bPlanarSections = false;
  ON_3dVector m_planar_section_direction = ON_3dVector::ZAxis;

  // Settings added in later versions go here, so the layout of this class does not change
  class CRhSurfaceBlendArgsPrivate* m_private = nullptr;
};

class RHINO_SDK_CLASS RhinoSdkBlend
{
public:
  /*
  Description
  Makes a curve blend between 2 points using the directions, 
  curvatures & continuities specified
  Parameters:
  [in] position   - endpoints of the blend
  [in] tangent    - start and end directions for the blend
  [in] curvature  - start and end curvature for the blend
  [in] continuity - start and end continuity for the blend
  0: position
  1: tangency
  2: curvature
  Returns:
  The blend as an ON_Curve if successful
  NULL if not
  */
  static ON_Curve* CurveBlend(
    ON_3dPoint position[2],
    ON_3dVector tangent[2],
    ON_3dVector curvature[2],
    int continuity[2]);

/*
  Description
    Makes a curve blend between 2 curves at the parameters specified
      with the directions and continuities specified
  Parameters:
    [in] pC0         - First curve to blend from
    [in] t0          - Parameter on first curve for blend endpoint
    [in] bRev0       - if bRev0 is false, the blend will go in the natural direction of the curve
                       if bRev0 is true, the blend will go in the opposite direction to the curve
    [in] continuity0 - continuity for the blend at the start
                           0: position
                           1: tangency
                           2: curvature
    [in] pC1         - Second curve to blend from
    [in] t1          - Parameter on second curve for blend endpoint
    [in] bRev1       - if bRev1 is false, the blend will go in the natural direction of the curve
                       if bRev1 is true, the blend will go in the opposite direction to the curve
    [in] continuity1 - continuity for the blend at the end
                           0: position
                           1: tangency
                           2: curvature
  Returns:
    The blend as an ON_Curve if successful
    NULL if not
*/
  static ON_Curve* CurveBlend(
    const ON_Curve* pC0,
    double t0,
    bool bRev0,
    int continuity0, 
    const ON_Curve* pC1,
    double t1,
    bool bRev1,
    int continuity1);


  /*
    Description
      Makes a surface blend between 2 surface edges
    Parameters:
      [in] pF0         - First face to blend from
      [in] pE0         - First edge to blend from
      [in] domain0     - domain on pE0 to use
      [in] bRev0       - if bRev0 is false, pE0 will be used in its natural direction
                         if bRev0 is true, pE0 will be used in the reversed direction
      [in] continuity0 - continuity for the blend at the start
                             0: position
                             1: tangency
                             2: curvature
                             3: G3
                             4: G4
      [in] pF1         - Second face to blend from
      [in] pE1         - Second edge to blend from
      [in] domain1     - domain on pE1 to use
      [in] bRev1       - if bRev1 is false, pE1 will be used in its natural direction
                         if bRev1 is true, pE1 will be used in the reversed direction
      [in] continuity1 - continuity for the blend at the start
                             0: position
                             1: tangency
                             2: curvature
                             3: G3
                             4: G4
      [out] output_surfaces - results returned here
    Returns:
      true if successful
      false if not
    Remarks:
      Same as the form that takes CArgsRhinoSurfaceBlend, with default settings and
      the absolute tolerance of the modeless user interface document.
  */
  ON_DEPRECATED_MSG("Use the form that takes a rhino document")
  static bool SurfaceBlend(
    const ON_BrepFace* pF0,
    const ON_BrepEdge* pE0,
    ON_Interval domain0,
    bool bRev0,
    int continuity0,
    const ON_BrepFace* pF1,
    const ON_BrepEdge* pE1,
    ON_Interval domain1,
    bool bRev1,
    int continuity1,
    ON_SimpleArray<ON_Brep*>& output_surfaces);

/*
  Description
    Makes a surface blend between 2 surface edges
  Parameters:
    [in] doc         - Rhino doc
    [in] pF0         - First face to blend from
    [in] pE0         - First edge to blend from
    [in] domain0     - domain on pE0 to use
    [in] bRev0       - if bRev0 is false, pE0 will be used in its natural direction
                       if bRev0 is true, pE0 will be used in the reversed direction
    [in] continuity0 - continuity for the blend at the start
                           0: position
                           1: tangency
                           2: curvature
                           3: G3
                           4: G4
    [in] pF1         - Second face to blend from
    [in] pE1         - Second edge to blend from
    [in] domain1     - domain on pE1 to use
    [in] bRev1       - if bRev1 is false, pE1 will be used in its natural direction
                       if bRev1 is true, pE1 will be used in the reversed direction
    [in] continuity1 - continuity for the blend at the start
                           0: position
                           1: tangency
                           2: curvature
                           3: G3
                           4: G4
    [out] output_surfaces - results returned here
  Returns:
    true if successful
    false if not
  Remarks:
    Same as the form that takes CArgsRhinoSurfaceBlend, with default settings and
    the absolute tolerance of doc.
*/
  static bool SurfaceBlend(
    CRhinoDoc& doc,
    const ON_BrepFace* pF0,
    const ON_BrepEdge* pE0,
    ON_Interval domain0,
    bool bRev0,
    int continuity0,
    const ON_BrepFace* pF1,
    const ON_BrepEdge* pE1,
    ON_Interval domain1,
    bool bRev1,
    int continuity1,
    ON_SimpleArray<ON_Brep*>& output_surfaces);

/*
  Description
    Makes the surface blend the BlendSrf command makes between 2 surface edges
  Parameters:
    [in] pF0         - First face to blend from
    [in] pE0         - Edge of pF0 to blend from
    [in] domain0     - Part of pE0 to use, in pE0's parameters
    [in] bRev0       - if bRev0 is false, pE0 will be used in its natural direction
                       if bRev0 is true, pE0 will be used in the reversed direction
    [in] pF1         - Second face to blend from
    [in] pE1         - Edge of pF1 to blend from
    [in] domain1     - Part of pE1 to use, in pE1's parameters
    [in] bRev1       - if bRev1 is false, pE1 will be used in its natural direction
                       if bRev1 is true, pE1 will be used in the reversed direction
    [in] args        - Continuity and the other BlendSrf settings
    [in] tolerance   - Fitting tolerance, the document's absolute tolerance in BlendSrf
    [out] output_surfaces - results are appended here
  Returns:
    true if successful
    false if not
  Remarks:
    When the whole of two closed edges is used, the blend is closed, and the seams
    and directions are matched the way BlendSrf matches them automatically.
    bRev0 and bRev1 are not used in that case.
*/
  static bool SurfaceBlend(
    const ON_BrepFace* pF0,
    const ON_BrepEdge* pE0,
    ON_Interval domain0,
    bool bRev0,
    const ON_BrepFace* pF1,
    const ON_BrepEdge* pE1,
    ON_Interval domain1,
    bool bRev1,
    const CArgsRhinoSurfaceBlend& args,
    double tolerance,
    ON_SimpleArray<ON_Brep*>& output_surfaces);

/*
  Description
    Scales unit tangents at the ends of blend curves according to the angle and distance between them
  Parameters:
    [in] P0          - Point at first end of blend
    [in] P1          - Point at second end of curve
    [in][out] T0     - Unit tangent in, returned with length changed for making blend curve
    [in][out] T1     - Unit tangent in, returned with length changed for making blend curve
    [in] pointcount  - Number of cvs that will be made in the blend cure
*/
  static void ScaleBlendTangents(
    ON_3dPoint P0, 
    ON_3dPoint P1,
    ON_3dVector& T0,
    ON_3dVector& T1,
    int pointcount);

/*
  Description
    Makes a curve blend between points on 2 surface edges
    The blend will be tangent to the surfaces and perpendicular to the edges
  Parameters:
    [in] pF0         - First face to blend from
    [in] pE0         - First edge to blend from
    [in] t0          - location on pE0 for first end of blend curve
    [in] bRev0       - if bRev0 is false, the edge's trim and the edge are in the same direction
                       if bRev0 is true, the edge is reversed from its trim
    [in] continuity0 - continuity for the blend at the start
                           0: position
                           1: tangency
                           2: curvature
  Parameters:
    [in] pF1         - Second face to blend from
    [in] pE1         - Second edge to blend from
    [in] t1          - location on pE1 for second end of blend curve
    [in] bRev1       - if bRev1 is false, the edge's trim and the edge are in the same direction
                       if bRev1 is true, the edge is reversed from its trim
    [in] continuity0 - continuity for the blend at the start
                           0: position
                           1: tangency
                           2: curvature
  Returns:
    Blend curve if successful
    NULL if not
*/
  static ON_Curve* SurfaceBlendShape(
    const ON_BrepFace* pF0,
    const ON_BrepEdge* pE0,
    double t0,
    bool bRev0,
    int continuity0,
    const ON_BrepFace* pF1,
    const ON_BrepEdge* pE1,
    double t1,
    bool bRev1,
    int continuity1);

/*
  Description
    Finds the location, tangent and curvature for a blend curve
    that will be tangent and curvature continuous to the surface
    and perpendicular to the edge at t
  Parameters:
    [in] pF          - Face to blend from
    [in] pE          - Edge to blend from
    [in] t           - parameter on pE to use
    [in] bRev        - if bRev is false, the edge's trim and the edge are in the same direction
                       if bRev is true, the edge is reversed from its trim
    [out] P          - Point at pE(t)
    [out] T          - Unit direction tangent to surface and perpendicular to pE at t
    [out] K          - Curvature of the surface in the direction of t
  Returns:
    true if successful
    false if not
*/
  static bool GetSurfaceTangentAndCurvature(
    const ON_BrepFace* pF,
    const ON_BrepEdge* pE, 
    double t,
    bool bRev,
    ON_3dPoint& P, 
    ON_3dVector& T, 
    ON_3dVector& K);
};





