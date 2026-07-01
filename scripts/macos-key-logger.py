#!/usr/bin/env -S uv run
# /// script
# requires-python = ">=3.11"
# dependencies = ["pyobjc-framework-Quartz"]
# ///
"""
Logs key events to stdout showing macOS virtual keycode, USB HID keycode, and key name.
Useful for verifying which keycode a pedal or external HID device sends to macOS.

Run with:  uv run scripts/macos-key-logger.py
       or: ./scripts/macos-key-logger.py  (after chmod +x)
       or: nix run github:DivitMittal/TLTR#macos-key-logger

Requires Input Monitoring permission for the terminal:
  System Settings -> Privacy & Security -> Input Monitoring
"""
import Quartz, sys, termios, atexit

if sys.stdin.isatty():
    _saved_attrs = termios.tcgetattr(sys.stdin)
    _noecho = termios.tcgetattr(sys.stdin)
    _noecho[3] &= ~termios.ECHO
    termios.tcsetattr(sys.stdin, termios.TCSANOW, _noecho)
    atexit.register(termios.tcsetattr, sys.stdin, termios.TCSADRAIN, _saved_attrs)

# macOS virtual keycode -> key name
VK = {
    0:'A', 1:'S', 2:'D', 3:'F', 4:'H', 5:'G', 6:'Z', 7:'X',
    8:'C', 9:'V', 11:'B', 12:'Q', 13:'W', 14:'E', 15:'R',
    16:'Y', 17:'T', 31:'O', 32:'U', 34:'I', 35:'P', 37:'L',
    38:'J', 40:'K', 45:'N', 46:'M',
    18:'1', 19:'2', 20:'3', 21:'4', 22:'6', 23:'5',
    25:'9', 26:'7', 28:'8', 29:'0',
    36:'Return', 48:'Tab', 49:'Space', 51:'Delete', 53:'Escape',
    123:'Left', 124:'Right', 125:'Down', 126:'Up',
    115:'Home', 119:'End', 116:'PageUp', 121:'PageDown',
    117:'ForwardDelete',
    122:'F1',  120:'F2',  99:'F3',  118:'F4',  96:'F5',  97:'F6',
    98:'F7',  100:'F8', 101:'F9', 109:'F10', 103:'F11', 111:'F12',
    105:'F13', 107:'F14', 113:'F15', 106:'F16',
    64:'F17',  79:'F18',  80:'F19',  90:'F20',
}

# macOS virtual keycode -> modifier name
MODIFIER_VK = {
    54:'RCmd', 55:'LCmd',
    56:'LShift', 57:'CapsLock', 58:'LOpt', 59:'LCtrl',
    60:'RShift', 61:'ROpt', 62:'RCtrl', 63:'Fn',
}

# key name -> USB HID keycode (Usage Page 0x07)
HID = {
    'A':0x04,'B':0x05,'C':0x06,'D':0x07,'E':0x08,'F':0x09,'G':0x0A,'H':0x0B,
    'I':0x0C,'J':0x0D,'K':0x0E,'L':0x0F,'M':0x10,'N':0x11,'O':0x12,'P':0x13,
    'Q':0x14,'R':0x15,'S':0x16,'T':0x17,'U':0x18,'V':0x19,'W':0x1A,'X':0x1B,
    'Y':0x1C,'Z':0x1D,
    '1':0x1E,'2':0x1F,'3':0x20,'4':0x21,'5':0x22,'6':0x23,'7':0x24,
    '8':0x25,'9':0x26,'0':0x27,
    'Return':0x28,'Escape':0x29,'Delete':0x2A,'Tab':0x2B,'Space':0x2C,
    'Left':0x50,'Right':0x4F,'Down':0x51,'Up':0x52,
    'Home':0x4A,'End':0x4D,'PageUp':0x4B,'PageDown':0x4E,'ForwardDelete':0x4C,
    'F1':0x3A,'F2':0x3B,'F3':0x3C,'F4':0x3D,'F5':0x3E,'F6':0x3F,
    'F7':0x40,'F8':0x41,'F9':0x42,'F10':0x43,'F11':0x44,'F12':0x45,
    'F13':0x68,'F14':0x69,'F15':0x6A,'F16':0x6B,'F17':0x6C,'F18':0x6D,
    'F19':0x6E,'F20':0x6F,
    'LCtrl':0xE0,'LShift':0xE1,'LOpt':0xE2,'LCmd':0xE3,
    'RCtrl':0xE4,'RShift':0xE5,'ROpt':0xE6,'RCmd':0xE7,
    'CapsLock':0x39,
}

_prev_flags = 0


def mod_str(flags):
    parts = []
    if flags & Quartz.kCGEventFlagMaskCommand:
        parts.append('Cmd')
    if flags & Quartz.kCGEventFlagMaskShift:
        parts.append('Shift')
    if flags & Quartz.kCGEventFlagMaskControl:
        parts.append('Ctrl')
    if flags & Quartz.kCGEventFlagMaskAlternate:
        parts.append('Opt')
    return '+'.join(parts)


def callback(proxy, etype, event, refcon):
    global _prev_flags
    vk = Quartz.CGEventGetIntegerValueField(event, Quartz.kCGKeyboardEventKeycode)
    flags = Quartz.CGEventGetFlags(event)

    if etype == Quartz.kCGEventFlagsChanged:
        name = MODIFIER_VK.get(vk, '?')
        hid = HID.get(name)
        hid_str = f'0x{hid:02X}' if hid else '?'
        changed = flags ^ _prev_flags
        direction = 'down' if (flags & changed) else 'up'
        _prev_flags = flags
        print(f'vk={vk:<4}  hid={hid_str:<6}  {name}  [{direction}]')
    else:
        name = VK.get(vk, '?')
        hid = HID.get(name)
        hid_str = f'0x{hid:02X}' if hid else '?'
        mods = mod_str(flags)
        suffix = f'  [{mods}]' if mods else ''
        print(f'vk={vk:<4}  hid={hid_str:<6}  {name}{suffix}')

    sys.stdout.flush()
    return event


mask = (Quartz.CGEventMaskBit(Quartz.kCGEventKeyDown)
        | Quartz.CGEventMaskBit(Quartz.kCGEventFlagsChanged))

tap = Quartz.CGEventTapCreate(
    Quartz.kCGSessionEventTap,
    Quartz.kCGHeadInsertEventTap,
    Quartz.kCGEventTapOptionListenOnly,
    mask,
    callback, None)

if not tap:
    print('Failed to create event tap.')
    print('Grant access: System Settings -> Privacy & Security -> Input Monitoring -> add your terminal')
    sys.exit(1)

src = Quartz.CFMachPortCreateRunLoopSource(None, tap, 0)
Quartz.CFRunLoopAddSource(Quartz.CFRunLoopGetCurrent(), src, "kCFRunLoopDefaultMode")
Quartz.CGEventTapEnable(tap, True)
print('Listening (Ctrl+C to quit)...')
Quartz.CFRunLoopRun()
