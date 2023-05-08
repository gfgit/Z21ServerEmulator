#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Z21Server;
class PowerStatusLED;
class QComboBox;
class RBusInputModel;
class AccessoryModel;
class LocoDriveModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setupConnections(Z21Server *z21);

private slots:
    void onPowerStateChanged(int state);
    void onPowerComboIndexChanged();
    void onNewThrottle();

private:
    Ui::MainWindow *ui;

    PowerStatusLED *m_powerStatusLed;
    QComboBox *m_powerCombo;

    Z21Server *m_server = nullptr;
    RBusInputModel *rbusModel;
    AccessoryModel *accessoryModel;
    LocoDriveModel *locoModel;
};
#endif // MAINWINDOW_H
