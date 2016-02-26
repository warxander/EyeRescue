#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtGlobal>

class Settings
{
public:
    explicit Settings();

    bool busyMode() const;
    void setBusyMode(bool busyMode);

    bool playSound() const;
    void setPlaySound(bool playSound);

    quint8 remindTime() const;
    void setRemindTime(const quint8 &remindTime);

    quint8 lockTime() const;
    void setLockTime(const quint8 &lockTime);

private:
    quint8 lockTime_;
    quint8 remindTime_;
    bool playSound_;
    bool busyMode_;
};

#endif // SETTINGS_H
