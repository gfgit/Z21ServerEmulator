#include "throttlewidget.h"

#include "server/loco/locomanager.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>

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

    throttleSlider = new QSlider;
    lay->addRow(throttleSlider);

    QHBoxLayout *butLay = new QHBoxLayout;
    reverseBut = new QPushButton(QLatin1String("<"));
    reverseBut->setToolTip(tr("Reverse"));
    butLay->addWidget(reverseBut);

    stopBut = new QPushButton(QLatin1String("Stop"));
    stopBut->setToolTip(tr("Emergency Stop"));
    butLay->addWidget(stopBut);

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
    connect(stopBut, &QPushButton::clicked, this, &ThrottleWidget::emergencyStop);

    connect(m_locoMgr, &LocoManager::locoSpeedChanged, this, &ThrottleWidget::handleSpeedChanged);

    addressSpinBox->setRange(0, 9999);
    handleSpeedChanged(0, 0, 128, true);

    QFont boldText;
    boldText.setPointSize(18);
    boldText.setBold(true);
    forwardBut->setFont(boldText);
    reverseBut->setFont(boldText);
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
    sendSpeedValue(0);
}

void ThrottleWidget::handleSpeedChanged(int address, int speed, int speedSteps, bool dir)
{
    if(address != addressSpinBox->value())
        return;

    setDirection(dir ? Direction::Forward : Direction::Reverse);

    QSignalBlocker blk(throttleSpinBox
                       );
    throttleSpinBox->setMaximum(speedSteps - 1);
    throttleSlider->setMaximum(speedSteps - 1);

    throttleSpinBox->setValue(speed);
    throttleSlider->setValue(speed);
}

void ThrottleWidget::sendSpeedValue(int speed)
{
    throttleSpinBox->setValue(speed);

    if(!addressSpinBox->value())
        return;

    if(m_direction == Direction::Forward)
        speed |= (1 << 7);

    switch (throttleSpinBox->maximum() + 1)
    {
    case 14:
        m_locoMgr->setSpeed14(addressSpinBox->value(), speed);
        break;
    case 28:
        m_locoMgr->setSpeed28(addressSpinBox->value(), speed);
        break;
    case 128:
        m_locoMgr->setSpeed128(addressSpinBox->value(), speed);
        break;
    default:
        break;
    }
}
