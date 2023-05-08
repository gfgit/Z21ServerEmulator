#ifndef THROTTLEWIDGET_H
#define THROTTLEWIDGET_H

#include <QWidget>

class QLineEdit;
class QSlider;
class QSpinBox;
class QPushButton;

class LocoManager;

enum class Direction
{
    Forward = 0,
    Reverse
};

class ThrottleWidget : public QWidget
{
    Q_OBJECT
public:
    ThrottleWidget(LocoManager *locoMgr, QWidget *parent = nullptr);

    void setDirection(Direction value);

public slots:
    void emergencyStop();
    void handleSpeedChanged(int address, int speed, int speedSteps, bool dir);

private slots:
    void sendSpeedValue(int speed);

private:
    LocoManager *m_locoMgr;

    QLineEdit *nameEdit;
    QSpinBox *addressSpinBox;

    QSlider *throttleSlider;
    QSpinBox *throttleSpinBox;

    QPushButton *forwardBut;
    QPushButton *reverseBut;
    QPushButton *stopBut;

    Direction m_direction;
};

#endif // THROTTLEWIDGET_H
