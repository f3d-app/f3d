#include "engine.h"

#include "TestSDKHelpers.h"

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

std::unique_ptr<f3d::engine> engine;

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@property (strong) NSOpenGLContext *openGLContext;
@property (strong) NSTimer *renderTimer;
@property std::string dataPath;
@property std::string outPath;
@property f3d::image result;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  NSRect frame = NSMakeRect(100, 100, 300, 300);
  self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
  [self.window setTitle:@"F3D COCOA Window Test"];
  
  NSOpenGLPixelFormatAttribute attrs[] = {
      NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
      NSOpenGLPFAColorSize, 24,
      NSOpenGLPFAAlphaSize, 8,
      NSOpenGLPFADepthSize, 32,
      NSOpenGLPFAAccelerated,
      0
  };
  
  NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  self.openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
  [self.openGLContext makeCurrentContext];

  engine = std::make_unique<f3d::engine>(f3d::engine::createExternalCOCOA());
  engine->getWindow().setSize(300, 300);
  engine->getScene().add(_dataPath + "/data/cow.vtp");
  _result = engine->getWindow().renderToImage(false);

  [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.1];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
  if (!TestSDKHelpers::RenderTest(_result, _dataPath + "baselines/", _outPath,
            "TestSDKExternalWindowCOCOA"))
  {
    exit(1);
  }
}

@end

int TestSDKExternalWindowCOCOA(int argc, char* argv[])
{
  @autoreleasepool {
    NSApplication *app = [NSApplication sharedApplication];
    AppDelegate *delegate = [[AppDelegate alloc] init];
    delegate.dataPath = argv[1];
    delegate.outPath = argv[2];
    app.delegate = delegate;

    [app run];
  }
  return 0;
}
