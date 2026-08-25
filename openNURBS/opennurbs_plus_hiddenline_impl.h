
#if !defined( ON_HIDDENLINEIMPL_INC_ )
#define ON_HIDDENLINEIMPL_INC_

///////////////////////////////////////
/***  These declarations define the interface ON_HiddenLineDrawingImplBase .**/
///////////////////////////////////////


class ON_CLASS ON_HiddenLineDrawingImplBase
{
public:
	virtual ~ON_HiddenLineDrawingImplBase() = default;

	virtual ON_HiddenLineDrawingImplBase* DuplicateHere(
		ON_SimpleArray<  ON_HLDFullCurve*>&,
		ON_SimpleArray<  ON_HLDCurve*>&,
		ON_SimpleArray< const ON_HLD_Object*>& )=0;
	
	virtual bool SetAbsoluteTolerance(double absoluteTolerance) = 0;
	virtual double AbsoluteTolerance() const= 0;
	virtual bool IsValid(ON_TextLog* text_log = nullptr) const = 0;
	virtual void Dump(ON_TextLog&) const = 0;						// for debugging

	// See TL_HiddenLineDrawing::SetViewport(...)
	virtual bool SetViewport(const ON_Viewport& v) = 0;
	virtual const ON_Viewport& Viewport() const = 0;

	// See TL_HiddenLineDrawing::AddClippingPlane(...)
	virtual bool AddClippingPlane(ON_PlaneEquation Clip, ON__UINT_PTR id ) = 0;
	virtual const ON_SimpleArray<ON_PlaneEquation>& GetClippingPlanes( ON_SimpleArray< ON__UINT_PTR>* clip_id) const = 0;

  virtual void EnableOccludingSection(bool bEnabble)=0;
  virtual bool OccludingSectionOption() const = 0;

	// See TL_HiddenLineDrawing::IncludeTangentEdges(...)
	virtual void IncludeTangentEdges(bool include) = 0;
	virtual bool  GetIncludeTangentEdges() const = 0;

	// See TL_HiddenLineDrawing::IncludeTangentSeams(...)
	virtual bool IncludeTangentSeams(bool include) = 0;
	virtual bool GetIncludeTangentSeams() const  = 0;

	// See TL_HiddenLineDrawing::IncludeHiddenCurves(...)
	virtual bool IncludeHiddenCurves(bool include) = 0;
	virtual bool GetIncludeHidden() const = 0;

	// See TL_HiddenLineDrawing::SetContext(...)
	virtual void SetContext(const ON_HiddenLineDrawingImplBase& Source) = 0;
	virtual void SetContextToTL(class TL_HiddenLineDrawingImpl& Destination) const = 0;

	virtual int AddObject(const ON_Geometry* geom, const ON_Xform* xform, ON_UUID uuid, ON__UINT_PTR m_id) = 0;
	virtual int AddObject(std::shared_ptr<const ON_Geometry> shr_geom_ptr, const ON_Xform* xform, ON_UUID uuid, ON__UINT_PTR m_id) = 0;
	virtual int AddObject(class TL_HLDObject* hld_obj) = 0;


  virtual bool EnableSelectiveClipping(ON_HLD_Object& obj, bool bEnable, const ON_SimpleArray< ON__UINT_PTR>* active_clip_ids) = 0;

  // Note: These functions have implementations so this class and
  // any derived class can access ON_HLD_Object members
  void AddSelectiveClippingPlane(ON_HLD_Object& obj, bool bActive);
  void SetSelectiveClipping( ON_HLD_Object& obj, bool bEnable) ;

  virtual bool SelectiveClippingOption(const ON_HLD_Object& obj) const;
  virtual ON_SimpleArray<ON__UINT_PTR> ActiveClipIds(const ON_HLD_Object& obj) const=0;

	virtual bool Draw(bool bAllowUseMP,
		ON_ProgressReporter* progress,
		ON_Terminator* terminator) = 0;

	virtual bool  Merge(
		ON_HiddenLineDrawingImplBase& OtherHLD,
		ON_Terminator* terminator) = 0;

	virtual const ON_SimpleArray< const class ON_HLDPoint*>& GetHLDPoints()=0;

	// See TL_HiddenLineDrawing::Flatten(...)
	virtual bool Flatten() = 0;
	virtual bool HasBeenFlattened() const = 0;

	virtual bool CompactCurves() = 0;	// remove the NULLs from m_Crv and adjust crv.ci fields

	// Get tight bounding box of the hidden line drawing
	virtual ON_BoundingBox GetBoundingBox(bool IncludeHidden) = 0;

	// Note: If we are not computing topology no need for these classes
	virtual bool HasTopology() const = 0;	// true if m_V an

	virtual const ON_Xform& World2HiddenLine() const=0;			// World to HLD-coordinate System.  
	virtual const ON_Xform& HiddenLine2World() const=0;			// HLD to World coordinate System.  
	virtual ON_3dVector CameraDirection(ON_3dPoint wp)const=0;	// world camera direction ( increasing toward scene) at a world point
	virtual ON_3dVector CamLocDir() const=0;								//returns camera location, if perspective, or else camera direction.

	virtual bool IsPerspective() const=0;
	virtual bool IsFullCurveValid(const ON_HLDFullCurve& FC, ON_TextLog*log) const;
	virtual int NumberDrawn() const = 0;

  virtual void RejoinCompatibleVisible() = 0;

	virtual void EmergencyDestroy() = 0;
};


#endif 


									
