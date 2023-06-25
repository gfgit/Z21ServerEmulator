#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "server/z21server.h"
#include "server/retroaction/rbusinputmodel.h"
#include "server/accessories/accessorymodel.h"
#include "server/loco/locodrivemodel.h"

#include "widgets/powerstatusled.h"
#include <QComboBox>

#include "widgets/throttlewidget.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Power State header
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

    // R-BUS tab
    rbusModel = new RBusInputModel(this);
    ui->rbusTableView->setModel(rbusModel);

    // Accessory Tab
    accessoryModel = new AccessoryModel(this);
    ui->accessoryTableView->setModel(accessoryModel);

    // Loco Tab
    locoModel = new LocoDriveModel(this);
    ui->locoTableView->setModel(locoModel);

    connect(ui->actionNew_Throttle, &QAction::triggered, this, &MainWindow::onNewThrottle);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections(Z21Server *z21)
{
    m_server = z21;

    connect(m_server, &Z21Server::powerStateChanged, m_powerStatusLed,
            &PowerStatusLED::setPowerState_slot, Qt::QueuedConnection);
    m_powerStatusLed->setPowerState(m_server->getPower());

    connect(m_powerCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::onPowerComboIndexChanged);
    connect(m_server, &Z21Server::powerStateChanged, this, &MainWindow::onPowerStateChangedFromZ21,
            Qt::QueuedConnection);
    onPowerStateChangedFromZ21(int(m_server->getPower()));

    connect(ui->actionForce_Update, &QAction::triggered, m_server, &Z21Server::forceReadUpdate);

    connect(m_server, &QObject::destroyed, this, [this]() { m_server = nullptr; });

    rbusModel->setRetroAction(m_server->getRBUS());
    accessoryModel->setAccessoryMgr(m_server->getAccessoryMgr());
    locoModel->setLocoMgr(m_server->getLocoMgr());
}

void MainWindow::onPowerStateChangedFromZ21(int state)
{
    int idx = m_powerCombo->findData(state);
    QSignalBlocker blk(m_powerCombo);
    m_powerCombo->setCurrentIndex(idx);
    m_powerStatusLed->setPowerState_slot(state);
}

void MainWindow::onPowerComboIndexChanged()
{
    QVariant val = m_powerCombo->currentData();
    Z21::PowerState state = Z21::PowerState(val.toInt());
    if (!m_server || state == m_server->getPower())
        return;

    QMetaObject::invokeMethod(
      m_server, [this, state]() { m_server->setPower(state); }, Qt::QueuedConnection);
}

void MainWindow::onNewThrottle()
{
    ThrottleWidget *throttle = new ThrottleWidget(m_server->getLocoMgr(), this);
    throttle->setWindowFlag(Qt::Window);
    throttle->setAttribute(Qt::WA_DeleteOnClose);
    throttle->show();
}
