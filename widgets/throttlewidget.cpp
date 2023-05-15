#include "throttlewidget.h"

#include "server/loco/locomanager.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>

ThrottleWidget::ThrottleWidget(LocoManager *locoMgr, QWidget *parent) :
    QWidget{parent},
    m_locoMgr(locoMgr)
{
    QFormLayout *lay = new QFormLayout(this);

    nameEdit = new QLineEdit;
    lay->addRow(tr("Name"), nameEdit);

    addressSpinBox = new QSpinBox;
    lay->addRow(tr("Address"), addressSpinBox);

    throttleSpinBox = new QSpinBox;
    lay->addRow(tr("Throttle"), throttleSpinBox);

    QHBoxLayout *throttleLay = new QHBoxLayout;
    throttleSlider = new QSlider;
    throttleLay->addWidget(throttleSlider);

    stepsCombo = new QComboBox;
    stepsCombo->addItem(QLatin1String("14"), int(Z21::DCCSpeedSteps::_14));
    stepsCombo->addItem(QLatin1String("28"), int(Z21::DCCSpeedSteps::_28));
    stepsCombo->addItem(QLatin1String("128"), int(Z21::DCCSpeedSteps::_128));

    throttleLay->addWidget(stepsCombo);

    lay->addRow(throttleLay);

    QHBoxLayout *butLay = new QHBoxLayout;
    reverseBut = new QPushButton(QLatin1String("<"));
    reverseBut->setToolTip(tr("Reverse"));
    butLay->addWidget(reverseBut);

    stopBut = new QPushButton(QLatin1String("0"));
    stopBut->setToolTip(tr("Set speed to zero"));
    butLay->addWidget(stopBut);

    emergencyStopBut = new QPushButton(QLatin1String("Stop"));
    emergencyStopBut->setToolTip(tr("Emergency Stop"));
    butLay->addWidget(emergencyStopBut);

    forwardBut = new QPushButton(QLatin1String(">"));
    forwardBut->setToolTip(tr("Forward"));
    butLay->addWidget(forwardBut);

    lay->addRow(butLay);

    connect(throttleSlider, &QSlider::sliderMoved,
            throttleSpinBox, &QSpinBox::setValue);
    connect(throttleSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            throttleSlider, &QSlider::setValue);
    connect(throttleSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &ThrottleWidget::sendSpeedValue);

    connect(forwardBut, &QPushButton::clicked, this, [this]() { setDirection(Direction::Forward); });
    connect(reverseBut, &QPushButton::clicked, this, [this]() { setDirection(Direction::Reverse); });
    connect(emergencyStopBut, &QPushButton::clicked, this, &ThrottleWidget::emergencyStop);
    connect(stopBut, &QPushButton::clicked, this, &ThrottleWidget::normalStop);

    connect(m_locoMgr, &LocoManager::locoSpeedChanged, this, &ThrottleWidget::handleSpeedChanged);
    connect(addressSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ThrottleWidget::loadLoco);

    addressSpinBox->setRange(0, 9999);
    handleSpeedChanged(0, 0, 128, true);

    QFont boldText;
    boldText.setPointSize(18);
    boldText.setBold(true);
    forwardBut->setFont(boldText);
    reverseBut->setFont(boldText);
    emergencyStopBut->setFont(boldText);
    stopBut->setFont(boldText);

    setMinimumSize(200, 100);
    setWindowTitle(tr("Throttle"));
}

void ThrottleWidget::setDirection(Direction value)
{
    m_direction = value;
    QPalette pal;
    QPalette redPal = pal;
    redPal.setBrush(QPalette::ButtonText, Qt::red);

    forwardBut->setPalette(m_direction == Direction::Forward ? redPal : pal);
    reverseBut->setPalette(m_direction == Direction::Reverse ? redPal : pal);

    if(addressSpinBox->value())
        m_locoMgr->setLocoDir(addressSpinBox->value(), m_direction == Direction::Forward);
}

void ThrottleWidget::emergencyStop()
{
    if(m_status == LocoStatus::EmergencyStopped)
    {
        //Reset to normal
        normalStop();
    }
    else
    {
        //Go in emergency stop
        setLocoStatus(LocoStatus::EmergencyStopped);
        sendSpeedValue(0);
    }
}

void ThrottleWidget::normalStop()
{
    setLocoStatus(LocoStatus::Stopped);
    sendSpeedValue(0);
}

void ThrottleWidget::setSpeed(int speed, bool send)
{
    if(speed == m_speed || speed > throttleSpinBox->maximum())
        return;

    m_speed = speed;
    throttleSpinBox->setValue(m_speed);

    if(send)
        sendSpeedValue(m_speed);
}

void ThrottleWidget::handleSpeedChanged(int address, int encodedSpeed, int speedSteps, bool dir)
{
    if(address != addressSpinBox->value())
        return;

    setDirection(dir ? Direction::Forward : Direction::Reverse);
    setSpeedSteps(Z21::speedStepsToEnum(speedSteps));

    LocoStatus status = LocoStatus::Stopped;
    const int speed = decodeSpeed(encodedSpeed, m_speedSteps, status);
    setLocoStatus(status);

    QSignalBlocker blk(throttleSpinBox);
    throttleSpinBox->setValue(speed);
    throttleSlider->setValue(speed);
}

void ThrottleWidget::loadLoco(int address)
{
    m_address = 0; //Disable sending values

    setLocoStatus(LocoStatus::EmergencyStopped);
    setDirection(Direction::Forward);

    if(address != 0 && m_locoMgr->isLocoPresent(address))
    {
        bool dir = m_locoMgr->getLocoDir(address);
        setDirection(dir ? Direction::Forward : Direction::Reverse);

        setSpeedSteps(m_locoMgr->getLocoSpeedSteps(address));

        LocoStatus status = LocoStatus::Stopped;
        const int speed = decodeSpeed(m_locoMgr->getLocoSpeed(address), m_speedSteps, status);
        setLocoStatus(status);
        setSpeed(speed, false);
    }

    //Update value and UI
    m_address = address;
    QSignalBlocker blk(addressSpinBox);
    addressSpinBox->setValue(address);
}

void ThrottleWidget::sendSpeedValue(int speed)
{
    if(!m_address)
        return;

    int encodedSpeed = encodeSpeed(speed, m_speedSteps, m_status);

    m_locoMgr->setLocoSpeed(m_address, encodedSpeed,
                            Z21::speedStepsToInt(m_speedSteps), m_direction == Direction::Forward);
}

void ThrottleWidget::setSpeedSteps(Z21::DCCSpeedSteps speedSteps)
{
    m_speedSteps = speedSteps;

    //Update speed steps combo
    int idx = stepsCombo->findData(int(speedSteps));
    stepsCombo->setCurrentIndex(idx);

    //Update slider and spin box maximum
    int highestStep = Z21::speedStepsToInt(m_speedSteps);
    if(m_speedSteps == Z21::DCCSpeedSteps::_128)
        highestStep -= 2; //Remove "1" step which is Emergency stop, and "0" Stop which is part of value

    QSignalBlocker blk(throttleSpinBox);
    QSignalBlocker blk1(throttleSlider);
    throttleSpinBox->setMaximum(highestStep);
    throttleSlider->setMaximum(highestStep);
}

void ThrottleWidget::setLocoStatus(LocoStatus status)
{
    m_status = status;
    if(m_status == LocoStatus::EmergencyStopped)
    {
        throttleSpinBox->setPrefix(QLatin1String("ESTOP "));
        throttleSpinBox->setEnabled(false);
        throttleSlider->setEnabled(false);
    }
    else
    {
        throttleSpinBox->setPrefix(QString());
        throttleSpinBox->setEnabled(true);
        throttleSlider->setEnabled(true);
    }
}

int ThrottleWidget::encodeSpeed(const int speed, Z21::DCCSpeedSteps speedSteps, LocoStatus status)
{
    if(status == LocoStatus::EmergencyStopped)
        return 0x01; //Step 1 is E-Stop

    int val = 0;

    switch (speedSteps)
    {
    case Z21::DCCSpeedSteps::_14:
    case Z21::DCCSpeedSteps::_128:
        if(speed > 0)
            val = speed + 1; //Skip E-Stop step
        break;
    case Z21::DCCSpeedSteps::_28:
        if(speed > 0)
        {
            val = speed + 3; //Skip 2 values of E-Stop and another value for Stop
            val = ((val >> 1) & 0x0F) | ((val & 0x01) << 4);
        }
        break;
    default:
        break;
    }

    return val;
}

int ThrottleWidget::decodeSpeed(const int val, Z21::DCCSpeedSteps speedSteps, LocoStatus &outStatus)
{
    if(val == 0x01 || (speedSteps == Z21::DCCSpeedSteps::_28 && val == 0x11))
    {
        outStatus = LocoStatus::EmergencyStopped;
        return 0;
    }

    int speed = val;

    switch (speedSteps)
    {
    case Z21::DCCSpeedSteps::_14:
    case Z21::DCCSpeedSteps::_128:
        if(speed >= 1)
            speed -= 1; //Skip E-Stop step
        break;
    case Z21::DCCSpeedSteps::_28:
        speed = ((val & 0x0F) << 1) | ((val & 0x10) >> 4);
        if(speed >= 3)
            speed -= 3 ;//Skip 2 values of E-Stop and another value for Stop
        break;
    default:
        break;
    }

    if(speed == 0)
        outStatus = LocoStatus::Stopped;
    else
        outStatus = LocoStatus::Running;
    return speed;
}
