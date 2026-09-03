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
  CRhinoMouseEventArgs(int mouseX, int mouseY, CRhinoMouseCallback::mouse_button button, CRhinoViewport* viewport);
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

  // Description:
  //  The viewport this event happened in
  CRhinoViewport* Viewport() const;

  // Description:
  //  The view this event came from, or nullptr when it came from a surface that is not a view.
  //  Prefer Pipeline() in code that has to work in both.
  CRhinoView* View() const;

  // Description:
  //  The pipeline drawing this surface, whether or not it is a view.
  CRhinoDisplayPipeline* Pipeline() const;

  // Description:
  //  Device pixels per logical pixel for the surface this event came from. 1.0 when unknown.
  float DpiScale() const;

private:
  void InitializePrivateData() const;

  int m_mouse_x = 0;
  int m_mouse_y = 0;
  CRhinoMouseCallback::mouse_button m_button = CRhinoMouseCallback::mouse_button::no_mouse_button;
  unsigned int m_viewport_sn = 0;
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
  virtual void OnTrackpadScroll(const CRhinoMouseEventArgs& mouse, float horizontal, float vertical);

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
  //  Does this object have keyboard focus. Only one user interface object has
  //  keyboard focus at a time, and only that object is sent characters and keys.
  //  Everything typed in a viewport goes to the command line as usual when no
  //  object has focus, or when the object with focus does not use the key.
  bool HasKeyboardFocus() const;

  // Description:
  //  Called when a character is typed while this object has keyboard focus.
  // Parameters:
  //  character - [in] the character that was typed
  //  flags - [in] platform key state flags
  // Returns:
  //  True when this object used the character. A character this object does not
  //  use carries on to the command line, the way typing in a viewport normally
  //  works. The base class returns false.
  virtual bool OnCharacter(unsigned int character, unsigned int flags);

  // Description:
  //  Called when a key goes down while this object has keyboard focus. This is
  //  where the keys that do not produce characters arrive: the arrows, Home,
  //  End, Backspace, Delete, Enter and Escape.
  // Parameters:
  //  virtualKey - [in] virtual key code, VK_LEFT and friends
  //  flags - [in] platform key state flags
  // Returns:
  //  True when this object used the key. The base class returns false.
  virtual bool OnKeyDown(unsigned int virtualKey, unsigned int flags);

  // Description:
  //  How keyboard focus arrived. A control can want to behave differently for
  //  each: a text box selects all of its text when Tab lands on it, the way a
  //  dialog field does, but leaves the text alone when it was clicked, because
  //  the click also says where in the text to put the caret.
  enum class FocusReason : int
  {
    Unknown = 0,    // asked for in code, with nothing more specific said
    Pointer = 1,    // clicked on
    Navigation = 2, // Tab or Shift+Tab moved on to it from a sibling
  };

  // Description:
  //  Why the object that has keyboard focus was given it. Read this from
  //  OnKeyboardFocusChanged; it means nothing once focus has moved on.
  static FocusReason KeyboardFocusReason();

  // Description:
  //  Called when this object gains or loses keyboard focus. The base class
  //  implementation does nothing. KeyboardFocusReason says how focus arrived.
  virtual void OnKeyboardFocusChanged(bool hasFocus);

  // Description:
  //  Can this object take keyboard focus? This is what Tab looks at when it goes
  //  looking for the next control to move to. The base class returns false.
  virtual bool AcceptsKeyboardFocus() const;

  // Description:
  //  Give keyboard focus to the control that comes after this one, or before it,
  //  among the controls in the same container. Tab order is the order the
  //  controls were added in and it wraps around at the ends. Controls that are
  //  hidden or that do not accept keyboard focus are passed over.
  // Parameters:
  //  forward - [in] true moves on to the next control, false back to the previous
  //  doc - [in] document to redraw, can be nullptr
  // Returns:
  //  True when focus moved to another control. False when this control is not in
  //  a container, or when no other control in it will take focus.
  bool MoveKeyboardFocusToSibling(bool forward, CRhinoDoc* doc);

  // Description:
  //  Take keyboard focus away from whichever object has it, if any. This is what
  //  a click that lands somewhere other than on that object does, and it is how
  //  code ends an edit without waiting for the user to press Enter.
  // Parameters:
  //  doc - [in] document to redraw, can be nullptr
  static void ClearKeyboardFocus(CRhinoDoc* doc);

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

  // Description:
  //  Take keyboard focus. The object that had it, if any, is told that it lost it.
  //  The reason is what KeyboardFocusReason hands back for as long as this object
  //  keeps focus.
  void SetKeyboardFocus(CRhinoDoc* doc, FocusReason reason = FocusReason::Unknown);

  // Description:
  //  Give up keyboard focus. Does nothing when this object does not have it.
  void ReleaseKeyboardFocus(CRhinoDoc* doc);

private:
  void ShowTooltipAfter(UINT uElapse);
  void ClearTooltip();

  // The view calls these when a key or a character arrives. They go to the object
  // that has keyboard focus and to no other object.
  static void OnCharHandler(unsigned int rhinoViewSerialNumber,
    unsigned int character,
    unsigned int flags,
    bool& continueExecution);
  static void OnKeyDownHandler(unsigned int rhinoViewSerialNumber,
    unsigned int virtualKey,
    unsigned int flags,
    bool& continueExecution);

  // Is an object in this view holding keyboard focus? What is typed at a viewport belongs
  // to that object rather than to the command line.
  static bool HasKeyboardFocusObject(unsigned int rhinoViewSerialNumber);

  // The view calls these as an input method editor composes. They go to the object that
  // has keyboard focus and to no other object.
  static void OnCompositionStartHandler(unsigned int rhinoViewSerialNumber, bool& continueExecution);
  static void OnCompositionChangedHandler(unsigned int rhinoViewSerialNumber, const wchar_t* composition,
    int caretPosition, int targetStart, int targetLength, bool& continueExecution);
  static void OnCompositionCommittedHandler(unsigned int rhinoViewSerialNumber, const wchar_t* result, bool& continueExecution);
  static void OnCompositionEndHandler(unsigned int rhinoViewSerialNumber, bool& continueExecution);

  // Where to put the composition and candidate windows: the caret of the object that has
  // keyboard focus, in device pixels from the top left of the view. False when no object
  // has focus, or the one that has it does not say where its caret is.
  static bool GetCompositionRectsHandler(unsigned int rhinoViewSerialNumber, ON_4iRect& caretRect, ON_4iRect& exclusionRect);

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
    float horizontal, float vertical,
    bool& continueExecution);

  // A display window is drawn by a display pipeline but is not a CRhinoView, so it has no view
  // serial number to dispatch through. These are the same handlers keyed on the pipeline
  // instead. The view flavours above forward to these.
  static void OnMouseDownHandler(CRhinoDisplayPipeline* pipeline,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseMoveHandler(CRhinoDisplayPipeline* pipeline,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseUpHandler(CRhinoDisplayPipeline* pipeline,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseDoubleClickHandler(CRhinoDisplayPipeline* pipeline,
    CRhinoMouseCallback::mouse_button button,
    ON_2iPoint mousePoint,
    unsigned int flags,
    bool& continueExecution);
  static void OnMouseWheelHandler(CRhinoDisplayPipeline* pipeline,
    ON_2iPoint screenPoint,
    int delta,
    bool& continueExecution);
  static void OnTrackpadScrollHandler(CRhinoDisplayPipeline* pipeline,
    ON_2iPoint mousePoint,
    float horizontal, float vertical,
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
  friend class CRhinoDisplayWindowBase;
  friend class CRhUserInterfaceObjectPrivate;
  friend class CRhinoUserInterfaceGrid;
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
  //  Compute the location and size of this control in the coordinates of the
  //  thing that is drawing it.
  // Parameters:
  //  pipeline - the display pipeline to compute location and size inside of.
  //    The same control can end up in different places in different pipelines,
  //    because of how it is aligned or because it is tracking a 3d point. Take
  //    the pipeline rather than the view it belongs to: a detail on a layout is
  //    drawn by its own nested pipeline whose view is the page view, so a view
  //    cannot say which of the two is being drawn.
  //  logicalPixels - should the rectangle be return in device or logical pixels
  virtual ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const;

  void SetText(const wchar_t* text);
  const ON_wString& Text() const;

  // Description:
  //  Height of the text in logical pixels. A value <= 0 (default) means use
  //  the standard control font height. Controls that draw no text ignore this.
  void SetTextHeight(float height);
  float TextHeight() const;

  // Description:
  //  Color used to draw the text. The default is ON_UNSET_COLOR, which means
  //  use the standard text color for the theme that is in use.
  void SetTextColor(const ON_Color& color);
  ON_Color TextColor() const;

  // Description:
  //  Where the text sits inside the control's rectangle. This is not the same
  //  as SetAlignment, which is where the control's rectangle sits inside its
  //  parent. The default is left / center.
  void SetTextAlignment(ControlHorizontalAlignment h, ControlVerticalAlignment v);
  void GetTextAlignment(ControlHorizontalAlignment& h, ControlVerticalAlignment& v) const;

  void SetImageSVG(const wchar_t* svg);
  const ON_wString& ImageSVG() const;

protected:
  class CRhUserInterfaceControlPrivate* m_ui_control_private = nullptr;
};

// Description:
//  A control that draws a single line of text. Labels do not respond to the
//  mouse; they are the descriptive text that goes in the first column of a
//  CRhinoUserInterfaceGrid, but they can be used anywhere a control can.
class RHINO_SDK_CLASS CRhinoUserInterfaceLabel : public CRhinoUserInterfaceControl
{
public:
  CRhinoUserInterfaceLabel();
  CRhinoUserInterfaceLabel(const wchar_t* text);
  virtual ~CRhinoUserInterfaceLabel();

  void Draw(CRhinoDisplayPipeline& dp) override;

  ON_2fSize ComputeSize() const override;
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;

private:
  class CRhUserInterfaceLabelPrivate* m_ui_label_private = nullptr;
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
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;
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
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;
private:
  class CRhUserInterfaceIconPrivate* m_ui_icon_private = nullptr;
};

// Description:
//  A control that draws a check box with optional text beside it. Clicking
//  anywhere on the control changes the state and calls OnCheckedChanged, which
//  is the virtual to override to do something about the change.
class RHINO_SDK_CLASS CRhinoUserInterfaceCheckBox : public CRhinoUserInterfaceControl
{
public:
  enum class CheckState : int
  {
    Off = 0,
    On = 1,
    Indeterminate = 2,
  };

public:
  CRhinoUserInterfaceCheckBox();
  CRhinoUserInterfaceCheckBox(const wchar_t* text);
  virtual ~CRhinoUserInterfaceCheckBox();

  void Draw(CRhinoDisplayPipeline& dp) override;
  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseClick(const CRhinoMouseEventArgs& mouse) override;

  ON_2fSize ComputeSize() const override;
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;

  // Description:
  //  State of the check box. Setting the state does not call OnCheckedChanged;
  //  that is only for a state the user changed by clicking.
  CheckState GetCheckState() const;
  void SetCheckState(CheckState state);

  // Description:
  //  Called after a click has changed the state. A click turns an
  //  indeterminate check box on. The base implementation does nothing.
  virtual void OnCheckedChanged();

private:
  class CRhUserInterfaceCheckboxPrivate* m_ui_checkbox_private = nullptr;
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
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;

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

// Description:
//  A single line text box. Click it to give it keyboard focus, then type. The
//  characters go into the box instead of to the command line for as long as it
//  has focus; Enter commits what was typed, Escape puts back the text the box
//  started with, and both give the focus back to the viewport.
class RHINO_SDK_CLASS CRhinoUserInterfaceTextBox : public CRhinoUserInterfaceControl
{
public:
  CRhinoUserInterfaceTextBox();
  CRhinoUserInterfaceTextBox(const wchar_t* text);
  virtual ~CRhinoUserInterfaceTextBox();

  void Draw(CRhinoDisplayPipeline& dp) override;
  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseMove(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDoubleClick(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;

  bool OnCharacter(unsigned int character, unsigned int flags) override;
  bool OnKeyDown(unsigned int virtualKey, unsigned int flags) override;
  void OnKeyboardFocusChanged(bool hasFocus) override;
  bool AcceptsKeyboardFocus() const override;

  ON_2fSize ComputeSize() const override;
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;

  // Description:
  //  Give this text box keyboard focus, as clicking it does. The document is
  //  used to redraw and can be nullptr.
  void TakeKeyboardFocus(CRhinoDoc* doc);

  // Description:
  //  Text drawn, dimmed, when the box is empty and does not have focus.
  void SetPlaceholderText(const wchar_t* text);
  const ON_wString& PlaceholderText() const;

  // Description:
  //  Where the insertion point sits, as a count of characters from the start of
  //  the text. Setting it clamps to the length of the text, and drops whatever
  //  was selected: a caret on its own is a selection of nothing.
  int CaretPosition() const;
  void SetCaretPosition(int position);

  // Description:
  //  The run of text that is selected: where it starts and how many characters
  //  long it is. A length of zero means nothing is selected. Dragging across the
  //  text selects, as do shift and the arrow keys, Ctrl+A, and a double click on
  //  a word. Typing, or pasting, replaces what is selected.
  //
  //  SetSelection clamps to the text and leaves the caret at the end of the run.
  //  Both ends survive a text change no worse than the caret does: they are
  //  clamped when they are read.
  int SelectionStart() const;
  int SelectionLength() const;
  ON_wString SelectedText() const;
  void SetSelection(int start, int length);
  void SelectAll();
  void ClearSelection();

  // Description:
  //  A read only text box can take focus and be scrolled through, but typing
  //  does not change it. Off by default.
  void SetReadOnly(bool readOnly);
  bool ReadOnly() const;

  // Description:
  //  Most characters the text will hold. 0, the default, means no limit.
  void SetMaximumLength(int maximumLength);
  int MaximumLength() const;

  // Description:
  //  Clipboard. Ctrl+C, Ctrl+X and Ctrl+V call these while the box has keyboard
  //  focus. Copy and cut work on the selected run and do nothing when nothing is
  //  selected. Paste replaces the selection, or puts what is on the clipboard in
  //  at the caret when there is none, with anything that would break the line
  //  flattened to a space. Cut takes nothing out of a read only box, and paste
  //  puts nothing into one.
  // Returns:
  //  True if the clipboard or the text changed.
  bool CopyToClipboard() const;
  bool CutToClipboard();
  bool PasteFromClipboard();

  // Description:
  //  The box the text sits in: the color it is filled with, and the color of the
  //  outline around it. Both default to ON_UNSET_COLOR, which means use standard
  //  colors for the theme in use. The outline is drawn thicker and brighter while
  //  the box has keyboard focus, whatever color it is given.
  void SetBackgroundColor(const ON_Color& color);
  ON_Color BackgroundColor() const;
  void SetBorderColor(const ON_Color& color);
  ON_Color BorderColor() const;

  // Description:
  //  Color drawn behind the selected run of text. ON_UNSET_COLOR, the default,
  //  means use a standard highlight color for the theme in use.
  void SetSelectionColor(const ON_Color& color);
  ON_Color SelectionColor() const;

  // Description:
  //  Called every time the text changes, once per keystroke.
  virtual void OnTextChanged();

  // Description:
  //  Called when Enter is pressed, and when the box loses focus with the text
  //  changed from what it started with. This is where a caller does something
  //  about the new text.
  virtual void OnTextCommitted();

  // Description:
  //  Called when Escape is pressed. The text has already been put back to what
  //  it was when the box took focus.
  virtual void OnEditCanceled();
private:
  friend class CRhUserInterfaceTextBoxPrivate;

  // Moves the insertion point, either dragging the far end of the selection
  // along with it or dropping the selection and starting again from here
  void MoveCaretTo(int position, bool extendSelection);

  class CRhUserInterfaceTextBoxPrivate* m_ui_textbox_private = nullptr;
};

// Description:
//  A control that arranges child controls in a grid of rows and columns.
//  A grid shares ownership of the controls added to it: it holds a
//  std::shared_ptr to each one, so a control lives for as long as the grid or
//  anyone else is holding on to it.
//
//  Example, a two column grid with a fixed width label column:
//    CRhinoUserInterfaceGrid* grid = new CRhinoUserInterfaceGrid(2);
//    grid->SetColumnFixedWidth(0, 90);
//    grid->SetColumnFlexibleWidth(1);
//    grid->SetSize(260, 0); // 0 height means "as tall as the rows need"
//    grid->AddLabel(L"Radius");
//    grid->AddChild(radiusSlider); // radiusSlider is a shared_ptr
//    doc.AddUserInterfaceObject(grid, CommandId());
class RHINO_SDK_CLASS CRhinoUserInterfaceGrid : public CRhinoUserInterfaceControl
{
public:
  enum class ColumnSizing : int
  {
    // Column is exactly the width passed to SetColumnFixedWidth
    Fixed = 0,
    // Column is as wide as the widest ComputeSize() of the controls in it
    Auto = 1,
    // Column shares the width the fixed and auto columns leave over,
    // in proportion to the weight passed to SetColumnFlexibleWidth
    Flexible = 2,
  };

public:
  CRhinoUserInterfaceGrid();
  CRhinoUserInterfaceGrid(int columnCount);
  virtual ~CRhinoUserInterfaceGrid();

  // Description:
  //  Number of columns in the grid. The default is 1. Changing the column
  //  count reflows the existing children.
  int ColumnCount() const;
  void SetColumnCount(int count);

  // Description:
  //  Number of rows the current children occupy. This is computed from the
  //  child count, the column count and any column spans.
  int RowCount() const;

  // Description:
  //  Column width in logical pixels, never resized.
  void SetColumnFixedWidth(int column, float widthLP);

  // Description:
  //  Column is as wide as the widest control in it. Controls report their
  //  width from ComputeSize(), so a label column sized this way is exactly as
  //  wide as its longest description.
  void SetColumnAutoWidth(int column);

  // Description:
  //  Column shares the space the fixed and auto columns do not use. With one
  //  flexible column that column gets all of the remaining width; with more
  //  than one the remainder is divided in proportion to the weights.
  void SetColumnFlexibleWidth(int column, double weight);

  ColumnSizing GetColumnSizing(int column) const;

  // Description:
  //  Width a column will never go below, in logical pixels. Useful for
  //  flexible columns in a grid that can be narrow. The default is 0.
  void SetColumnMinimumWidth(int column, double minimumLP);
  double ColumnMinimumWidth(int column) const;

  // Description:
  //  Width of a column in logical pixels as of the most recent layout.
  //  Returns 0 before the grid has been laid out.
  double ColumnWidth(int column) const;

  // Description:
  //  How the controls in a column sit in their cells. The default is
  //  Stretch / Center, which makes a control fill the column width and center
  //  itself vertically in the row. Any horizontal setting other than Stretch
  //  gives the control the width it reports from ComputeSize().
  void SetColumnContentAlignment(int column, ControlHorizontalAlignment h, ControlVerticalAlignment v);
  void GetColumnContentAlignment(int column, ControlHorizontalAlignment& h, ControlVerticalAlignment& v) const;

  // Description:
  //  Height of every row in logical pixels. The default is 0 which means each
  //  row is as tall as the tallest control in it.
  void SetRowHeight(float heightLP);
  float RowHeight() const;

  // Description:
  //  Height of a row in logical pixels as of the most recent layout.
  //  Returns 0 before the grid has been laid out.
  float ComputedRowHeight(int row) const;

  // Description:
  //  Space between columns and between rows in logical pixels.
  //  The defaults are 6 and 2.
  void SetColumnSpacing(float spacingLP);
  float ColumnSpacing() const;
  void SetRowSpacing(float spacingLP);
  float RowSpacing() const;

  // Description:
  //  Space between the edges of the grid and its cells in logical pixels.
  //  The default is 6 on all four sides.
  void SetPadding(float paddingLP);
  void SetPadding(float left, float top, float right, float bottom);
  ON_4fRect Padding() const;

  // Description:
  //  Number of cells that have been filled, including empty cells added with
  //  AddEmptyCell. This is not the same as ColumnCount() * RowCount().
  int ChildCount() const;

  // Description:
  //  Append a control to the next cell. The grid keeps a reference to the
  //  control and removes it from the list of top level user interface objects
  //  if it was in that list.
  void AddChild(std::shared_ptr<CRhinoUserInterfaceControl> child);

  // Description:
  //  Append a cell with nothing in it. Use this to leave a hole in a row.
  void AddEmptyCell();

  // Description:
  //  Append a label to the next cell. This is the same as adding a
  //  CRhinoUserInterfaceLabel, which is what most first column cells are. The
  //  label is returned so that its text alignment, color or height can be
  //  adjusted.
  std::shared_ptr<CRhinoUserInterfaceLabel> AddLabel(const wchar_t* text);

  // Description:
  //  Append a label that spans every column, for a section heading. The
  //  heading starts a new row when the row being filled is partly full.
  std::shared_ptr<CRhinoUserInterfaceLabel> AddHeading(const wchar_t* text);

  // Description:
  //  Insert a control at a cell index, shifting the controls after it along.
  void InsertChild(std::shared_ptr<CRhinoUserInterfaceControl> child, int index);

  // Description:
  //  Take a control out of the grid. The grid hands its reference back, so the
  //  control stays alive as long as the caller holds on to it. The shared_ptr
  //  is empty for an empty cell or an out of range index.
  std::shared_ptr<CRhinoUserInterfaceControl> RemoveChild(int index);

  // Description:
  //  Empty the grid. The controls in it are deleted unless something else is
  //  still holding a reference to them.
  void RemoveAllChildren();

  // Description:
  //  Control in a cell. The shared_ptr is empty for an empty cell or an out of
  //  range index.
  std::shared_ptr<CRhinoUserInterfaceControl> Child(int index) const;
  std::shared_ptr<CRhinoUserInterfaceControl> Cell(int row, int column) const;

  // Description:
  //  Number of columns a cell occupies. The default is 1. Use this for a
  //  section heading or a control that runs the full width of the grid.
  //  A span is clamped to the number of columns left in the row.
  void SetChildColumnSpan(int index, int span);
  int ChildColumnSpan(int index) const;

  // Description:
  //  Color the grid fills its rectangle with. The default is ON_UNSET_COLOR
  //  for both, which means draw no background at all.
  void SetBackgroundColor(const ON_Color& color);
  ON_Color BackgroundColor() const;

  void SetBorderColor(const ON_Color& color);
  ON_Color BorderColor() const;
  void SetBorderThickness(float thicknessLP);
  float BorderThickness() const;
  void SetCornerRadius(float radiusLP);
  float CornerRadius() const;

  // Description:
  //  Whether a click where the grid has no child control belongs to the grid or
  //  to whatever is behind it. The default is false, so clicks on the padding,
  //  on the gaps between cells and on empty cells go through to the viewport,
  //  which is what a heads up display over a model wants. Set it to true for a
  //  grid that should behave like a solid panel and swallow everything inside
  //  its rectangle.
  void SetBackgroundClaimsMouse(bool on);
  bool BackgroundClaimsMouse() const;

  // Description:
  //  Draw a horizontal line between every pair of rows. Off by default.
  void EnableRowSeparators(bool on);
  bool RowSeparatorsEnabled() const;

  // Description:
  //  Draw a horizontal line under one row, for a heading row that wants a rule
  //  under it when a line between every row would be too much. Off by default.
  //  EnableRowSeparators(true) draws them all whatever this is set to.
  void EnableRowSeparatorBelowRow(int row, bool on);
  bool RowSeparatorBelowRow(int row) const;
  void SetRowSeparatorColor(const ON_Color& color);
  ON_Color RowSeparatorColor() const;

  void Draw(CRhinoDisplayPipeline& dp) override;
  bool IsMouseOver(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseEnter(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseLeave(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseMove(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDown(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseUp(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseDoubleClick(const CRhinoMouseEventArgs& mouse) override;
  void OnMouseWheel(const CRhinoMouseEventArgs& mouse, int delta) override;
  void OnTrackpadScroll(const CRhinoMouseEventArgs& mouse, float horizontal, float vertical) override;

  ON_2fSize ComputeSize() const override;
  ON_4fRect ComputedRect(const CRhinoDisplayPipeline* pipeline, bool logicalPixels) const override;

protected:
  void ContinueAnimate(CRhinoDoc* doc) override;

private:
  // Description:
  //  Mark the layout as out of date. The grid calls this itself when
  //  something it knows about changes. Call it after changing something it
  //  cannot see, such as the text of a label in an auto width column.
  void InvalidateLayout();

  // Description:
  //  Position and size every child. The grid does this itself before it draws
  //  and before it hit tests, so most callers never need to call it. Call it
  //  when you need a child's ComputedRect to be up to date outside of a draw.
  void LayoutCells();

  class CRhUserInterfaceGridPrivate* m_ui_grid_private = nullptr;
};

