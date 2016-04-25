#include "mainwindow.h"
#include "settings.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QCloseEvent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
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
    QMainWindow{parent}, ui{new Ui::MainWindow}, systemTray{new QSystemTrayIcon{this}}, player{new QMediaPlayer{this}},
    continueMessageBox{new QMessageBox{this}}, busyModeAction{new QAction{"Busy mode", this}},
    lockTimer{new QTimer{this}}, remindTimer{new QTimer{this}}, settings(new Settings())
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
    initMediaPlayer();
    initMessageBox();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::lock()
{
    stopTimers();

    if (busyModeAction->isChecked())
    {
        notify("Get some rest");

        if (continueMessageBox->exec() == QMessageBox::Yes)
            resetTimers();
    }
    else
    {
        if (lockScreen() == QProcess::NormalExit)
        {
            if (continueMessageBox->exec() == QMessageBox::Yes)
                resetTimers();
        }
        else
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  "Your lock screen is not available."
                                  "<br>Please <a href = 'mailto:dikanchukov@mail.ru'>contact</a> developer.",
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

    systemTray->showMessage(qApp->applicationName(), message);

}

void MainWindow::buttonBoxClicked(QAbstractButton* button)
{
    switch(ui->buttonBox->standardButton(button))
    {
    case QDialogButtonBox::Reset:
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

    ui->lockScreenSpinBox->setValue(settings->lockTime());
    ui->remindSpinBox->setValue(settings->remindTime());
    ui->playSoundCheckBox->setChecked(settings->playSound());
}

void MainWindow::writeSettings()
{
    QSettings qSettings;

    settings->setLockTime(ui->lockScreenSpinBox->value());
    settings->setRemindTime(ui->remindSpinBox->value());
    settings->setPlaySound(ui->playSoundCheckBox->isChecked());

    qSettings.setValue(LOCK_TIME_SETTING, settings->lockTime());
    qSettings.setValue(REMIND_TIME_SETTING, settings->remindTime());
    qSettings.setValue(PLAY_SOUND_SETTING, settings->playSound());
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
    trayMenu->addSeparator();

    busyModeAction->setCheckable(true);
    busyModeAction->setChecked(true);
    trayMenu->addAction(busyModeAction);

    trayMenu->addAction("&Settings", this, SLOT(show()));
    trayMenu->addSeparator();
    trayMenu->addAction("E&xit", qApp, SLOT(quit()));

    setActiveStatusIcon(false);

    connect(systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));

    systemTray->setContextMenu(trayMenu);
    systemTray->show();
}

void MainWindow::initMediaPlayer()
{
    QMediaPlaylist* playlist {new QMediaPlaylist{this}};
    playlist->addMedia(QUrl("qrc:/sounds/notify"));

    player->setPlaylist(playlist);
    player->setVolume(NOTIFY_VOLUME);
}

void MainWindow::initMessageBox()
{
    continueMessageBox->setIcon(QMessageBox::Question);
    continueMessageBox->setWindowModality(Qt::ApplicationModal);
    continueMessageBox->setWindowTitle(qApp->applicationName());
    continueMessageBox->setText("Ready to continue?");
    continueMessageBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    continueMessageBox->setDefaultButton(QMessageBox::Yes);
    continueMessageBox->setEscapeButton(QMessageBox::No);
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

    QStringList const lockScreenCommandsForWin
    {
        "rundll32.exe user32.dll,LockWorkStation"
    };

#ifdef Q_OS_WIN
    QStringList const &commands = lockScreenCommandsForWin;
#else
    QStringList const &commands = lockScreenCommands;
#endif

    foreach (QString const & command, commands)
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
