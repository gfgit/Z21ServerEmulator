#ifndef RBUSRETROACTION_H
#define RBUSRETROACTION_H

#include <QObject>

#include <bitset>

#include "server/z21server_constants.h"

class Z21Server;

extern "C" void notifyz21S88Data(uint8_t group);

/*!
 * \brief The RBusRetroaction class
 *
 * Which really is about S88 modules
 * because R-BUS is only supported by ROCO and not easy to hack with
 */
class RBusRetroaction : public QObject
{
    Q_OBJECT
private:
    friend class Z21Server;
    friend void ::notifyz21S88Data(uint8_t);

    explicit RBusRetroaction(Z21Server *server);

    void sendS88Status(int group);

public slots:
    void setInputState(int module, int port, bool val);

public:
    inline bool getInputState(int module, int port) const
    {
        if(module < 0 || module >= Z21::S88_MODULE_COUNT)
            return false;

        return modules[module].test(port);
    }

signals:
    void inputStateChanged(int module, int port, bool val);

private:
    Z21Server *m_server;

    std::bitset<8> modules[Z21::S88_MODULE_COUNT];
};

#endif // RBUSRETROACTION_H
