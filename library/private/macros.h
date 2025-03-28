#ifndef f3d_macros_h
#define f3d_macros_h

/**
 * Cross-compiler warning silencing macros
 */
#if defined(_MSC_VER)
#define F3D_SILENT_WARNING_PUSH() __pragma(warning(push))
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING) __pragma(warning(disable : MSVC_CODE))
#define F3D_SILENT_WARNING_POP() __pragma(warning(pop))
#elif defined(__clang__)
#define F3D_SILENT_WARNING_PUSH() _Pragma("clang diagnostic push")
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING)                                             \
  _Pragma("clang diagnostic ignored \"-W" GNU_STRING "\"")
#define F3D_SILENT_WARNING_POP() __pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define F3D_SILENT_WARNING_PUSH() _Pragma("GCC diagnostic push")
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING)                                             \
  _Pragma("GCC diagnostic ignored \"-W" GNU_STRING "\"")
#define F3D_SILENT_WARNING_POP() __pragma("GCC diagnostic pop")
#else
#define F3D_SILENT_WARNING_PUSH()
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING)
#define F3D_SILENT_WARNING_POP()
#endif

#endif
