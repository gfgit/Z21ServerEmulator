#include "powerstatusled.h"

#include <QTimerEvent>
#include <QPaintEvent>

#include <QPainter>

QRgb getZ21PowerStateColor(Z21::PowerState state)
{
    switch (state)
    {
    case Z21::PowerState::Normal:
    case Z21::PowerState::EmergencyStop:
        return qRgb( 33, 128, 252);
    case Z21::PowerState::TrackVoltageOff:
        return qRgb(  0,   0,  0);
    case Z21::PowerState::ShortCircuit:
        return qRgb(251,  42,  29);
    case Z21::PowerState::ServiceMode:
        return qRgb(125, 248, 108);
    default:
        break;
    }

    return qRgb(128, 128, 128); //Gray for Unknown value
}

PowerStatusLED::PowerStatusLED(QWidget *parent) :
    QWidget{parent}
{
    setMinimumSize(50, 100);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}

PowerStatusLED::~PowerStatusLED()
{
    stopBlinkTimer();
}

QSize PowerStatusLED::sizeHint() const
{
    return QSize(50, 100);
}

Z21::PowerState PowerStatusLED::powerState() const
{
    return m_powerState;
}

void PowerStatusLED::setPowerState(Z21::PowerState newPowerState)
{
    m_powerState = newPowerState;

    switch (m_powerState)
    {
    case Z21::PowerState::EmergencyStop:
    case Z21::PowerState::ShortCircuit:
        startBlinkTimer();
        break;
    default:
        stopBlinkTimer();
        break;
    }

    update();
}

void PowerStatusLED::setPowerState_slot(int state)
{
    setPowerState(Z21::PowerState(state));
}

void PowerStatusLED::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == m_blinkTimerId)
    {
        m_blinkState = !m_blinkState;
        update();
    }
}

void PowerStatusLED::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    //White background
    painter.fillRect(e->rect(), Qt::white);

    QRectF ledRect = rect();
    ledRect.setRight(qMin(ledRect.height(), ledRect.center().x()));

    QRectF textRect = rect();
    textRect.setLeft(ledRect.right());

    if(m_blinkState)
    {
        //Draw LED circe
        QColor color(getZ21PowerStateColor(m_powerState));
        painter.setBrush(color);

        qreal radius = qMin(ledRect.width(), ledRect.height()) / 2 * 0.9;
        painter.drawEllipse(ledRect.center(), radius, radius);
    }

    painter.setBrush(Qt::NoBrush);

    const QString label = Z21::getPowerStateName(m_powerState);
    QTextOption opt;
    opt.setAlignment(Qt::AlignCenter);
    opt.setWrapMode(QTextOption::WordWrap);

    QFont f;
    f.setBold(true);
    f.setPointSizeF(textRect.height() / 5);
    painter.setFont(f);

    painter.drawText(textRect, label, opt);
}

void PowerStatusLED::startBlinkTimer()
{
    if(m_blinkTimerId)
        return;
    m_blinkTimerId = startTimer(1000, Qt::CoarseTimer);
    m_blinkState = true;
}

void PowerStatusLED::stopBlinkTimer()
{
    if(m_blinkTimerId)
    {
        killTimer(m_blinkTimerId);
        m_blinkTimerId = 0;
        m_blinkState = true;
    }
}
