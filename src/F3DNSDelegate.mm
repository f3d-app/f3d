#import "F3DNSDelegate.h"

#import "Cocoa/Cocoa.h"

#import "F3DLoader.h"

// This is a subclass of NSApplicationDelegate.
@interface F3DNSDelegateInternal : NSObject<NSApplicationDelegate>
@property F3DLoader* Loader;
@end

@implementation F3DNSDelegateInternal
@synthesize Loader;

// ----------------------------------------------------------------------------
- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
  Loader->AddFile([filename UTF8String]);
  Loader->LoadFile();
  return YES;
}

@end

// ----------------------------------------------------------------------------
void F3DNSDelegate::InitializeDelegate(F3DLoader* loader)
{
  F3DNSDelegateInternal* delegate = [F3DNSDelegateInternal alloc];
  [NSApplication sharedApplication];
  [NSApp setDelegate:delegate];
  [delegate setLoader:loader];
}
