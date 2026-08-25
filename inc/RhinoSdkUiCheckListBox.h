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

/////////////////////////////////////////////////////////////////////////////
// RhinoSdkUiCheckListBox.h
//

#pragma once

#if defined (ON_OS_WINDOWS)

/////////////////////////////////////////////////////////////////////////////
// CRhinoUiListBox.h
//

class RHINO_SDK_CLASS CRhinoUiListBox : public CListBox
{
  DECLARE_DYNAMIC(CRhinoUiListBox)

public:
  // Constructors
  CRhinoUiListBox();
  virtual BOOL32 Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

  virtual ~CRhinoUiListBox();

protected:
  virtual void PreSubclassWindow();
  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};


/////////////////////////////////////////////////////////////////////////////
// CRhinoUiCheckListBox.h
//

class RHINO_SDK_CLASS CRhinoUiCheckListBox : public CCheckListBox
{
	DECLARE_DYNAMIC(CRhinoUiCheckListBox)

public:
  // Constructors
	CRhinoUiCheckListBox();
  virtual BOOL32 Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

  virtual ~CRhinoUiCheckListBox();

  /*
  Description:
    Enables or disables right button click to uncheck all checked items and
    only check the item that was clicked on.
  */
  void EnableRightButtonClickCheckOne(bool bEnable);

  /*
  Description:
    Enables or disabled left mouse button dragging to check items.
  */
  void EnableLeftButtonDragCheck(bool bEnable);

protected:
  virtual void PreSubclassWindow();
  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  DECLARE_MESSAGE_MAP()

private:
  bool m_bRightButtonClickCheckOne = false;
  bool m_bLeftButtonDragCheck = false;
};

#else

#define CRhinoUiListBox CListBox
#define CRhinoUiCheckListBox CCheckListBox

#endif // if defined (ON_OS_WINDOWS)
