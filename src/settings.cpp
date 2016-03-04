#include "settings.h"

static quint8 const LOCK_TIME_DEFAULT = 20;
static quint8 const REMIND_TIME_DEFAULT = 1;
static bool const PLAY_SOUND_DEFAULT = true;

Settings::Settings(): lockTime_(20), remindTime_(REMIND_TIME_DEFAULT),
    playSound_(PLAY_SOUND_DEFAULT)
{
}

bool Settings::playSound() const
{
    return playSound_;
}

void Settings::setPlaySound(bool playSound)
{
    playSound_ = playSound;
}

quint8 Settings::remindTime() const
{
    return remindTime_;
}

void Settings::setRemindTime(const quint8 &remindTime)
{
    remindTime_ = remindTime;
}

quint8 Settings::lockTime() const
{
    return lockTime_;
}

void Settings::setLockTime(const quint8 &lockTime)
{
    lockTime_ = lockTime;
}
