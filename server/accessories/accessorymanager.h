#ifndef ACCESSORYMANAGER_H
#define ACCESSORYMANAGER_H

#include <QObject>

#include <bitset>

#include "server/z21server_constants.h"

class Z21Server;

/*!
 * \brief The AccessoryManager class
 *
 * Manages railway accessories
 */
class AccessoryManager : public QObject
{
    Q_OBJECT
private:
    friend class Z21Server;

    explicit AccessoryManager(Z21Server *server);

public:
    void setAccessoryState(int index, int port, bool val);

    inline void setAccessoryState(int address, bool val)
    {
        setAccessoryState(address / 8, address % 8, val);
    }

    inline bool getAccessoryState(int index, int port) const
    {
        if(index < 0 || index >= Z21::ACCESSORY_COUNT / 8)
            return false;

        return accessories[index].test(port);
    }

    inline bool getAccessoryState(int address) const
    {
        return getAccessoryState(address / 8, address % 8);
    }

signals:
    void accessoryStateChanged(int index, int port, bool val);

private:
    Z21Server *m_server;

    std::bitset<8> accessories[Z21::ACCESSORY_COUNT / 8];
};

#endif // ACCESSORYMANAGER_H
