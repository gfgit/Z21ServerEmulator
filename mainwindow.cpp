#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTimerEvent>

#include "z21server_constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->shortCircuitBut, &QPushButton::clicked, this, &MainWindow::setShortCircuit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == m_blinkTimerId)
    {
        blinkState = !blinkState;

        QPixmap pix(QSize(16, 16));
        pix.fill(blinkState ? textColor : Qt::transparent);
        ui->powerLabelIcon->setPixmap(pix);
    }
}

void MainWindow::setPowerStateLed(int state)
{
    Z21::PowerState powerState = Z21::PowerState(state);

    QString text;
    bool blink = false;

    switch (powerState)
    {
    case Z21::PowerState::Normal:
        text = tr("Normal");
        textColor = Qt::cyan;
        break;
    case Z21::PowerState::ServiceMode:
        text = tr("Service Mode");
        textColor = Qt::green;
        break;
    case Z21::PowerState::EmergencyStop:
        text = tr("Emergency Stop");
        textColor = Qt::cyan;
        blink = true;
        break;
    case Z21::PowerState::ShortCircuit:
        text = tr("Short Circuit");
        textColor = Qt::red;
        blink = true;
        break;
    case Z21::PowerState::TrackVoltageOff:
        text = tr("Power Off");
        textColor = Qt::black;
        break;
    default:
        text = tr("Error");
        textColor = Qt::black;
        break;
    }

    ui->powerLabel->setText(text);

    QPixmap pix(QSize(16, 16));
    pix.fill(textColor);
    ui->powerLabelIcon->setPixmap(pix);

    blinkState = true;
    if(blink && !m_blinkTimerId)
    {
        m_blinkTimerId = startTimer(1000);
    }
    else if(!blink && m_blinkTimerId)
    {
        killTimer(m_blinkTimerId);
        m_blinkTimerId = 0;
    }
}

