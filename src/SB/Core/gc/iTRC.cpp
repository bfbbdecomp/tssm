#include "iTRC.h"

#include <dolphin/gx/GXCull.h>
#include <dolphin/gx/GXFrameBuffer.h>
#include <dolphin/gx/GXGeometry.h>
#include <dolphin/gx/GXManage.h>
#include <dolphin/gx/GXPixel.h>
#include <dolphin/gx/GXTexture.h>
#include <dolphin/gx/GXTransform.h>
#include <dolphin/mtx.h>
#include <dolphin/vi/vifuncs.h>
#include <libc/string.h>
#include <types.h>

#define FONT_MATRIX_ID 0x1E
#define TEXTBOX_MAX_TEXT_LENGTH 256

extern void GXPosition3s16(S32, S32, S32);
extern void GXTexCoord2s16(S32, S32);

namespace ROMFont
{
    OSFontHeader* mFontData;
    void* mXFBs[2];
    void* mCurrentFrameBuffer;
    GXRenderModeObj* mRenderMode;
    bool mFirstFrame = true;
} // namespace ROMFont

namespace ResetButton
{
    bool mResetEnabled = 1;
    BOOL mWasResetButtonPressed;
    bool mDiscIded; // Return type may be wrong
    void (*mSndKill)();
    void (*mMovieKillFunction)(); // Return type may be wrong
} // namespace ResetButton

namespace iTRCDisk
{
    char mMessage[256];

    void (*mSndSuspend)();
    void (*mSndResume)();
    void (*mAsyncLoadSuspend)();
    void (*mAsyncLoadResume)();
    void (*mPadStopRumbling)();
    void (*mMovieSuspendFunction)();
    void (*mMovieResumeFunction)();
    void (*mMovieResetCallBack)();
    bool mInFModLockingCode;

} // namespace iTRCDisk

void iTRCDisk::SetDVDState()
{
}

void iTRCDisk::DisplayErrorMessage()
{
}

bool iTRCDisk::SetInFModLockingCode(bool Func)
{
    return mInFModLockingCode = Func;
}

bool iTRCDisk::IsDiskIDed()
{
    return DVDCheckDisk();
}

void iTRCDisk::SetMovieResetCallBack(void (*Func)())
{
    mMovieResetCallBack = Func;
}

void iTRCDisk::SetMovieResumeFunction(void (*Func)())
{
    mMovieResumeFunction = Func;
}

void iTRCDisk::SetMovieSuspendFunction(void (*Func)())
{
    mMovieSuspendFunction = Func;
}

void iTRCDisk::SetAsyncLoadResumeFunction(void (*Func)())
{
    mAsyncLoadResume = Func;
}

void iTRCDisk::SetAsyncLoadSuspendFunction(void (*Func)())
{
    mAsyncLoadSuspend = Func;
}

void iTRCDisk::SetSndResumeFunction(void (*Func)())
{
    mSndResume = Func;
}

void iTRCDisk::SetSndSuspendFunction(void (*Func)())
{
    mSndSuspend = Func;
}

void iTRCDisk::SetPadStopRumblingFunction(void (*Func)())
{
    mPadStopRumbling = Func;
}

bool iTRCDisk::Init(void)
{
    return ROMFont::Init();
}

void ResetButton::SetMovieKillFunciton(void (*Func)())
{
    ResetButton::mMovieKillFunction = Func;
}

void ResetButton::SetSndKillFunction(void (*Func)())
{
    ResetButton::mSndKill = Func;
}

void ResetButton::DisableReset()
{
    ResetButton::mResetEnabled = 0;
}

void ResetButton::EnableReset()
{
    ResetButton::mResetEnabled = 1;
}

void ROMFont::DrawTextBox(S32 param_1, S32 param_2, S32 param_3, S32 param_4, char* str)
{
    char* tokenizedString;
    S32 baseWidth;
    S32 tokWidth;
    S32 iVar4;
    S32 iVar5;
    char acStack_128[TEXTBOX_MAX_TEXT_LENGTH];

    if (str != NULL)
    {
        RenderBegin();
        strcpy(acStack_128, str);
        //tokenizedString = strtok(acStack_128, " ");
        baseWidth = GetWidth(" ");
        iVar4 = param_2 + param_4;
        iVar5 = param_1;
        while ((tokenizedString != NULL && (param_2 < iVar4)))
        {
            tokWidth = GetWidth(tokenizedString);
            if (param_1 + param_3 < iVar5 + tokWidth)
            {
                param_2 = param_2 + mFontData->cellHeight;
                iVar5 = param_1;
            }
            DrawString(iVar5, param_2, tokenizedString);
            //tokenizedString = strtok(NULL, " ");
            iVar5 = tokWidth + baseWidth + iVar5;
        }
        RenderEnd();
    }
}
