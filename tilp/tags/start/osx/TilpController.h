#import <Cocoa/Cocoa.h>

@interface TilpController : NSObject
{
    IBOutlet id BoxesController;
    IBOutlet id currentFolder;
    IBOutlet id dirlistTree;
    IBOutlet id mainWindow;
    IBOutlet id dlgboxEntry;
    IBOutlet id memoryUsed;
    IBOutlet id MenuController;
    IBOutlet id numberOfFolders;
    IBOutlet id numberOfVars;
    IBOutlet id PrefsController;
    IBOutlet id ToolbarController;
    NSMutableDictionary *tilpConfig;
    NSUserDefaults *prefs;
}
@end