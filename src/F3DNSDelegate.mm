#import "F3DNSDelegate.h"

#import "Cocoa/Cocoa.h"

#import "F3DLoader.h"
#import "vtkRenderWindow.h"

// This is a subclass of NSApplicationDelegate.
@interface F3DNSDelegateInternal : NSObject<NSApplicationDelegate>
@property F3DLoader* Loader;
@property vtkRenderWindow* RenderWindow;
@end

@implementation F3DNSDelegateInternal
@synthesize Loader;
@synthesize RenderWindow;

// ----------------------------------------------------------------------------
- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
  (void)theApplication;
  Loader->AddFile([filename UTF8String]);
  Loader->LoadFile();
  RenderWindow->Render();
  return YES;
}

@end

// ----------------------------------------------------------------------------
void F3DNSDelegate::InitializeDelegate(F3DLoader* loader, vtkRenderWindow* renWin)
{
  F3DNSDelegateInternal* delegate = [F3DNSDelegateInternal alloc];
  [NSApplication sharedApplication];
  [NSApp setDelegate:delegate];
  [delegate setLoader:loader];
  [delegate setRenderWindow:renWin];
}
