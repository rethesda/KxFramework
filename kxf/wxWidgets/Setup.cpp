#include "kxf-pch.h"
#include "Setup.h"
#include "Setup-IncludeBasic.h"

/////////////////////////////////////////////////////////////////////////////
// Options for wxWidgets that are required for kxf-based applications
//
// Refer to the original 'setup.h' file in the wxWidgets distribution for 
// options description.
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// Compatibility settings
// ----------------------------------------------------------------------------

// Required because wxWidgets uses old CRT functions
static_assert(_CRT_SECURE_NO_DEPRECATE == 1);
static_assert(_CRT_SECURE_NO_WARNINGS == 1);
static_assert(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS == 1);

// None of the options below are strictly required
//static_assert(WXWIN_COMPATIBILITY_2_8 == 0);
//static_assert(WXWIN_COMPATIBILITY_3_0 == 0);
//static_assert(wxDIALOG_UNIT_COMPATIBILITY == 0);
//static_assert(wxUSE_UNSAFE_WXSTRING_CONV == 0);
//static_assert(wxNO_UNSAFE_WXSTRING_CONV == 1);

// ----------------------------------------------------------------------------
// Common options
// ----------------------------------------------------------------------------
static_assert(wxUSE_GUI == 1);
static_assert(wxUSE_UNICODE == 1);
static_assert(wxUSE_UNICODE_WCHAR == 1);

// ----------------------------------------------------------------------------
// Interoperability with the standard library
// ----------------------------------------------------------------------------
static_assert(wxUSE_STL == 1);
static_assert(wxUSE_STD_DEFAULT == 1);
static_assert(wxUSE_STD_CONTAINERS_COMPATIBLY == wxUSE_STD_DEFAULT);
static_assert(wxUSE_STD_CONTAINERS == wxUSE_STD_DEFAULT);
static_assert(wxUSE_STD_IOSTREAM == wxUSE_STD_DEFAULT);
static_assert(wxUSE_STD_STRING == wxUSE_STD_DEFAULT);
static_assert(wxUSE_STD_STRING_CONV_IN_WXSTRING == wxUSE_STL);

// ----------------------------------------------------------------------------
// Non GUI features selection
// ----------------------------------------------------------------------------
static_assert(wxUSE_INTL == 1);
static_assert(wxUSE_IPV6 == 1);

// ----------------------------------------------------------------------------
// Windows-only settings
// ----------------------------------------------------------------------------
static_assert(wxUSE_WINSOCK2 == 1);
