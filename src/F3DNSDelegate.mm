#import "F3DNSDelegate.h"

#import "Cocoa/Cocoa.h"

#import "vtkF3DRenderer.h"
#import "F3DLoader.h"

// This is a subclass of NSApplicationDelegate.
@interface F3DNSDelegateInternal : NSObject<NSApplicationDelegate>
@property vtkF3DRenderer* Renderer;
@end

@implementation F3DNSDelegateInternal
@synthesize Renderer;

// ----------------------------------------------------------------------------
- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
  F3DLoader& loader = F3DLoader::GetInstance();
  loader.AddFile([filename UTF8String]);
  loader.LoadCurrentIndex(Renderer);
  return YES;
}

@end

// ----------------------------------------------------------------------------
void F3DNSDelegate::InitializeDelegate(vtkF3DRenderer* ren)
{
  F3DNSDelegateInternal* delegate = [F3DNSDelegateInternal alloc];
  [NSApplication sharedApplication];
  [NSApp setDelegate:delegate];
  [delegate setRenderer:ren];
}
