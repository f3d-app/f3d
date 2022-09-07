#import "F3DNSDelegate.h"

#import "Cocoa/Cocoa.h"

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
