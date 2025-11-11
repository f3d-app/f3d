#import "F3DNSDelegate.h"

// For some reason, this warning is emitted in the CI, so it needs to be disabled
// /Applications/Xcode_13.2.1.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX12.1.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSURLHandle.h:84:1: error: method has no return type specified; defaults to 'id' [-Werror,-Wmissing-method-return-type]
// - initWithURL:(NSURL *)anURL cached:(BOOL)willCache API_DEPRECATED("", macos(10.0, 10.4)) API_UNAVAILABLE(ios, watchos, tvos);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-method-return-type"
#import "AppKit/NSApplication.h"
#import "AppKit/NSOpenPanel.h"
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#pragma clang diagnostic pop

#import "F3DConfig.h"
#import "F3DStarter.h"

// This is a subclass of NSApplicationDelegate.
@interface F3DNSDelegateInternal : NSObject<NSApplicationDelegate>
@property F3DStarter* Starter;
@property BOOL ShouldHandleFileOpening;
@end

@implementation F3DNSDelegateInternal
@synthesize Starter;
@synthesize ShouldHandleFileOpening;

// ----------------------------------------------------------------------------
- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
  (void)theApplication;

  NSArray *arguments = [[NSProcessInfo processInfo] arguments];

  if (ShouldHandleFileOpening || arguments.count <= 1)
  {
    int index = Starter->AddFile([filename UTF8String]);
    if (index > -1)
    {    
      Starter->LoadFileGroup(index);
    }   
    Starter->Render();
  }
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

  // setup "file menu"
  NSMenuItem* fileMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
  NSMenu* fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
  [fileMenuItem setSubmenu:fileMenu];
  [fileMenu release];

  [fileMenuItem setSubmenu:fileMenu];
  [fileMenu addItemWithTitle:@"Open File"
                        action:@selector(openFile:)
                  keyEquivalent:@"o"];

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

  ShouldHandleFileOpening = false;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
  ShouldHandleFileOpening = true;
}

- (void)openFile
{
  auto cstrings = F3DStarter::GetExtensions();
  F3DNSDelegate::ShowOpenFileDialog(cstrings.data(), cstrings.size());
}

- (void)openFileDialogWithAllowedTypes:(NSArray<NSString*>*)allowedTypes
{
  NSOpenPanel* openPanel = [NSOpenPanel openPanel];

  [openPanel setCanChooseFiles:YES];
  [openPanel setCanChooseDirectories:NO];
  [openPanel setAllowsMultipleSelection:NO];

  NSMutableArray<UTType *> *contentTypes = [NSMutableArray array];
  for (NSString *type in allowedTypes) 
  {
      UTType *utType = [UTType typeWithFilenameExtension:type];
      if (utType)
      {
          [contentTypes addObject:utType];
      }
  }
  openPanel.allowedContentTypes = contentTypes;

  NSModalResponse result = [openPanel runModal];
  if (result == NSModalResponseOK)
  {
    NSURL *selectedFile = [[openPanel URLs] firstObject];
    if (selectedFile) 
    {
      int index = self.Starter->AddFile([[selectedFile path] UTF8String]);
      if (index > -1)
      {
        self.Starter->LoadFileGroup(index);
      }
      self.Starter->Render();
    }
  }
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

void F3DNSDelegate::ShowOpenFileDialog(const char* const* extensions, unsigned long long extensionsSize)
{
  F3DNSDelegateInternal* delegate = (F3DNSDelegateInternal*)[NSApp delegate];

  NSMutableArray<NSString*>* allowedTypes = [NSMutableArray arrayWithCapacity:extensionsSize];
  for (size_t i = 0; i < extensionsSize; ++i)
  {
    [allowedTypes addObject:[NSString stringWithUTF8String:extensions[i]]];
  }

  [delegate openFileDialogWithAllowedTypes:allowedTypes];
}
