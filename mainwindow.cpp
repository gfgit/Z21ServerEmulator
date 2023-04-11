#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "server/z21server.h"
#include "server/retroaction/rbusinputmodel.h"

#include "widgets/powerstatusled.h"
#include <QComboBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Power State header
    m_powerStatusLed = new PowerStatusLED(this);

    m_powerCombo = new QComboBox(this);
    m_powerCombo->addItem(Z21::getPowerStateName(Z21::PowerState::Normal),
                          int(Z21::PowerState::Normal));
    m_powerCombo->addItem(Z21::getPowerStateName(Z21::PowerState::EmergencyStop),
                          int(Z21::PowerState::EmergencyStop));
    m_powerCombo->addItem(Z21::getPowerStateName(Z21::PowerState::TrackVoltageOff),
                          int(Z21::PowerState::TrackVoltageOff));
    m_powerCombo->addItem(Z21::getPowerStateName(Z21::PowerState::ShortCircuit),
                          int(Z21::PowerState::ShortCircuit));
    m_powerCombo->addItem(Z21::getPowerStateName(Z21::PowerState::ServiceMode),
                          int(Z21::PowerState::ServiceMode));

    QHBoxLayout *powerLay = new QHBoxLayout;
    powerLay->addWidget(m_powerStatusLed);
    powerLay->addWidget(m_powerCombo);
    ui->verticalLayout->insertLayout(0, powerLay);

    //R-BUS tab
    rbusModel = new RBusInputModel(this);
    ui->rbusTableView->setModel(rbusModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections(Z21Server *z21)
{
    m_server = z21;

    connect(m_server, &Z21Server::powerStateChanged,
            m_powerStatusLed, &PowerStatusLED::setPowerState_slot);
    m_powerStatusLed->setPowerState(m_server->getPower());

    connect(m_powerCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPowerComboIndexChanged);
    connect(m_server, &Z21Server::powerStateChanged,
            this, &MainWindow::onPowerStateChanged);
    onPowerStateChanged(int(m_server->getPower()));

    connect(m_server, &QObject::destroyed, this, [this](){ m_server = nullptr; });

    rbusModel->setRetroAction(m_server->getRBUS());
}

void MainWindow::onPowerStateChanged(int state)
{
    int idx = m_powerCombo->findData(state);
    m_powerCombo->setCurrentIndex(idx);
}

void MainWindow::onPowerComboIndexChanged()
{
    QVariant val = m_powerCombo->currentData();
    Z21::PowerState state = Z21::PowerState(val.toInt());
    if(!m_server || state == m_server->getPower())
        return;

    m_server->setPower(state);
}
