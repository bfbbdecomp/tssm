#include "zUI.h"
#include "zUICustom.h"

unsigned char zUICustomConditionallyDisabled::PreRender()
{
    return m_bCondition; // struct sees to be missing some data
}

unsigned char zUICustomConditionallyDisabled::PreAutoMenu()
{
    return m_bCondition; // struct sees to be missing some data
}

void zUICustomImageSlideshow::SetTextureList(xGroup* m_TextureGroup)
{
    xGroupGetCount(m_TextureGroup);
}

void zUICustom::PostApplyMotionFrame(zUIMotionFrame*)
{
}

void zUICustom::PreApplyMotionFrame(zUIMotionFrame*)
{
}

void zUICustom::PostInitMotion()
{
}

void zUICustom::PreInitMotion()
{
}

void zUICustom::Exit()
{
}

void zUICustom::PostRender()
{
}

U8 zUICustom::PreAutoMenu()
{
    return 1;
}

U8 zUICustom::PreRender()
{
    return 1;
}

void zUICustom::PostUpdate(float)
{
}

void zUICustom::PreUpdate(float)
{
}

U8 zUICustom::HandleEvent(xBase*, unsigned int, const float*, xBase*, unsigned int)
{
    return 1;
}

void zUICustom::ResetMotion()
{
}

void zUICustom::Reset()
{
}

void zUICustom::Setup()
{
}

void zUICustom::Init()
{
}
