
#pragma once // Public SDK header

#include "RhRdkObject.h"
#include "IRhRdkDecal.h"

class CRhRdkMaterial;

/** \class CRhRdkObjectAncestry

	This class describes the parent chain for an object so that the "By Parent" material source can be properly implemented.
	When a reference to this class is required, and the object is inside a block or worksession, supply the full parent chain starting
	from the object that is in the document itself (usually the block reference), followed by the nested block reference or concrete
	geometry object...until you reach the object itself.

*/
class RHRDK_SDK CRhRdkObjectAncestry : public CRhRdkObject
{
public:
	CRhRdkObjectAncestry();
	CRhRdkObjectAncestry(const CRhRdkObjectAncestry& src);
	CRhRdkObjectAncestry(const CRhRdkObjectAncestry* pSrc);
	CRhRdkObjectAncestry(const ON_SimpleArray<const CRhinoInstanceObject*>& a);
	virtual ~CRhRdkObjectAncestry();

	static const CRhRdkObjectAncestry empty;

	const CRhRdkObjectAncestry& operator=(const CRhRdkObjectAncestry&);

	const CRhRdkObjectAncestry& operator += (const CRhinoInstanceObject* pObject);

	const CRhinoInstanceObject* TopLevelParent(void) const;

	bool IsTopLevel(void) const;

	const ON_SimpleArray<const CRhinoInstanceObject*>& Ancestry(void) const;

	ON_Xform CompleteTransform(void) const;

private:
	ON_SimpleArray<const CRhinoInstanceObject*> m_Objects;
	void* m_pReserved;
};

/** \class CRhRdkDocumentAwareDecal

	This class associates a document with a decal and provides access to
	various things that need a document in order to be computed.

*/
class RHRDK_SDK CRhRdkDocumentAwareDecal : public ON_Decal
{
public:
	CRhRdkDocumentAwareDecal();
	CRhRdkDocumentAwareDecal(unsigned int doc_sn, const ON_Decal& decal);

	/** \return The serial number of the document the decal is associated with. */
	unsigned int DocSerialNumber(void) const;

	/** \return a pointer to the texture the decal is using. */
	const CRhRdkTexture* Texture(void) const;

	/** \return The texture projection mode the decal is using. */
	CRhRdkTexture::ProjectionModes TextureProjectionMode(void) const;

	/** Blend color with the decal color at a given point.
		Checks if the decal has color on a given point with a given face normal. If it has, then 'colInOut' is
		blended with that color and the function returns true. IMPORTANT: If the mapping type is UV, 'point'
		must be the texture coordinates of the desired point on the object carrying the decal.
		\param point is the point in space or, if the decal is uv-mapped, the uv-coordinate of that point.
		\param normal is the face normal of the given point.
		\param colInOut is the color to blend to the decal color.
		\param uvOut is the UV on the texture that the color point was read from.
		\return \e true if the given point hits the decal, else \e false. */
	bool Color(const ON_3dPoint& point, const ON_3dVector& normal,
	           IN OUT CRhRdkColor& col_in_out, OUT ON_2dPoint& uv_out) const;

protected:
	virtual const ON_Decal& operator = (const ON_Decal& d) override { return ON_Decal::operator =  (d); }
	virtual bool operator == (const ON_Decal& d)     const override { return ON_Decal::operator == (d); }
	virtual bool operator != (const ON_Decal& d)     const override { return ON_Decal::operator != (d); }

private:
	class CImpl;
	CImpl* _impl;
};

class RHRDK_SDK CRhRdkDecalIterator : public CRhRdkObject
{
public:
	virtual ~CRhRdkDecalIterator() { }

	/** Reset the iterator to get the first decal again. */
	virtual void Reset(void) = 0;

	/** Retrieve the next decal.
		\return A pointer to the next decal or null if no more decals. */
	RDK_DEPRECATED virtual const IRhRdkDecal* NextDecal(void) = 0;

	/** \return A CRC representing the combined, order-dependent CRC of this iterator's decals. */
	virtual RDK_DECAL_CRC CRC(void) const = 0; // [SDK_UNFREEZE] ON_DECAL_CRC
};

/** \class CRhRdkDecalIteratorEx

	This class provides a means of iterating over decals. An instance of this object can be obtained
	by calling CRhRdkObjectDataAccess::NewDecalIterator(). The implementation of this class is thread-safe
	so that it can be used from multiple threads during rendering.

*/
class RHRDK_SDK CRhRdkDecalIteratorEx : public CRhRdkDecalIterator
{
public:
	/** Retrieve the next decal. Supersedes the one above that returns IRhRdkDecal.
		\return A pointer to the next decal or null if no more decals. */
	virtual const CRhRdkDocumentAwareDecal* NextDecalEx(void) = 0;
};

/** \class CRhRdkObjectDataAccess

	This class provides a means of accessing data associated with a Rhino object or layer.
*/
class RHRDK_SDK CRhRdkObjectDataAccess
{
public:
	virtual ~CRhRdkObjectDataAccess();

	/** Construct to access data of Rhino object by object uuid. */
	CRhRdkObjectDataAccess(const CRhinoDoc& doc, const UUID& uuidRhinoObject);

	/** Deprecated; not document-aware. */
	RDK_DEPRECATED CRhRdkObjectDataAccess(const UUID& uuidRhinoObject);

	/** Construct to access data of Rhino object by object pointer. */
	CRhRdkObjectDataAccess(const CRhinoObject* pRhinoObject);

	/** Construct to access data of Rhino layer by layer pointer. */
	CRhRdkObjectDataAccess(const CRhinoLayer* pRhinoLayer);

	//-------- Material. Works only for object.
	
	/** \return the material associated with this object, depending on the material source.
		\note Currently, 'By Parent' is not implemented. */
	virtual UUID ObjectMaterial(const CRhRdkObjectAncestry*) const;

	//-------- Material. Works for object or layer.

	/** \return Instance id of RDK material associated with the object. 
		Internal use - consider using ObjectMaterial. */
	virtual UUID MaterialInstanceId(void) const;

	/** Set an RDK material to be associated with the object or layer.
		\param uuidMaterialInstance is the instance id of the material.
		\return \e true if successful, else \e false. */
	virtual bool SetMaterialInstanceId(const UUID& uuidMaterialInstance) const;

	/** Set an RDK material to be associated with the object or layer.
		\param pMaterial is a pointer to the material.
		\return \e true if successful, else \e false. */
	virtual bool SetMaterialInstanceId(const CRhRdkMaterial* pMaterial) const;

	/** Set an RDK material to be associated with the object.
		\param pMaterial is a pointer to the material.
		\param bClearPerFace if \e true will cause per-face materials to be cleared.
		 Otherwise they will be retained. Only makes sense for objects, not layers.
		\return \e true if successful, else \e false. */
	bool SetMaterialInstanceId(const CRhRdkMaterial* pMaterial, bool bClearPerFace) const;

	/** By default, content instance ids are verified as existing in the document.
		In certain cases this verification may not be required. It can be disabled
		and re-enabled by calling this method. */
	void SetVerifyInstanceId(bool b);

	//-------- Mapping.

	/** Get an array of used mapping channel ids.
		\param arrayOut is filled with 1-based channel identifiers.
		\return \e true if successful, else \e false. */
	virtual bool GetUsedMappingChannels(ON_SimpleArray<int>& arrayOut) const;

	/** Get the mapping and local xform for a channel.
		\param iChannelId is the channel identifier (1-based).
		\param mappingOut receives a copy of the mapping.
		\param xformLocalOut receives the local transform associated with the channel.
		\return \e true if successful, else \e false. */
	virtual bool Mapping(int iChannelId, ON_TextureMapping& mappingOut, ON_Xform& xformLocalOut) const;

	//-------- Decals. Works only for object.

	/** This method is deprecated in favor of NewDecalIteratorEx. */
	RDK_DEPRECATED virtual CRhRdkDecalIterator* NewDecalIterator(const UUID& uuidRenderEngine = RhinoApp().GetDefaultRenderApp()) const;

	/** \return An iterator for accessing decals or null if this object is not connected to a valid Rhino object.
		\param visible_only is true to only return visible decals, false to return all decals. Render plug-ins
		 should pass true for this parameter.
		\param render_engine_id is the render engine id of the renderer. The default is the current renderer which
		 should be sufficient for render plug-ins.
		The caller shall delete the iterator. */
	virtual_su CRhRdkDecalIteratorEx* NewDecalIteratorEx(bool visible_only, const ON_UUID& render_engine_id = RhinoApp().GetDefaultRenderApp()) const;

	/** Remove all decals from the object.
		\return \e true if this object is connected to a valid Rhino object, else \e false. */
	virtual bool RemoveAllDecals(void) const;

	/** Remove the specified decal from the object.
		\return \e true if the decal was removed, \e false if the decal does not exist,
		 or if this object is not connected to a valid Rhino object. */
	virtual bool RemoveDecal(RDK_DECAL_CRC id) const;

	/** This method is deprecated in favor of the one below. */
	RDK_DEPRECATED RDK_DECAL_CRC AddDecal(const IRhRdkDecal& decal) const;

	/** Add a new decal to the object. */
	ON_DecalObjectAttributesWrapper::AddDecalResults AddDecal(const ON_Decal& decal) const;

	/** Emergency virtual function for future expansion. */
	virtual void* EVF(const wchar_t* wszFunc, void* pvData);

private:
	/** \internal For RDK internal use only. */
	class CImpl;
	CImpl* m_pImpl;
};

//###############################################################################################################
// These functions are deprecated because the equivalent functionality is now in ON_DecalObjectAttributesWrapper.

// These are still implemented as calls into ON_DecalObjectAttributesWrapper.
RDK_DEPRECATED RHRDK_SDK RDK_DECAL_CRC RhRdkAddDecalToObjectAttributes(ON_3dmObjectAttributes&, const IRhRdkDecal&);
RDK_DEPRECATED RHRDK_SDK RDK_DECAL_CRC RhRdkAddDecalToObjectAttributes(ON_3dmObjectAttributes&, const ON_Decal&);
RDK_DEPRECATED RHRDK_SDK bool     RhRdkRemoveDecalFromObjectAttributes(ON_3dmObjectAttributes&, RDK_DECAL_CRC);
RDK_DEPRECATED RHRDK_SDK void RhRdkRemoveAllDecalsFromObjectAttributes(ON_3dmObjectAttributes&);
RDK_DEPRECATED RHRDK_SDK ON_Decal*   RhRdkNewDecalFromObjectAttributes(ON_3dmObjectAttributes&, RDK_DECAL_CRC);

// Use ON_DecalObjectAttributesWrapper::NewDecalFromObjectAttributes() instead.
RDK_DEPRECATED RHRDK_SDK IRhRdkDecal* RhRdkFindDecalOnObjectAttributes(ON_3dmObjectAttributes&, RDK_DECAL_CRC);

RDK_NOT_IMPL // Use ON_3dmObjectAttributes::GetDecalArray() instead.
RDK_DEPRECATED RHRDK_SDK CRhRdkDecalIterator* RhRdkNewDecalIteratorForObjectAttributes(const ON_3dmObjectAttributes&);
//###############################################################################################################
