
#pragma once // Public SDK header

#include "RhRdkDefinitions.h"

class IRhRdk_XMLSection;
class CRhRdkColor;

RDK_DEPRECATED typedef DWORD RDK_DECALID;  // Please use ON_DECAL_CRC instead if possible.

#if (defined _DEBUG) && (defined HUMAN_READABLE_DECAL_CRC)
using RDK_DECAL_CRC = ON_DECAL_CRC;
#else
typedef DWORD RDK_DECAL_CRC;               // Please use ON_DECAL_CRC instead if possible.
#endif

#define RDK_NIL_DECAL_CRC RDK_DECAL_CRC(0) // Please use ON_NIL_DECAL_CRC instead if possible.

// IRhRdkDecal is deprecated. You should use ON_Decal and CRhRdkDocumentAwareDecal instead.
RDK_DEPRECATED_CLASS class RHRDK_SDK IRhRdkDecal
{
public:
	virtual ~IRhRdkDecal() { }
	enum eMapping { mapUnset =-1, mapPlanar = 0, mapCylindrical = 1, mapSpherical = 2, mapUV = 3, mapForce32bit = 0xFFFFFFFF };
	enum eProjection { projUnset =-1, projForward = 0, projBackward = 1, projBoth = 2, projForce32bit = 0xFFFFFFFF };
	RDK_DEPRECATED virtual RDK_DECAL_CRC Id(void) const { return CRC(); }
	virtual RDK_DECAL_CRC CRC(void) const = 0;
	virtual UUID TextureInstanceId(void) const;
	virtual const CRhRdkTexture* Texture(void) const = 0;
	virtual void SetTexture(const CRhRdkTexture* pTexture) = 0;
	virtual eMapping Mapping(void) const = 0;
	virtual eProjection Projection(void) const = 0;
	virtual bool MapToInside(void) const = 0;
	virtual double Transparency(void) const = 0;
	virtual ON_3dPoint Origin(void) const = 0;
	virtual ON_3dVector VectorUp(void) const = 0;
	virtual ON_3dVector VectorAcross(void) const = 0;
	virtual double Height(void) const = 0;
	virtual double Radius(void) const = 0;
	RDK_DEPRECATED virtual double LatStart(void) const;
	RDK_DEPRECATED virtual double LatEnd(void) const;
	RDK_DEPRECATED virtual double LonStart(void) const;
	RDK_DEPRECATED virtual double LonEnd(void) const;
	virtual void GetHorzSweep(double& sta, double& end) const = 0;
	virtual void GetVertSweep(double& sta, double& end) const = 0;
	virtual void UVBounds(double& dMinUOut, double& dMinVOut, double& dMaxUOut, double& dMaxVOut) const = 0;
	virtual const IRhRdk_XMLSection* CustomData(void) const = 0;
	virtual bool Color(const ON_3dPoint& ptPoint, const ON_3dVector& vecNormal, CRhRdkColor& colInOut, ON_2dPoint& uvOut) const = 0;
	virtual void* EVF(const wchar_t*, void*) = 0;
	static bool GetTextureMapping(const IRhRdkDecal& decal, ON_TextureMapping& mappingOut);
};

RDK_DEPRECATED_CLASS class RHRDK_SDK IRhRdkDecal2 : public IRhRdkDecal
{
public:
	//This is mainly for decal access from the change queue
	virtual ON__UINT32 TextureRenderCRC(const CRhRdkContent::CRenderCRCFlags& rcrcFlags) const = 0;
	virtual ON__UINT32 TextureRenderCRC(const CRhRdkContent::CRenderCRCFlags& rcrcFlags, const IRhRdkLinearWorkflow& lw) const = 0;
};

RHRDK_SDK unsigned int RhRdkGetTextureRenderCRC(const CRhinoDoc& doc, const ON_Decal& decal, ON__UINT64 rcf, const ON_LinearWorkflow* lw);
RHRDK_SDK bool RhRdkGetDecalColor(const CRhinoDoc& doc, const ON_Decal& decal, const ON_3dPoint& ptPoint, const ON_3dVector& vecNormal, ON_4fColor& colInOut, ON_2dPoint& uvOut);
