//
// Copyright (c) 1993-2025 Robert McNeel & Associates. All rights reserved.
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

class RHINO_SDK_CLASS CRhinoAnimatingDouble
{
public:
  CRhinoAnimatingDouble();
  CRhinoAnimatingDouble(double value);
  ~CRhinoAnimatingDouble();
  
  double Value() const;
  void SetValue(double value);
private:
  friend class CRhAnimatingDoublePrivate;
  class CRhAnimatingDoublePrivate* m_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoMouseEventArgs
{
public:
  CRhinoMouseEventArgs() = default;
  CRhinoMouseEventArgs(int mouseX, int mouseY, CRhinoMouseCallback::mouse_button button, CRhinoView* view);
  CRhinoMouseEventArgs(const CRhinoMouseEventArgs& other);
  CRhinoMouseEventArgs& operator=(const CRhinoMouseEventArgs& other);
  ~CRhinoMouseEventArgs();

  // Description:
  //  Which mouse button(s) are actively pressed
  CRhinoMouseCallback::mouse_button MouseButton() const;

  // Description:
  //  Returns mouse position in logical pixel units
  ON_2fPoint MousePositionLP() const;

  // Description:
  //  Returns mouse position in on screen device pixel units
  ON_2fPoint MousePositionDP() const;

  ON_3dPoint MousePosition3d() const;

  ON_2fPoint MousePositionClientLP(const ON_4iRect& rect) const;

  ON_Line MouseFrustumLine() const;

  bool IsMouseOver(const CRhinoObject* rhinoObject) const;
  bool IsMouseOver(const ON_Line& line) const;
  bool IsMouseOver(const ON_3dPoint& point) const;
  bool IsMouseOver(const ON_4iRect& rect, bool logicalPixels) const;
  bool IsMouseOver(const ON_Curve& curve, double& t) const;

  CRhinoDoc* Document() const;
  CRhinoView* View() const;
  
private:
  void InitializePrivateData() const;
  
  int m_mouse_x = 0;
  int m_mouse_y = 0;
  CRhinoMouseCallback::mouse_button m_button = CRhinoMouseCallback::mouse_button::no_mouse_button;
  unsigned int m_view_sn = 0;
  mutable class CRhMouseEventArgsPrivate* m_private = nullptr;
};

enum class AnimationStyle
{
  Linear,
  EaseIn,
  EaseOut,
  EaseInOut
};

// Description:
//  Base class for in-viewport user interface objects. These are objects that
//  are shown and can respond to mouse events for things like heads up displays
//  or clickable and dragable items.
class RHINO_SDK_CLASS CRhinoUserInterfaceObject
{
public:
  CRhinoUserInterfaceObject();
  virtual ~CRhinoUserInterfaceObject();

  unsigned int RuntimeSerialNumber() const;
  unsigned int DocumentRuntimeSerialNumber() const;
  ON_UUID DocumentGroupId() const;

  bool RegisterForAllDocuments();
  bool UnregisterFromAllDocuments();

  void BindToActiveViewport(bool on);
  bool IsBoundToActiveViewport() const;
  void BindToViewport(unsigned int viewport_sn);
  unsigned int IsBoundToViewport() const;
  
  CRhinoUserInterfaceObject* Parent();
  const CRhinoUserInterfaceObject* Parent() const;

  // Description:
  //  All user interface objects must implement a Draw function. This is where
  //  drawing using the input display pipeline is performed. The base class
  //  Draw function does nothing.
  virtual void Draw(CRhinoDisplayPipeline& dp);
  
  // Description:
  //  Return true if the input mouse is considered to be over this object
  virtual bool IsMouseOver(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when first goes over this object (IsMouseOver returns true)
  virtual void OnMouseEnter(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when mouse leaves this object (IsMouseOver returns false)
  virtual void OnMouseLeave(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when a mouse button goes down over this UI element
  // Parameters:
  //  mouse - mouse position, button and view
  virtual void OnMouseDown(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when a mouse moves over this UI element
  // Parameters:
  //  mouse - mouse position, button and view
  virtual void OnMouseMove(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when a mouse button goes up over this UI element
  // Parameters:
  //  mouse - mouse position, button and view
  virtual void OnMouseUp(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when a mouse clicks on this UI element
  // Parameters:
  //  mouse - mouse position, button and view
  virtual void OnMouseClick(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when a mouse double clicks on this UI element
  // Parameters:
  //  mouse - mouse position, button and view
  virtual void OnMouseDoubleClick(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Called when a mouse wheel event occurs over this user interface object.
  //  The control needs to explicitly enable mouse wheel support (default off)
  //  in order to receive these events
  virtual void OnMouseWheel(const CRhinoMouseEventArgs& mouse, int delta);

  // Description:
  //  Called when a trackpad scroll event occurs and the mouse is over
  //  this user interface control. The control also needs to explicitly
  //  turn on track pad scroll support (default off) in order to receive
  //  these events.
  virtual void OnTrackpadScroll(const CRhinoMouseEventArgs& mouse, double horizontal, double vertical);

  // Description:
  //  If this is a "world space" type user interface object (grip, text dot, direction arrow),
  //  this function returns the minimum distance to the camera to allow for depth sorting
  //  user interface objects before drawing them.
  //  The default return value is ON_UNSET_VALUE
  virtual double CameraDepth(const ON_Plane& cameraPlane) const;

  bool Visible() const;
  void SetVisible(bool visible);

  // Description:
  //  Sort order for user interface object to be drawn. Values <= 0 will be drawn
  //  before overlay cinduits and values > 0 wil be drawn after overlay conduits.
  int DisplayOrder() const;
  void SetDisplayOrder(int order);

  bool ReceivesMouseWheelEvents() const;
  void SetReceivesMouseWheelEvents(bool enable);

  bool ReceivesTrackpadScrollEvents() const;
  void SetReceivesTrackpadScrollEvents(bool enable);

  // Description:
  //  Call RunCommand to have the virtual OnRunCommand function executed in the
  //  scope of a command. If you are modifying the document in a mouse function,
  //  this is the best approach to do so.
  void RunCommand(const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Use this method to register a new animation
  //  this method also should get called in Draw routine of the parent class for every animation in m_animations
  void Animate(CRhinoAnimatingDouble& value, double target, double duration, AnimationStyle style, CRhinoDoc* doc);

  // Description:
  //  Called after the above RunCommand function is called. This will be
  //  executing while inside a Rhino command.
  virtual CRhinoCommand::result OnRunCommand(const CRhinoCommandContext& context, const CRhinoMouseEventArgs& mouse);

  // Description:
  //  Does this object have mouse capture. Mouse capture is typically set when
  //  the mouse is down and dragging ui objects around. Only one object has
  //  mouse capture at any given time
  bool HasMouseCapture() const;

  // Description:
  //  Set mouse cursor that will be displayed when the mouse is "over" this object
  void SetCursor(HCURSOR cursor);
  
  // Description:
  //  Set or clear the tooltip string for this object
  void SetTooltip(const wchar_t* tooltip);

public:
  void SetData(const wchar_t* key, const wchar_t* value);
  void SetData(const wchar_t* key, bool value);
  void SetData(const wchar_t* key, int value);
  void SetData(const wchar_t* key, double value);
  void SetData(const wchar_t* key, ON_UUID value);
  
  bool GetData(const wchar_t* key, ON_wString& value) const;
  bool GetData(const wchar_t* key, bool& value) const;
  bool GetData(const wchar_t* key, int& value) const;
  bool GetData(const wchar_t* key, double& value) const;
  bool GetData(const wchar_t* key, ON_UUID& value) const;
  
protected:
  virtual void ContinueAnimate(CRhinoDoc* doc);
  void SetMouseCapture(CRhinoDoc* doc);

private:
  void ShowTooltipAfter(UINT uElapse);
  void ClearTooltip();

  static void OnMouseDownHandler(unsigned int rhinoViewSerialNumber,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseMoveHandler(unsigned int rhinoViewSerialNumber,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseUpHandler(unsigned int rhinoViewSerialNumber,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseDoubleClickHandler(unsigned int rhinoViewSerialNumber,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseWheelHandler(unsigned int rhinoViewSerialNumber,
    ON_2iPoint mousePoint,
    int delta,
    bool& continueExecution);
  static void OnTrackpadScrollHandler(unsigned int rhinoViewSerialNumber,
    ON_2iPoint mousePoint,
    double horizontal, double vertical,
    bool& continueExecution);

  static bool UserInterfaceObjectsExist(CRhinoDoc* doc);
  static void DrawUserInterfaceObjects(CRhinoDisplayPipeline& dp, bool beforeDrawOverlays);

  static bool PerformingHitTest();

  void Animate(class CRhUserInterfaceAnimation* animation, CRhinoDoc* doc);
  void SetDocumentRuntimeSerialNumber(unsigned int sn);

private:
  const unsigned int m_user_interface_sn;
  unsigned int m_document_runtime_sn = 0;
  ON_UUID m_groupId;
protected:
  mutable class CRhUserInterfaceObjectPrivate* m_private = nullptr;

  friend class CRhinoDoc;
  friend class CRhinoDisplayPipeline;
  friend class CRhinoObjRef;
  friend class CRhViewBase;
  friend class CRhUserInterfaceObjectPrivate;
};

class RHINO_SDK_CLASS CRhinoTextDotUserInterfaceObject : public CRhinoUserInterfaceObject
{
public:
  CRhinoTextDotUserInterfaceObject();
  CRhinoTextDotUserInterfaceObject(ON_3dPoint location, const wchar_t* text);
  virtual ~CRhinoTextDotUserInterfaceObject();
  void Draw(CRhinoDisplayPipeline& dp) override;
  void Draw(CRhinoDisplayPipeline& dp, const ON_Color& fillColor, const ON_Color& textColor, const ON_Color& borderColor);

  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;

  double CameraDepth(const ON_Plane& cameraPlane) const override;

  void SetText(const wchar_t* text);
  void SetDotLocation(const ON_3dPoint& center);
  void SetDotColors(const ON_Color& background, const ON_Color& border, const ON_Color& text);
  void SetTextHeightPoints(int height, bool mouseOver);
  int TextHeightPoints(bool mouseOver) const;
private:
  class CRhTextDotUserInterfaceObjectPrivate* m_textdot_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoGripUserInterfaceObject : public CRhinoUserInterfaceObject
{
public:
  enum class GripShape
  {
    Circle = 0,
    Square = 1,
    Triangle = 2,
    X = 3,
  };
public:
  CRhinoGripUserInterfaceObject();
  CRhinoGripUserInterfaceObject(const ON_3dPoint& location);
  virtual ~CRhinoGripUserInterfaceObject();
  void Draw(CRhinoDisplayPipeline& dp) override;

  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseMove(const CRhinoMouseEventArgs& mouse) override;

public:
  double CameraDepth(const ON_Plane& cameraPlane) const override;

  // Description:
  //  Gets called while the grip is being dragged to update the location.
  //  The default implementation calls SetGripLocation
  virtual void OnDrag(const ON_3dPoint& point, const CRhinoMouseEventArgs& mouse);

  GripShape GetGripShape() const;
  void SetGripShape(GripShape shape);

  float GripStrokeWidth() const;
  void SetGripStrokeWidth(float width);

  float GripShapeRotationRadians() const;
  void SetGripShapeRotationRadians(float angle);

  void SetGripLocation(const ON_3dPoint& point);
  ON_3dPoint GripLocation() const;

  void SetGripColor(const ON_Color& color);
  ON_Color GripColor() const;

  void SetGripFillColor(const ON_Color& color);
  ON_Color GripFillColor() const;

  void SetGripRadius(float radius);
  float GripRadius() const;

  void ClearConstraints();
  bool Constrain(const ON_Curve& curve);
  bool Constrain(const ON_Circle& circle);
  bool Constrain(const ON_Line& line);
  bool Constrain(const ON_Arc& arc);

  void PermitObjectSnap(bool permit);
  bool ObjectSnapPermitted() const;

  int AddSnapPoints(int pointCount, const ON_3dPoint* snapPoints);
  void ClearSnapPoints();

  void EnableObjectSnapCursors(bool enable);
  bool ObjectSnapCursorsEnabled() const;

  void EnableOnObjectCursors(bool enable);
  bool OnObjectCursorsEnabled() const;

  ON_3dPoint ComputeDragPoint(const CRhinoMouseEventArgs& mouse);
protected:
  class CRhGripUserInterfaceObjectPrivate* m_grip_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoDirectionGripUserInterfaceObject : public CRhinoGripUserInterfaceObject
{
public:
  CRhinoDirectionGripUserInterfaceObject();
  CRhinoDirectionGripUserInterfaceObject(const ON_3dPoint& origin, const ON_3dVector& direction);
  virtual ~CRhinoDirectionGripUserInterfaceObject();

  void OnDrag(const ON_3dPoint& point, const CRhinoMouseEventArgs& mouse) override;

  void Draw(CRhinoDisplayPipeline& dp) override;
  void Draw(CRhinoDisplayPipeline& dp, const ON_Color& color);

  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;

  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseMove(const CRhinoMouseEventArgs& mouse) override;

  bool ArrowsVisibleInViewport(const CRhinoViewport* viewport) const;

  void SetGripPointVisible(bool on);
  bool IsGripPointVisible() const;

  void SetAsOneWay(bool oneWay);
  bool IsOneWay() const;

  void SetGripDirection(const ON_3dVector& direction);
  ON_3dVector GripDirection() const;

  void SetDirectionLineLength(float length);
  float DirectionLineLength() const;

  CRhinoGripUserInterfaceObject::GripShape ArrowShape() const;
  void SetArrowShape(CRhinoGripUserInterfaceObject::GripShape shape);

  void SetArrowRadius(float radius);
  float ArrowRadius() const;
private:
  class CRhDirectionUserInterfaceObjectPrivate* m_direction_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoRotationGripUserInterfaceObject : public CRhinoGripUserInterfaceObject
{
public:
  CRhinoRotationGripUserInterfaceObject();
  CRhinoRotationGripUserInterfaceObject(const ON_Plane& plane, double radius);
  virtual ~CRhinoRotationGripUserInterfaceObject();

  void OnDrag(const ON_3dPoint& point, const CRhinoMouseEventArgs& mouse) override;
  virtual void OnRotationDrag(double angle, const CRhinoMouseEventArgs& mouse);

  void Draw(CRhinoDisplayPipeline& dp) override;
  void Draw(CRhinoDisplayPipeline& dp, const ON_Color& color);

  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;

  bool ArcVisibleInViewport(const CRhinoViewport* viewport) const;

  void SetPlane(const ON_Plane& plane);
  ON_Plane Plane() const;

  void SetRotationRadius(double radius);
  double RotationRadius() const;

  void SetGripPointVisible(bool on);
  bool IsGripPointVisible() const;
private:
  class CRhRotationGripUserInterfaceObjectPrivate* m_rotation_private = nullptr;
};

enum class ControlHorizontalAlignment : int
{
  Left = 0,
  Center = 1,
  Right = 2,
  Stretch = 3
};

enum class ControlVerticalAlignment : int
{
  Top = 0,
  Center = 1,
  Bottom = 2,
  Stretch = 3
};

// Description:
//  Controls are traditional 2d user interface elements that can be drawn and
//  respond to mouse events in a rhino view.
class RHINO_SDK_CLASS CRhinoUserInterfaceControl : public CRhinoUserInterfaceObject
{
public:
  CRhinoUserInterfaceControl();
  virtual ~CRhinoUserInterfaceControl();

  // Description:
  //  Controls can be made to "track" 3d locations in a viewport. In other
  //  words, the control will move around the viewport as if it were glued to
  //  a 3d location. By default the tracking point is unset which means the
  //  control will not track a 3d location
  void SetTrackingPoint(const ON_3dPoint& pt);
  ON_3dPoint TrackingPoint() const;

  // Description:
  //  The size of this control in logical pixels. If size is 0 (default) for
  //  width or height, then a size is computed for the control based on it's
  //  contents.
  void SetSize(float width, float height);
  void SetSize(const ON_2fSize& size);
  ON_2fSize Size() const;

  // Description:
  //  Location of this control in logical pixels. The actual location where the
  //  control ends up on the screen is determined by a combination of location,
  //  alignment and optionally a 3d teacking point.
  void SetLocation(float x, float y);
  void SetLocation(ON_2fPoint location);
  ON_2fPoint Location() const;

  // Description:
  //  Convenience function to set both location and alignment in one call
  void SetLocation(ON_2fPoint location, ControlHorizontalAlignment hAlign, ControlVerticalAlignment vAlign);

  // Description:
  //  How this control is aligned in its parent
  void SetAlignment(ControlHorizontalAlignment h, ControlVerticalAlignment v);
  void GetAlignment(ControlHorizontalAlignment& h, ControlVerticalAlignment& v) const;

  // Description:
  //  Compute the size of this control in logical pixels.
  virtual ON_2fSize ComputeSize() const;

  // Description:
  //  Compute the location and size of this control in the coordinates of a view.
  // Parameters:
  //  view - the view to use for computing location and size inside of. The
  //    same control can be in different locations for different views due to
  //    how the control is aligned in the view or if it is tracking a 3d point
  //  logicalPixels - should the rectangle be return in device or logical pixels
  virtual ON_4dRect ComputedRect(const CRhinoView* view, bool logicalPixels) const;

  void SetText(const wchar_t* text);
  const ON_wString& Text() const;

  void SetImageSVG(const wchar_t* svg);
  const ON_wString& ImageSVG() const;

protected:
  class CRhUserInterfaceControlPrivate* m_ui_control_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoUserInterfaceButton : public CRhinoUserInterfaceControl
{
public:
  CRhinoUserInterfaceButton();
  ~CRhinoUserInterfaceButton();

  void Draw(CRhinoDisplayPipeline& dp) override;
  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;

  ON_2fSize ComputeSize() const override;
  ON_4dRect ComputedRect(const CRhinoView* view, bool logicalPixels) const override;
private:
  class CRhUserInterfaceButtonPrivate* m_ui_button_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoUserInterfaceIcon : public CRhinoUserInterfaceControl
{
public:
  CRhinoUserInterfaceIcon();
  ~CRhinoUserInterfaceIcon();

  void Draw(CRhinoDisplayPipeline& dp) override;
  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;

  ON_2fSize ComputeSize() const override;
  ON_4dRect ComputedRect(const CRhinoView* view, bool logicalPixels) const override;
private:
  class CRhUserInterfaceIconPrivate* m_ui_icon_private = nullptr;
};

class RHINO_SDK_CLASS CRhinoUserInterfaceSlider : public CRhinoUserInterfaceControl
{
public:
  CRhinoUserInterfaceSlider();
  ~CRhinoUserInterfaceSlider();

  void Draw(CRhinoDisplayPipeline& dp) override;
  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseMove(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;

  ON_2fSize ComputeSize() const override;
  ON_4dRect ComputedRect(const CRhinoView* view, bool logicalPixels) const override;

  // Description:
  //  Sliders can be either horizontal or vertically oriented
  bool HorizontalOrientation() const;
  void SetOrientation(bool horizontal);
  
  void SetRange(const ON_Interval& range);
  ON_Interval Range() const;
  void SetAllowValueBeforeRangeStart(bool enable);
  bool AllowValueBeforeRangeStart() const;
  void SetAllowValueAfterRangeEnd(bool enable);
  bool AllowValueAfterRangeEnd() const;
  void SetValue(double value);
  double Value() const;
  virtual void OnValueChanged();

  void EnableValueDisplay(bool on);
  bool DisplayValue() const;
  void SetDigitPrecision(int digits);
  int DigitPrecision() const;

  ON_wString ValueAsFormattedString() const;
private:
  class CRhUserInterfaceSliderPrivate* m_ui_slider_private = nullptr;
};

