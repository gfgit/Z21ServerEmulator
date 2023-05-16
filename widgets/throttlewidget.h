#ifndef THROTTLEWIDGET_H
#define THROTTLEWIDGET_H

#include <QWidget>
#include "server/z21server_constants.h"

class QLineEdit;
class QSlider;
class QSpinBox;
class QPushButton;
class QComboBox;
class QCheckBox;

class LocoManager;

enum class Direction
{
    Forward = 0,
    Reverse
};

enum class LocoStatus
{
    Stopped = 0,
    EmergencyStopped,
    Running
};

class ThrottleWidget : public QWidget
{
    Q_OBJECT
public:
    ThrottleWidget(LocoManager *locoMgr, QWidget *parent = nullptr);

    void setDirection(Direction value);

public slots:
    void emergencyStop();
    void normalStop();
    void handleSpeedChanged(int address, int encodedSpeed, int speedSteps, bool dir);
    void loadLoco(int address);
    void setSyncSpeed(bool val);

private slots:
    void sendToZ21();
    void setSpeed_slot(int speed);

private:
    void setSpeedSteps(Z21::DCCSpeedSteps speedSteps);
    void setLocoStatus(LocoStatus status, bool send = true);
    void setSpeed(int speed, bool send = true);
    static int encodeSpeed(const int speed, Z21::DCCSpeedSteps speedSteps, LocoStatus status);
    static int decodeSpeed(int speed, Z21::DCCSpeedSteps speedSteps, LocoStatus& outStatus);

private:
    LocoManager *m_locoMgr;

    QLineEdit *nameEdit;
    QSpinBox *addressSpinBox;
    QCheckBox *syncSpeedCheck;

    QSlider *throttleSlider;
    QSpinBox *throttleSpinBox;
    QComboBox *stepsCombo;

    QPushButton *forwardBut;
    QPushButton *reverseBut;
    QPushButton *stopBut;
    QPushButton *emergencyStopBut;

    int m_address = 0;
    int m_speed = 0;
    Direction m_direction = Direction::Forward;
    Z21::DCCSpeedSteps m_speedSteps = Z21::DCCSpeedSteps::_128;
    LocoStatus m_status = LocoStatus::Stopped;
    bool m_syncSpeed = true;
};

#endif // THROTTLEWIDGET_H
