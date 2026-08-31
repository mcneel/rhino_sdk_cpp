//
//  SampleCppPlugIn.h
//
//  A minimal Rhino C++ plug-in.  These two files are cross-platform: the
//  same source builds on macOS and on Windows, with Xcode, Visual Studio
//  or CMake.  See the README for the per-platform project setup.
//

#pragma once

// The SDK preamble has to be compiled before anything else in every
// translation unit, and rhinoSdk.h after the platform headers.
//
// On macOS these four includes are the whole story, which is why they
// live here in the header.
//
// On Windows they are not enough on their own, because MFC and the
// Windows headers have to be compiled *between* the preamble and
// rhinoSdk.h.  The Windows project handles that by force-including a
// stdafx.h that pulls all of it in, in the right order, ahead of this
// file; by the time the compiler reaches these lines they are already
// satisfied and do nothing.  They are kept here so this header remains
// self-contained on the Mac and self-describing everywhere.

#include "SDK/inc/rhinoSdkStdafxPreamble.h"
#include "SDK/inc/rhinoSdk.h"
#include "SDK/inc/RhRdkHeaders.h"
#include "SDK/inc/rhinoSdkChecks.h"
