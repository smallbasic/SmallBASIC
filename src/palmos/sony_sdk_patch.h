/*
*   Patch Sony's SDK for prc-tools
*   This patch is based on Okabe Katsuyuki's patch
*/

#if !defined(SONY_SDK_PATCH)
#define SONY_SDK_PATCH

__asm("
.set    expInit,0
.set    expSlotDriverInstall,1
.set    expSlotDriverRemove,2
.set    expSlotLibFind,3
.set    expSlotRegister,4
.set    expSlotUnregister,5
.set    expCardInserted,6
.set    expCardRemoved,7
.set    expCardPresent,8
.set    expCardInfo,9
.set    expSlotEnumerate,10
.set    expMaxSelector,expSlotEnumerate
.set    expBigSelector,0x7FFF
");

// 0xA805 = sysLibTrapCustom
__asm("
.set    HRTrapGetAPIVersion,0xA805
.set    HRTrapWinClipRectangle,HRTrapGetAPIVersion+1
.set    HRTrapWinCopyRectangle,HRTrapGetAPIVersion+2
.set    HRTrapWinCreateBitmapWindow,HRTrapGetAPIVersion+3
.set    HRTrapWinCreateOffscreenWindow,HRTrapGetAPIVersion+4
.set    HRTrapWinCreateWindow,HRTrapGetAPIVersion+5
.set    HRTrapWinDisplayToWindowPt,HRTrapGetAPIVersion+6
.set    HRTrapWinDrawBitmap,HRTrapGetAPIVersion+7
.set    HRTrapWinDrawChar,HRTrapGetAPIVersion+8
.set    HRTrapWinDrawChars,HRTrapGetAPIVersion+9
.set    HRTrapWinDrawGrayLine,HRTrapGetAPIVersion+10
.set    HRTrapWinDrawGrayRectangleFrame,HRTrapGetAPIVersion+11
.set    HRTrapWinDrawInvertedChars,HRTrapGetAPIVersion+12
.set    HRTrapWinDrawLine,HRTrapGetAPIVersion+13
.set    HRTrapWinDrawPixel,HRTrapGetAPIVersion+14
.set    HRTrapWinDrawRectangle,HRTrapGetAPIVersion+15
.set    HRTrapWinDrawRectangleFrame,HRTrapGetAPIVersion+16
.set    HRTrapWinDrawTruncChars,HRTrapGetAPIVersion+17
.set    HRTrapWinEraseChars,HRTrapGetAPIVersion+18
.set    HRTrapWinEraseLine,HRTrapGetAPIVersion+19
.set    HRTrapWinErasePixel,HRTrapGetAPIVersion+20
.set    HRTrapWinEraseRectangle,HRTrapGetAPIVersion+21
.set    HRTrapWinEraseRectangleFrame,HRTrapGetAPIVersion+22
.set    HRTrapWinFillLine,HRTrapGetAPIVersion+23
.set    HRTrapWinFillRectangle,HRTrapGetAPIVersion+24
.set    HRTrapWinGetClip,HRTrapGetAPIVersion+25
.set    HRTrapWinGetDisplayExtent,HRTrapGetAPIVersion+26
.set    HRTrapWinGetFramesRectangle,HRTrapGetAPIVersion+27
.set    HRTrapWinGetPixel,HRTrapGetAPIVersion+28
.set    HRTrapWinGetWindowBounds,HRTrapGetAPIVersion+29
.set    HRTrapWinGetWindowExtent,HRTrapGetAPIVersion+30
.set    HRTrapWinGetWindowFrameRect,HRTrapGetAPIVersion+31
.set    HRTrapWinInvertChars,HRTrapGetAPIVersion+32
.set    HRTrapWinInvertLine,HRTrapGetAPIVersion+33
.set    HRTrapWinInvertPixel,HRTrapGetAPIVersion+34
.set    HRTrapWinInvertRectangle,HRTrapGetAPIVersion+35
.set    HRTrapWinInvertRectangleFrame,HRTrapGetAPIVersion+36
.set    HRTrapWinPaintBitmap,HRTrapGetAPIVersion+37
.set    HRTrapWinPaintChar,HRTrapGetAPIVersion+38
.set    HRTrapWinPaintChars,HRTrapGetAPIVersion+39
.set    HRTrapWinPaintLine,HRTrapGetAPIVersion+40
.set    HRTrapWinPaintLines,HRTrapGetAPIVersion+41
.set    HRTrapWinPaintPixel,HRTrapGetAPIVersion+42
.set    HRTrapWinPaintPixels,HRTrapGetAPIVersion+43
.set    HRTrapWinPaintRectangle,HRTrapGetAPIVersion+44
.set    HRTrapWinPaintRectangleFrame,HRTrapGetAPIVersion+45
.set    HRTrapWinRestoreBits,HRTrapGetAPIVersion+46
.set    HRTrapWinSaveBits,HRTrapGetAPIVersion+47
.set    HRTrapWinScreenMode,HRTrapGetAPIVersion+48
.set    HRTrapWinScrollRectangle,HRTrapGetAPIVersion+49
.set    HRTrapWinSetClip,HRTrapGetAPIVersion+50
.set    HRTrapWinSetWindowBounds,HRTrapGetAPIVersion+51
.set    HRTrapWinWindowToDisplayPt,HRTrapGetAPIVersion+52
.set    HRTrapBmpBitsSize,HRTrapGetAPIVersion+53
.set    HRTrapBmpSize,HRTrapGetAPIVersion+54
.set    HRTrapBmpCreate,HRTrapGetAPIVersion+55
.set    HRTrapFntGetFont,HRTrapGetAPIVersion+56
.set    HRTrapFntSetFont,HRTrapGetAPIVersion+57
.set    HRTrapFontSelect,HRTrapGetAPIVersion+58
.set    HRTrapSystem,HRTrapGetAPIVersion+59");

#include <SonyCLIE.h>
#include <SonyHRLib.h>

#endif



