#ifndef f3d_macros_h
#define f3d_macros_h

/**
 * Cross-compiler warning silencing macros
 * e.g. to silence deprecation warning:
 *
 * @code{.cpp}
 * F3D_SILENT_WARNING_PUSH()
 * F3D_SILENT_WARNING_DECL(4996, "deprecated-declarations")
 * // deprecated code here
 * F3D_SILENT_WARNING_POP()
 * @endcode
 */
#if defined(_MSC_VER)
#define F3D_SILENT_WARNING_PUSH() __pragma(warning(push))
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING) __pragma(warning(disable : MSVC_CODE))
#define F3D_SILENT_WARNING_POP() __pragma(warning(pop))
#elif defined(__clang__)
#define F3D_SILENT_WARNING_PUSH() _Pragma("clang diagnostic push")
#define F3D_INTERNAL_DO_PRAGMA(x) _Pragma(#x)
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING)                                             \
  F3D_INTERNAL_DO_PRAGMA(clang diagnostic ignored "-W" GNU_STRING)
#define F3D_SILENT_WARNING_POP() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define F3D_SILENT_WARNING_PUSH() _Pragma("GCC diagnostic push")
#define F3D_INTERNAL_DO_PRAGMA(x) _Pragma(#x)
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING)                                             \
  F3D_INTERNAL_DO_PRAGMA(GCC diagnostic ignored "-W" GNU_STRING)
#define F3D_SILENT_WARNING_POP() _Pragma("GCC diagnostic pop")
#else
#define F3D_SILENT_WARNING_PUSH()
#define F3D_SILENT_WARNING_DECL(MSVC_CODE, GNU_STRING)
#define F3D_SILENT_WARNING_POP()
#endif

#endif
