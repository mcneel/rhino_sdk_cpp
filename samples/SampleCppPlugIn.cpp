//
//  SampleCppPlugIn.cpp
//
//  A minimal Rhino C++ plug-in.  These two files are cross-platform: the
//  same source builds on macOS and on Windows, with Xcode, Visual Studio
//  or CMake.  See the README for the per-platform project setup.
//

#include "SampleCppPlugIn.h"

// Declares the RHINO_PLUG_IN_* macros used below.  Include it in exactly
// one .cpp file of your plug-in - the one holding the declaration block.
// If you forget it, RHINO_PLUG_IN_DECLARE expands to an #error that says
// so; that is deliberate, not a broken header.
#include "SDK/inc/rhinoSdkPlugInDeclare.h"

// The plug-in object below is a file-scope static, and so are any command
// objects.  Rhino requires the plug-in to be constructed first.  On MSVC,
// init_seg(lib) moves this file's static initialisers into an earlier
// segment to guarantee that; warning 4073 is the compiler noting that a
// non-standard initialisation order was requested, which is exactly what
// is wanted here.  Clang orders these correctly without help.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)
#endif

// ---------------------------------------------------------------------
// Plug-in declaration
//
// These macros export plain C functions that Rhino calls *before* it
// loads the plug-in, to decide whether it can.  They are not optional:
// without RHINO_PLUG_IN_DECLARE the plug-in has no SDK version stamped
// into it, and Rhino for Windows refuses it outright with
//
//     Unable to load ... : Rhino version not specified.
//
// RHINO_PLUG_IN_DECLARE is the one that matters for loading - it exports
// the SDK version and service release this plug-in was compiled against
// (and, on Windows, the MFC and compiler versions).  Rhino compares them
// with its own and declines to load a plug-in built against an SDK newer
// than itself, so keep the SDK submodule and your installed Rhino in
// step.
//
// The rest supply the metadata shown in Options > Plug-ins.  Fill them in
// with your own details.  RHINO_PLUG_IN_ID must be the same id that your
// CRhinoPlugIn::PlugInID() returns below, or Rhino will load the file and
// then fail to match it to the plug-in object inside.
// ---------------------------------------------------------------------

RHINO_PLUG_IN_DECLARE

RHINO_PLUG_IN_NAME(L"SampleCppPlugIn");
RHINO_PLUG_IN_ID(L"FC563BB1-C1D1-4529-8E7E-7B229D6F5AA4");
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)
RHINO_PLUG_IN_DESCRIPTION(L"Rhino C++ SDK sample plug-in");

RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Robert McNeel & Associates");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"146 North Canal Street, Suite 320, Seattle WA 98103");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"United States");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"206-545-6877");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"devsupport@mcneel.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"https://www.rhino3d.com");
RHINO_PLUG_IN_UPDATE_URL(L"https://github.com/mcneel/rhino_sdk_cpp");

// Not used here, and both are worth knowing about:
//
//   RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON1)
//     The icon shown in Options > Plug-ins.  It takes a Windows resource
//     id, so it has no meaning in a Mac-only or cross-platform target -
//     wrap it in #if defined(ON_RUNTIME_WIN) if you want one.
//
//   RHINO_PLUG_IN_DEPENDENCIES(L"<id>;<id>")
//     Ids of other plug-ins that must be loaded before this one.

// ---------------------------------------------------------------------
// The plug-in object
// ---------------------------------------------------------------------

class CSampleCppPlugIn : public CRhinoUtilityPlugIn
{
    GUID PlugInID() const override
    {
        // Must match RHINO_PLUG_IN_ID above.  Generate your own.
        static const GUID guid =
        { 0xfc563bb1, 0xc1d1, 0x4529, { 0x8e, 0x7e, 0x7b, 0x22, 0x9d, 0x6f, 0x5a, 0xa4 } };
        return guid;
    }

    const wchar_t *PlugInName() const override
    {
        return L"SampleCppPlugIn";
    }

    const wchar_t *PlugInVersion() const override
    {
        return L"1.0.0";
    }

    int OnLoadPlugIn(void) override
    {
        RhinoApp().Print(L"This is coming from a third party C++ plug-in\n");

        return 1;
    }
};

static CSampleCppPlugIn my_plug_in;

// ---------------------------------------------------------------------
// A command
// ---------------------------------------------------------------------

static class CSampleCppCommand : public CRhinoCommand
{
    UUID CommandUUID() override
    {
        static const GUID guid =
        { 0xac563bb1, 0xa1d1, 0x4529, { 0x8e, 0x7e, 0x7b, 0x22, 0xad, 0x6f, 0x5a, 0xa4 } };
        return guid;
    }

    const wchar_t *EnglishCommandName() override
    {
        return L"SampleCppCommand";
    }

    CRhinoCommand::result RunCommand(const CRhinoCommandContext& context) override
    {
        const ON_Line line(ON_origin, ON_3dPoint(4.0, 5.0, 6.0));

        context.m_doc.AddCurveObject(line);

        return success;
    }

} mycommand;
