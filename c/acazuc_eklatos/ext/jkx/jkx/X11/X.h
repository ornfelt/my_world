#ifndef X11_X_H
#define X11_X_H

#define None 0

#define Success           0
#define BadRequest        1
#define BadValue          2
#define BadWindow         3
#define BadPixmap         4
#define BadAtom           5
#define BadCursor         6
#define BadFont           7
#define BadMatch          8
#define BadDrawable       9
#define BadAccess         10
#define BadAlloc          11
#define BadColor          12
#define BadGC             13
#define BadIDChoice       14
#define BadName           15
#define BadLength         16
#define BadImplementation 17

#define CWBackPixmap       (1 << 0)
#define CWBackPixel        (1 << 1)
#define CWBorderPixmap     (1 << 2)
#define CWBorderPixel      (1 << 3)
#define CWBitGravity       (1 << 4)
#define CWWinGravity       (1 << 5)
#define CWBackingStore     (1 << 6)
#define CWBackingPlanes    (1 << 7)
#define CWBackingPixel     (1 << 8)
#define CWOverrideRedirect (1 << 9)
#define CWSaveUnder        (1 << 10)
#define CWEventMask        (1 << 11)
#define CWDontPropagate    (1 << 12)
#define CWColormap         (1 << 13)
#define CWCursor           (1 << 14)

#define CWX           (1 << 0)
#define CWY           (1 << 1)
#define CWWidth       (1 << 2)
#define CWHeight      (1 << 3)
#define CWBorderWidth (1 << 4)
#define CWSibling     (1 << 5)
#define CWStackMode   (1 << 6)

#define GCFunction          (1 << 0)
#define GCPlaneMask         (1 << 1)
#define GCForeground        (1 << 2)
#define GCBackground        (1 << 3)
#define GCLineWidth         (1 << 4)
#define GCLineStyle         (1 << 5)
#define GCCapStyle          (1 << 6)
#define GCJoinStyle         (1 << 7)
#define GCFillStyle         (1 << 8)
#define GCFillRule          (1 << 9)
#define GCTile              (1 << 10)
#define GCStipple           (1 << 11)
#define GCTileStipXOrigin   (1 << 12)
#define GCTileStipYOrigin   (1 << 13)
#define GCFont              (1 << 14)
#define GCSubwindowMode     (1 << 15)
#define GCGraphicsExposures (1 << 16)
#define GCClipXOrigin       (1 << 17)
#define GCClipYOrigin       (1 << 18)
#define GCClipMask          (1 << 19)
#define GCDashOffset        (1 << 20)
#define GCDashList          (1 << 21)
#define GCArcMode           (1 << 22)

#define NoEventMask               0
#define KeyPressMask             (1 << 0)
#define KeyReleaseMask           (1 << 1)
#define ButtonPressMask          (1 << 2)
#define ButtonReleaseMask        (1 << 3)
#define EnterWindowMask          (1 << 4)
#define LeaveWindowMask          (1 << 5)
#define PointerMotionMask        (1 << 6)
#define PointerMotionHintMask    (1 << 7)
#define Button1MotionMask        (1 << 8)
#define Button2MotionMask        (1 << 9)
#define Button3MotionMask        (1 << 10)
#define Button4MotionMask        (1 << 11)
#define Button5MotionMask        (1 << 12)
#define ButtonMotionMask         (1 << 13)
#define KeymapStateMask          (1 << 14)
#define ExposureMask             (1 << 15)
#define VisibilityChangeMask     (1 << 16)
#define StructureNotifyMask      (1 << 17)
#define ResizeRedirectMask       (1 << 18)
#define SubstructureNotifyMask   (1 << 19)
#define SubstructureRedirectMask (1 << 20)
#define FocusChangeMask          (1 << 21)
#define PropertyChangeMask       (1 << 22)
#define ColormapChangeMask       (1 << 23)
#define OwnerGrabButtonMask      (1 << 24)

#define CopyFromParent 0

#define InputOutput 1
#define InputOnly   2

#define PropModeReplace 0
#define PropModePrepend 1
#define PropModeAppend  2

#define Button1 1
#define Button2 2
#define Button3 3
#define Button4 4
#define Button5 5

#define ShiftMask   (1 << 0)
#define LockMask    (1 << 1)
#define ControlMask (1 << 2)
#define Mod1Mask    (1 << 3)
#define Mod2Mask    (1 << 4)
#define Mod3Mask    (1 << 5)
#define Mod4Mask    (1 << 6)
#define Mod5Mask    (1 << 7)
#define Button1Mask (1 << 8)
#define Button2Mask (1 << 9)
#define Button3Mask (1 << 10)
#define Button4Mask (1 << 11)
#define Button5Mask (1 << 12)
#define AnyModifier (1 << 15)

#define KeyPress         2
#define KeyRelease       3
#define ButtonPress      4
#define ButtonRelease    5
#define MotionNotify     6
#define EnterNotify      7
#define LeaveNotify      8
#define FocusIn          9
#define FocusOut         10
#define KeymapNotify     11
#define Expose           12
#define GraphicsExpose   13
#define NoExpose         14
#define VisibilityNotify 15
#define CreateNotify     16
#define DestroyNotify    17
#define UnmapNotify      18
#define MapNotify        19
#define MapRequest       20
#define ReparentNotify   21
#define ConfigureNotify  22
#define ConfigureRequest 23
#define GravityNotify    24
#define ResizeRequest    25
#define CirculateNotify  26
#define CirculateRequest 27
#define PropertyNotify   28
#define SelectionClear   29
#define SelectionRequest 30
#define SelectionNotify  31
#define ColormapNotify   32
#define ClientMessage    33
#define MappingNotify    34

#define RaiseLowest  0
#define LowerHighest 1

#define CoordModeOrigin   0
#define CoordModePrevious 1

#define Complex   0
#define Nonconvex 1
#define Convex    2

#define XYBitmap 0
#define XYPixmap 1
#define ZPixmap  2

#define LSBFirst 0
#define MSBFirst 1

#define NotUseful  0
#define WhenMapped 1
#define Always     2

#define StaticGray  0
#define GrayScale   1
#define StaticColor 2
#define PseudoColor 3
#define TrueColor   4
#define DirectColor 5

#define NotifyAncestor         0
#define NotifyVirtual          1
#define NotifyInferior         2
#define NotifyNonlinear        3
#define NotifyNonlinearVirtual 4

#define NotifyNormal 0
#define NotifyGrab   1
#define NotifyUngrab 2

#define Above    0
#define Below    1
#define TopIf    2
#define BottomIf 3
#define Opposite 4

#define PlaceOnTop    0
#define PlaceOnBottom 1

#define PropertyNewValue 0
#define PropertyDelete   1

#define ColormapUninstalled 0
#define ColormapInstalled   1

#define MappingModifier 0
#define MappingKeyboard 1
#define MappingPointer  2

#define NoSymbol 0

#define InputHint        (1 << 0)
#define StateInput       (1 << 1)
#define IconPixmapHint   (1 << 2)
#define IconWindowHint   (1 << 3)
#define IconPositionHint (1 << 4)
#define IconMaskHint     (1 << 5)
#define WindowGroupHint  (1 << 6)
#define UrgencyHint      (1 << 7)
#define AllHints         (InputHint | StateInput | IconPixmapHint \
                        | IconWindowHint | IconPositionHint | IconMaskHint \
                        | WindowGroupHint | UrgencyHint)

#define USPosition  (1 << 0)
#define USSize      (1 << 1)
#define PPosition   (1 << 2)
#define PSize       (1 << 3)
#define PMinSize    (1 << 4)
#define PMaxSize    (1 << 5)
#define PResizeInc  (1 << 6)
#define PAspect     (1 << 7)
#define PBaseSize   (1 << 8)
#define PWinGravity (1 << 9)
#define PAllHints   (USPosition | USSize | PPosition | PSize | PMinSize \
                   | PMaxSize | PResizeInc | PAspect | PBaseSize | PWinGravity)

#define GXclear        0
#define GXand          1
#define GXandReverse   2
#define GXcopy         3
#define GXandInverted  4
#define GXnoop         5
#define GXxor          6
#define GXor           7
#define GXnor          8
#define GXequiv        9
#define GXinvert       10
#define GXorReverse    11
#define GXcopyInverted 12
#define GXorInverted   13
#define GXnand         14
#define GXset          15

#define ClipByChildren   0
#define IncludeInferiors 1

#define ArcChord    0
#define ArcPieSlice 1

#define CapNotLast    0
#define CapButt       1
#define CapRound      2
#define CapProjecting 3

#define JoinMiter 0
#define JoinRound 1
#define JoinBevel 2

#define FillSolid          0
#define FillTiled          1
#define FillStippled       2
#define FillOpaqueStippled 3

#define EvenOddRule 0
#define WindingRule 1

#define LineSolid      0
#define LineOnOffDash  1
#define LineDoubleDash 2

#define ForgetGravity    0
#define UnmapGravity     0
#define NorthWestGravity 1
#define NorthGravity     2
#define NorthEastGravity 3
#define WestGravity      4
#define CenterGravity    5
#define EastGravity      6
#define SouthWestGravity 7
#define SouthGravity     8
#define SouthEastGravity 9
#define StaticGravity    10

#define CurrentTime 0

#define RevertToNone 0
#define RevertToPointerRoot 1
#define RevertToParent 2

#define MappingSuccess 0
#define MappingBusy    1
#define MappingFailed  2

#define ShiftMapIndex   0
#define LockMapIndex    1
#define ControlMapIndex 2
#define Mod1MapIndex    3
#define Mod2MapIndex    4
#define Mod3MapIndex    5
#define Mod4MapIndex    6
#define Mod5MapIndex    7

#define IsUnmapped   0
#define IsUnviewable 1
#define IsViewable   2

#define AnyPropertyType 0

#define GrabModeSync  0
#define GrabModeAsync 1

#define AnyButton 0

#define AlreadyGrabbed 1
#define InvalidTime    2
#define NotViewable    3
#define Frozen         4

#define CursorShape  0
#define TileShape    1
#define StippleShape 2

#define DontCareState  0
#define WithdrawnState 0
#define NormalState    1
#define ZoomState      2
#define IconicState    3
#define InactiveState  4

#define KBKeyClickPercent (1 << 0)
#define KBBellPercent     (1 << 1)
#define KBBellPitch       (1 << 2)
#define KBBellDuration    (1 << 3)
#define KBLed             (1 << 4)
#define KBLedMode         (1 << 5)
#define KBKey             (1 << 6)
#define KBAutoRepeatMode  (1 << 7)

#define AutoRepeatModeOff  0
#define AutoRepeatModeOn   1
#define AutoRepeatModeAuto 2

#define LedModeOff 0
#define LedModeOn  1

#endif
