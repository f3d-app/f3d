#import "F3DNSDelegate.h"

// For some reason, this warning is emitted in the CI, so it needs to be disabled
// /Applications/Xcode_13.2.1.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX12.1.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSURLHandle.h:84:1: error: method has no return type specified; defaults to 'id' [-Werror,-Wmissing-method-return-type]
// - initWithURL:(NSURL *)anURL cached:(BOOL)willCache API_DEPRECATED("", macos(10.0, 10.4)) API_UNAVAILABLE(ios, watchos, tvos);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-method-return-type"
#import "AppKit/NSApplication.h"
#pragma clang diagnostic pop

#import "F3DConfig.h"
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

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
  NSString* appName = [NSString stringWithUTF8String:F3D::AppName.c_str()];

  NSMenu* bar = [[NSMenu alloc] init];
  [NSApp setMainMenu:bar];

  NSMenuItem* appMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
  NSMenu* appMenu = [[NSMenu alloc] init];
  [appMenuItem setSubmenu:appMenu];
  [appMenu release];

  // setup "app menu"
  [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                     action:@selector(orderFrontStandardAboutPanel:)
              keyEquivalent:@""];

  [appMenu addItem:[NSMenuItem separatorItem]];

  NSMenu* servicesMenu = [[NSMenu alloc] init];
  [NSApp setServicesMenu:servicesMenu];
  [servicesMenu release];

  [[appMenu addItemWithTitle:@"Services"
                      action:nil
              keyEquivalent:@""] setSubmenu:servicesMenu];

  [appMenu addItem:[NSMenuItem separatorItem]];

  [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                      action:@selector(hide:)
              keyEquivalent:@"h"];

  [[appMenu addItemWithTitle:@"Hide Others"
                      action:@selector(hideOtherApplications:)
              keyEquivalent:@"h"]
    setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];

  [appMenu addItemWithTitle:@"Show All"
                      action:@selector(unhideAllApplications:)
              keyEquivalent:@""];

  [appMenu addItem:[NSMenuItem separatorItem]];

  [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                      action:@selector(terminate:)
              keyEquivalent:@"q"];

  // setup "window menu"
  NSMenuItem* windowMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
  NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
  [NSApp setWindowsMenu:windowMenu];
  [windowMenu release];

  [windowMenuItem setSubmenu:windowMenu];

  [windowMenu addItemWithTitle:@"Minimize"
                        action:@selector(performMiniaturize:)
                  keyEquivalent:@"m"];

  [windowMenu addItemWithTitle:@"Zoom"
                        action:@selector(performZoom:)
                  keyEquivalent:@""];

  [windowMenu addItem:[NSMenuItem separatorItem]];

  [windowMenu addItemWithTitle:@"Bring All to Front"
                        action:@selector(arrangeInFront:)
                 keyEquivalent:@""];

  [windowMenu addItem:[NSMenuItem separatorItem]];

  [[windowMenu addItemWithTitle:@"Enter Full Screen"
                         action:@selector(toggleFullScreen:)
                  keyEquivalent:@"f"]
    setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];
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
