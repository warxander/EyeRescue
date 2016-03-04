#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class QAbstractButton;
class QAction;
class QMediaPlayer;
class QMessageBox;
class QSystemTrayIcon;
class QTimer;
class Settings;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void lock();
    void remind();
    void notify(QString const & message);

    void buttonBoxClicked(QAbstractButton*);
    void systemTrayActivated(QSystemTrayIcon::ActivationReason);

    void changeRemindBeforeMaxValue(int);

    void resetTimers();
    void stopTimers();

private:
    void closeEvent(QCloseEvent* event) override;

    void readSettings();
    void writeSettings();

    void initSystemTrayIcon();
    void initMediaPlayer();
    void initMessageBox();

    int lockScreen() const;

    void setActiveStatusIcon(bool);

    Ui::MainWindow *ui;
    QSystemTrayIcon* systemTray;
    QMediaPlayer* player;
    QMessageBox* continueMessageBox;
    QAction* busyModeAction;
    QTimer* lockTimer;
    QTimer* remindTimer;
    QScopedPointer<Settings> settings;
};

#endif // MAINWINDOW_H
