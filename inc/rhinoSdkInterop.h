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

// Dictionary used for named callbacks to pass data back and forth
// between C++ and .NET
class RHINO_SDK_CLASS CRhinoParameterDictionary
{
public:
  CRhinoParameterDictionary();
  virtual ~CRhinoParameterDictionary();

  void SetString(const wchar_t* name, const wchar_t* value);
  bool GetString(const wchar_t* name, ON_wString& value) const;
  void SetBool(const wchar_t* name, bool value);
  bool GetBool(const wchar_t* name, bool& value) const;
  void SetInt(const wchar_t* name, int value);
  bool GetInt(const wchar_t* name, int& value) const;
  void SetUnsignedInt(const wchar_t* name, unsigned int value);
  bool GetUnsignedInt(const wchar_t* name, unsigned int& value) const;
  void SetUnsignedIntList(const wchar_t* name, const ON_SimpleArray<unsigned int>& list);
  bool GetUnsignedIntList(const wchar_t* name, ON_SimpleArray<unsigned int>& list);
  void SetDouble(const wchar_t* name, double value);
  bool GetDouble(const wchar_t* name, double& value) const;
  void SetPoint(const wchar_t* name, const ON_2iPoint& point);
  bool GetPoint(const wchar_t* name, ON_2iPoint& point) const;
  void SetPoint(const wchar_t* name, const ON_3dPoint& point);
  bool GetPoint(const wchar_t* name, ON_3dPoint& point) const;
  void SetVector(const wchar_t* name, const ON_3dVector& vector);
  bool GetVector(const wchar_t* name, ON_3dVector& vector) const;
  void SetColor(const wchar_t* name, const ON_Color& point);
  bool GetColor(const wchar_t* name, ON_Color& point) const;
  void SetUuid(const wchar_t* name, const ON_UUID& value);
  bool GetUuid(const wchar_t* name, ON_UUID& value) const;
  void SetUuidList(const wchar_t* name, const ON_SimpleArray<ON_UUID>& value);
  bool GetUuidList(const wchar_t* name, ON_SimpleArray<ON_UUID>& value) const;
  void SetViewport(const wchar_t* name, const ON_Viewport& viewport);
  bool GetViewport(const wchar_t* name, ON_Viewport& viewport) const;
  void SetLine(const wchar_t* name, const ON_Line& line);
  bool GetLine(const wchar_t* name, ON_Line& line) const;
  void SetArc(const wchar_t* name, const ON_Arc& arc);
  bool GetArc(const wchar_t* name, ON_Arc& arc) const;
  void SetPlane(const wchar_t* name, const ON_Plane& plane);
  bool GetPlane(const wchar_t* name, ON_Plane& plane) const;
  void SetPoints(const wchar_t* name, const ON_SimpleArray<ON_3dPoint>& pts);
  bool GetPoints(const wchar_t* name, ON_SimpleArray<ON_3dPoint>& pts) const;
  void SetMeshParameters(const wchar_t* name, const ON_MeshParameters& value);
  bool GetMeshParameters(const wchar_t* name, ON_MeshParameters& value) const;

private:
  friend class CRhParameterDictionary;
  class CRhParameterDictionary* m_private;
};

RHINO_SDK_FUNCTION
bool RhinoExecuteNamedCallback(const wchar_t* name, CRhinoParameterDictionary& p);
