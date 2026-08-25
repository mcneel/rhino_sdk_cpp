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

#if defined (RHINO_SDK_MFC)
class RHINO_SDK_CLASS CRhinoAppUiResourceManager : public CRhinoUiResourceManager
{
public:
  CRhinoAppUiResourceManager() = default;
  virtual ~CRhinoAppUiResourceManager();

public:
  HCURSOR GetCursor( cursors) override;
  bool GetImageList( images, CRhinoUiImageList&) override;
  COLORREF GetImageMaskingColor() override;
  bool GetBitmap( bitmaps, CRhinoDib&) override;
  void CreateTextCursors();

private:
  HCURSOR m_cursor_tb_shift = 0;
  HCURSOR m_cursor_tb_ctrl  = 0;
  HCURSOR m_cursor_tb_shift_ctrl = 0;
  HCURSOR m_cursor_tb_move = 0;
  HCURSOR m_cursor_tb_link = 0;
  HCURSOR m_cursor_tb_copy = 0;
  HCURSOR m_cursor_tb_delete = 0;
private:
  // no implementation - prohibit use
  CRhinoAppUiResourceManager(const CRhinoAppUiResourceManager&);
  CRhinoAppUiResourceManager& operator=(const CRhinoAppUiResourceManager&);
};
#endif

RHINO_SDK_FUNCTION bool RhinoOnToolBarButtonClicked(const wchar_t* lpsMacro, const wchar_t* lpsTooltip);

#if defined (ON_RUNTIME_WIN)
#define RHINO_UILIB_DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)\
  EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif

// The above macro does not use legal C++.  You cannot both declare and define a variable in a header file that is included everywhere.
// The following lines only declare the global variables and are duplicated in MacHelpers.mm to define the variables once.

#if !defined (ON_RUNTIME_WIN)
#if defined RHINO_THIRD_PARTY_OSX_PLUGIN_COMPILE
#define RHINO_UILIB_DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)  //Omit these from Mac plug-ins.
#else
#define RHINO_UILIB_DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)\
EXTERN_C const GUID DECLSPEC_SELECTANY name;
#endif
#endif

RHINO_UILIB_DEFINE_GUID( RHINO_COMMAND_DOCK_BAR_ID,                  0x1D3D1785, 0x2332, 0x428B, 0xA8, 0x38, 0xB2, 0xFE, 0x39, 0xEC, 0x50, 0xF4);
RHINO_UILIB_DEFINE_GUID( RHINO_CURVATURE_ANALYSIS_DOCK_BAR_ID,       0x948D3151, 0x91AA, 0x4959, 0xAD, 0x67, 0xBD, 0x49, 0xFB, 0xB2, 0x48, 0x79);
RHINO_UILIB_DEFINE_GUID( RHINO_CURVATURE_GRAPH_DOCK_BAR_ID,          0x54D560F5, 0xC1B2, 0x4BBA, 0x95, 0x34, 0x3C, 0x00, 0xE4, 0x0D, 0x30, 0x57);
RHINO_UILIB_DEFINE_GUID( RHINO_DRAFT_ANGLE_DOCK_BAR_ID,              0xF814F8D8, 0xEEC0, 0x46FD, 0x85, 0x85, 0xE0, 0xE8, 0x48, 0x6A, 0xB8, 0x4D);
RHINO_UILIB_DEFINE_GUID( RHINO_THICKNESS_DISP_DOCK_BAR_ID,           0xc0c9e0f1, 0xd554, 0x453b, 0x87, 0x12, 0xa0, 0x20, 0xac, 0xe0, 0xb8, 0xe4);
RHINO_UILIB_DEFINE_GUID( RHINO_EDGE_ANALYSIS_DOCK_BAR_ID,            0xC9FB93D6, 0xC1AB, 0x4464, 0xAD, 0x3A, 0xED, 0x2A, 0x8B, 0xC1, 0x20, 0x9B);
RHINO_UILIB_DEFINE_GUID( RHINO_EMAP_DOCK_BAR_ID,                     0xCA1753B1, 0xAE23, 0x4880, 0xAD, 0x93, 0x39, 0xB5, 0x29, 0x95, 0x22, 0xE6);
RHINO_UILIB_DEFINE_GUID( RHINO_OSNAP_DOCK_BAR_ID,                    0xd3c4a392, 0x88de, 0x4c4f, 0x88, 0xa4, 0xba, 0x56, 0x36, 0xef, 0x7f, 0x38);
RHINO_UILIB_DEFINE_GUID( RHINO_VIEW_MANAGER_DOCK_BAR_ID,             0x844b3278, 0xe78e, 0x4844, 0x86, 0x8d, 0x63, 0x9b, 0x9b, 0x65, 0x46, 0x50);
RHINO_UILIB_DEFINE_GUID( RHINO_ZEBRA_DOCK_BAR_ID,                    0x36EF80FE, 0x7D51, 0x41EE, 0x80, 0x44, 0xF1, 0x7B, 0x10, 0x73, 0x09, 0xCB);
RHINO_UILIB_DEFINE_GUID( RHINO_SELECTION_FILTER_DOCK_BAR_ID,         0x918191ca, 0x1105, 0x43f9, 0xa3, 0x4a, 0xdd, 0xa4, 0x27, 0x68, 0x83, 0xc1);
RHINO_UILIB_DEFINE_GUID( RHINO_CLASH_DOCK_BAR_ID,                    0x770ff06a, 0xd775, 0x41e4, 0xbb, 0xc7, 0x9f, 0x32, 0xed, 0x18, 0x9b, 0x5e);
RHINO_UILIB_DEFINE_GUID( RHINO_EDGE_CONTINUITY_DOCK_BAR_ID,          0xf7cda9c5, 0xa66c, 0x4ae6, 0x87, 0xb5, 0x89, 0xc2, 0x42, 0x54, 0xe3, 0xb3);

// Unused; kept around to not break the SDK
RHINO_UILIB_DEFINE_GUID( RHINO_DRAFT_ANGLE_ETO_DOCK_BAR_ID,    0xFE0CBB9E, 0x336B, 0x4482, 0x9A, 0xBE, 0xB9, 0x4B, 0x35, 0xB0, 0x08, 0xFD);

RHINO_UILIB_DEFINE_GUID( RHINO_CONTOUR_ANALYSIS_DOCK_BAR_ID,         0xa0f238df, 0x1898, 0x47cf, 0x91, 0x9d, 0xf8, 0x4d, 0xe9, 0x5a, 0x0b, 0xb9);

//Removed - these control bars are now included in the docking tab system.
//RHINO_UILIB_DEFINE_GUID( RHINO_LAYER_DOCK_BAR_ID,              0x16B641D3, 0x75AF, 0x4D2C, 0x86, 0x1E, 0xB9, 0x6E, 0x38, 0x7B, 0x6F, 0x80);
//RHINO_UILIB_DEFINE_GUID( RHINO_LAYER_MANAGER_DOCK_BAR_ID,      0x2363ed67, 0x616d, 0x4861, 0x9e, 0x87, 0xe2, 0xfe, 0x8f, 0xe7, 0x88, 0xa5);
//RHINO_UILIB_DEFINE_GUID( RHINO_PROPERTIES_DOCK_BAR_ID,         0x20A1348C, 0xD529, 0x44F6, 0x8B, 0xD3, 0xEC, 0x8C, 0x00, 0x1B, 0x2B, 0xB7);
//RHINO_UILIB_DEFINE_GUID( RHINO_NOTES_DOCK_BAR_ID,              0x6641B35A, 0xA21E, 0x4350, 0xA5, 0xF7, 0xCC, 0xCA, 0x3E, 0xC6, 0xAC, 0x72);
//RHINO_UILIB_DEFINE_GUID( RHINO_COMMAND_CONTEXT_BAR_ID,         0x7dd6a049, 0x72ca, 0x457d, 0xbf, 0x12, 0x37, 0x10, 0x0d, 0x7e, 0xe9, 0x67);

RHINO_UILIB_DEFINE_GUID( RHINO_DOCKBARTAB_LAYER,			   0x3610bf83, 0x047d, 0x4f7f, 0x93, 0xfd, 0x16, 0x3e, 0xa3, 0x05, 0xb4, 0x93);
RHINO_UILIB_DEFINE_GUID( RHINO_DOCKBARTAB_PROPERTIES,		   0x34ffb674, 0xc504, 0x49d9, 0x9f, 0xcd, 0x99, 0xcc, 0x81, 0x1d, 0xcd, 0xa2);                     

RHINO_UILIB_DEFINE_GUID(RHINO_END_ANALYSIS_DOCK_BAR_ID, 0x4c2d9eed, 0xbc64, 0x48ba, 0xb9, 0x10, 0x40, 0xa0, 0xe6, 0x5c, 0x0a, 0xe3);
RHINO_UILIB_DEFINE_GUID(RHINO_DIRECTION_ANALYSIS_DOCK_BAR_ID, 0x9a0a0262, 0x6e97, 0x4bb6, 0xa9, 0x0c, 0x4f, 0xd7, 0x93, 0x01, 0xd6, 0x77);

RHINO_UILIB_DEFINE_GUID(RHINO_DRAG_STRENGTH_DOCK_BAR_ID, 0x50eb6c7b, 0xbe0, 0x4166, 0xad, 0x17, 0xea, 0x69, 0x4c, 0x97, 0xa6, 0xf4);
//
// End of lines copied to MacHelpers.mm
//

#if defined (RHINO_SDK_MFC)
class RHINO_SDK_CLASS CRhinoAppUiDockBarManager : public CRhinoUiDockBarManager
{
public:
  CRhinoAppUiDockBarManager() = default;
  virtual ~CRhinoAppUiDockBarManager();

  class CRhinoStatusBar* GetRhinoStatusBar() const;
  class CRhOsnapToolBar* GetRhinoOsnapToolBar() const;
  class CRhSelectionFilterToolBar* GetRhinoSelectionFilterToolBar() const;

protected:
  bool OnCReateDefaultDockBars() override;
  bool CreateStatusBar( CFrameWnd* pMainFrame) override;
  void OnSetDockStateCompleted() override;

private:
  class CRhinoStatusBar* m_status_bar = nullptr;

  class CRhinoDropTarget* m_pNamedViewDropTarget = 0;
  class CRhinoDropTarget* m_pNamedCPlaneDropTarget = 0;
};

class RHINO_SDK_CLASS CRhinoAppUiManager : public CRhinoUiManager
{
public:
  CRhinoAppUiManager() = default;
  virtual ~CRhinoAppUiManager();

  CRhinoUiResourceManager& ResourceManager() override;
  CRhinoUiDockBarManager& DockBarManager() override;
  CRhinoUiPaintManager& PaintManager() override;
  bool IsPlugInLoaded(const UUID& plug_in_id) const override;
  bool LoadPlugIn(const UUID& plug_in_id, bool fail_quietly = false, bool ignore_load_failed = false) override;
  CRuntimeClass* RuntimeClassFromName(const UUID& plug_in_id, const wchar_t* lpsClassName) override;
  void DisplayDockLocationContextDialog(CDockBar& dock_bar, CPoint pt) override;
  bool ColorDialog(CWnd* pWndParent, ON_Color& color, bool include_button_colors = false, const wchar_t* dialog_title = NULL) override;

protected:
  CRhinoAppUiResourceManager m_test_resource_manager;
};

#endif
