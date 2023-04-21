#import "F3DNSDelegate.h"

// For some reason, this warning is emitted in the CI, so it needs to be disabled
// /Applications/Xcode_13.2.1.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX12.1.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSURLHandle.h:84:1: error: method has no return type specified; defaults to 'id' [-Werror,-Wmissing-method-return-type]
// - initWithURL:(NSURL *)anURL cached:(BOOL)willCache API_DEPRECATED("", macos(10.0, 10.4)) API_UNAVAILABLE(ios, watchos, tvos);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-method-return-type"
#import "AppKit/NSApplication.h"
#pragma clang diagnostic pop

#import "F3DStarter.h"

// This is a subclass of NSApplicationDelegate.
@interface F3DNSDelegateInternal : NSObject<NSApplicationDelegate>
@property F3DStarter* Starter;
@end

@implementation F3DNSDelegateInternal
@synthesize Starter;

// ----------------------------------------------------------------------------
- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
  (void)theApplication;
  Starter->AddFile([filename UTF8String]);
  Starter->LoadFile();
  Starter->Render();
  return YES;
}

@end

// ----------------------------------------------------------------------------
void F3DNSDelegate::InitializeDelegate(F3DStarter* Starter)
{
  F3DNSDelegateInternal* delegate = [F3DNSDelegateInternal alloc];
  [NSApplication sharedApplication];
  [NSApp setDelegate:delegate];
  [delegate setStarter:Starter];
}
