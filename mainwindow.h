#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class QAbstractButton;
class QSystemTrayIcon;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void activate();
    void remind();

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
    QTimer* activateTimer;
    QTimer* remindTimer;
    quint8 activateTime;
    quint8 remindTime;
};

#endif // MAINWINDOW_H
