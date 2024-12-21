/////////////////////////////////////////////////////////////////////////////
// Define user-provided macros
/////////////////////////////////////////////////////////////////////////////
#ifndef wxNO_UNSAFE_WXSTRING_CONV
	#define wxNO_UNSAFE_WXSTRING_CONV 1
#endif

// Since v3.1.4 wxWidgets is using this macro to switch class access modifiers
// depending on whether it's built as a static or dynamic library. We need to
// always define it as otherwise it's causing linker errors due to name mangling.
#ifndef WXBUILDING
	#define WXBUILDING
#endif
