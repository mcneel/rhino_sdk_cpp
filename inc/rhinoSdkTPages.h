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

#if defined(ON_RUNTIME_WIN) && defined(_MFC_VER)
#include "rhinoSdkTMfcPages.h"
#endif

#if defined(ON_RUNTIME_APPLE)
#if !defined RHINO_THIRD_PARTY_OSX_PLUGIN_COMPILE
#include "rhinoSdkTMacPages.h"
#endif
#endif
