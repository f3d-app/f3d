#include "context_cocoa.h"

#import <Cocoa/Cocoa.h>

namespace f3d::detail
{
//----------------------------------------------------------------------------
void* getCocoaOpenGLSymbol(const char* name)
{
  CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     name,
                                                     kCFStringEncodingASCII);

  void* symbol = CFBundleGetFunctionPointerForName(CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl")),
                                                   symbolName);

  CFRelease(symbolName);

  return symbol;
}
}
