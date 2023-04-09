#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void timerEvent(QTimerEvent *e) override;

signals:
    void setShortCircuit();

    void s88_state(int module, int port, bool value);

public slots:
    void setPowerStateLed(int state);

private:
    Ui::MainWindow *ui;
    int m_blinkTimerId = 0;
    bool blinkState = true;
    QColor textColor;
};
#endif // MAINWINDOW_H
