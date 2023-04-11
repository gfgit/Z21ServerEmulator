#ifndef POWERSTATUSLED_H
#define POWERSTATUSLED_H

#include <QWidget>

#include "../z21server_constants.h"

class PowerStatusLED : public QWidget
{
    Q_OBJECT
public:
    explicit PowerStatusLED(QWidget *parent = nullptr);
    ~PowerStatusLED();

    QSize sizeHint() const override;

    Z21::PowerState powerState() const;
    void setPowerState(Z21::PowerState newPowerState);

public slots:
    void setPowerState_slot(int state);

protected:
    void timerEvent(QTimerEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    void startBlinkTimer();
    void stopBlinkTimer();

private:
    int m_blinkTimerId = 0;
    bool m_blinkState = true;
    Z21::PowerState m_powerState = Z21::PowerState::TrackVoltageOff;
};

#endif // POWERSTATUSLED_H
