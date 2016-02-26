#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class QAbstractButton;
class QMediaPlayer;
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

    int lockScreen() const;

    void setActiveStatusIcon(bool);

    Ui::MainWindow *ui;
    QSystemTrayIcon* systemTray;
    QTimer* lockTimer;
    QTimer* remindTimer;
    QScopedPointer<Settings> settings;
    QMediaPlayer* player;
};

#endif // MAINWINDOW_H
