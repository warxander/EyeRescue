#include "mainwindow.h"
#include "settings.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QCloseEvent>
#include <QMediaPlayer>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTimer>

static QString const LOCK_TIME_SETTING = "lock_time";
static QString const REMIND_TIME_SETTING = "remind_time";
static QString const PLAY_SOUND_SETTING = "play_sound";
static QString const BUSY_MODE_SETTING = "busy_mode";

static quint16 const MILLISECONDS_PER_MIN = 60000;
static const int NOTIFY_VOLUME = 50;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow}, systemTray{new QSystemTrayIcon{this}}, lockTimer{new QTimer{this}},
    remindTimer{new QTimer{this}}, settings(new Settings()), player{new QMediaPlayer{this}}
{
    ui->setupUi(this);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    readSettings();

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
    connect(ui->lockScreenSpinBox, SIGNAL(valueChanged(int)), SLOT(changeRemindBeforeMaxValue(int)));

    connect(lockTimer, SIGNAL(timeout()), SLOT(lock()));
    connect(remindTimer, SIGNAL(timeout()), SLOT(remind()));

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    initSystemTrayIcon();

    player->setMedia(QUrl("qrc:/sounds/notify"));
    player->setVolume(NOTIFY_VOLUME);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::lock()
{
    stopTimers();

    if (settings->busyMode())
    {
        notify("Time to rest!");

        if (QMessageBox::question(this, qApp->applicationName(), "Ready to continue?") == QMessageBox::Yes)
            resetTimers();
    }
    else
    {
        if (lockScreen() == QProcess::NormalExit)
        {
            if (QMessageBox::question(this, qApp->applicationName(), "Ready to continue?") == QMessageBox::Yes)
                resetTimers();
        }
        else
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  "Your lock screen is not available."
                                  "\nPlease <a href = 'mailto:dikanchukov@mail.ru'>contact</a> developer.",
                                  QMessageBox::Ok);
            qApp->quit();
        }
    }

}

void MainWindow::remind()
{
    notify(QString::number(settings->remindTime()) + " min. left");
}

void MainWindow::notify(const QString & message)
{
    if (settings->playSound())
        player->play();

    systemTray->showMessage(qApp->applicationName(), "<p align=\"center\">" + message + "</p>");
}

void MainWindow::buttonBoxClicked(QAbstractButton* button)
{
    switch(ui->buttonBox->standardButton(button))
    {
    case QDialogButtonBox::RestoreDefaults:
    {
        readSettings();
        break;
    }
    case QDialogButtonBox::Apply:
    {
        writeSettings();
        hide();
        resetTimers();
        break;
    }
    default:
        break;
    }
}

void MainWindow::systemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger:
        {
            show();
            return;
        }
        default:
            break;
    }
}

void MainWindow::changeRemindBeforeMaxValue(int value)
{
    ui->remindSpinBox->setMaximum(--value);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

void MainWindow::readSettings()
{
    QSettings const qSettings;

    settings->setLockTime(qSettings.value(LOCK_TIME_SETTING, settings->lockTime()).toUInt());
    settings->setRemindTime(qSettings.value(REMIND_TIME_SETTING, settings->remindTime()).toUInt());
    settings->setPlaySound(qSettings.value(PLAY_SOUND_SETTING, settings->playSound()).toBool());
    settings->setBusyMode(qSettings.value(BUSY_MODE_SETTING, settings->busyMode()).toBool());

    ui->lockScreenSpinBox->setValue(settings->lockTime());
    ui->remindSpinBox->setValue(settings->remindTime());
    ui->playSoundCheckBox->setChecked(settings->playSound());
    ui->busyModeCheckBox->setChecked(settings->busyMode());
}

void MainWindow::writeSettings()
{
    QSettings qSettings;

    settings->setLockTime(ui->lockScreenSpinBox->value());
    settings->setRemindTime(ui->remindSpinBox->value());
    settings->setPlaySound(ui->playSoundCheckBox->isChecked());
    settings->setBusyMode(ui->busyModeCheckBox->isChecked());

    qSettings.setValue(LOCK_TIME_SETTING, settings->lockTime());
    qSettings.setValue(REMIND_TIME_SETTING, settings->remindTime());
    qSettings.setValue(PLAY_SOUND_SETTING, settings->playSound());
    qSettings.setValue(BUSY_MODE_SETTING, settings->busyMode());
}

void MainWindow::initSystemTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable() || !QSystemTrayIcon::supportsMessages())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              "Your system tray is not supported."
                              "\nPlease <a href = 'mailto:dikanchukov@mail.ru'>contact</a> developer.",
                              QMessageBox::Ok);
        qApp->quit();
    }


    QMenu* const trayMenu {new QMenu{this}};
    trayMenu->addAction("Sto&p", this, SLOT(stopTimers()));
    trayMenu->addAction("&Reset", this, SLOT(resetTimers()));
    trayMenu->addAction("&Settings", this, SLOT(show()));
    trayMenu->addSeparator();
    trayMenu->addAction("E&xit", qApp, SLOT(quit()));

    setActiveStatusIcon(false);

    connect(systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));

    systemTray->setContextMenu(trayMenu);
    systemTray->show();
}

int MainWindow::lockScreen() const
{
    QStringList const lockScreenCommands
    {
        "gnome-screensaver-command --lock",
        "xscreensaver-command --lock",
        "qdbus org.freedesktop.ScreenSaver /ScreenSaver Lock",
        "qdbus org.gnome.ScreenSaver /ScreenSaver Lock",
        "xlock"
    };

    foreach (QString const & command, lockScreenCommands)
        if (QProcess::execute(command) == QProcess::NormalExit)
            return QProcess::NormalExit;

    return QProcess::CrashExit;
}

void MainWindow::setActiveStatusIcon(bool active)
{
    if (active)
    {
        setWindowIcon(QIcon{":/icons/logo"});
        systemTray->setIcon(QIcon{":/icons/logo"});
    }
    else
    {
        setWindowIcon(QIcon{":/icons/logo_inactive"});
        systemTray->setIcon(QIcon{":/icons/logo_inactive"});
    }
}

void MainWindow::resetTimers()
{
    quint8 const lockTime = settings->lockTime();
    lockTimer->start(lockTime * MILLISECONDS_PER_MIN);
    remindTimer->start((lockTime - settings->remindTime()) * MILLISECONDS_PER_MIN);

    setActiveStatusIcon(true);
}

void MainWindow::stopTimers()
{
    lockTimer->stop();
    remindTimer->stop();

    setActiveStatusIcon(false);
}
