#include "locodrivemodel.h"

#include "server/loco/locomanager.h"

LocoDriveModel::LocoDriveModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

QVariant LocoDriveModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
        case Address:
            return tr("Address");
        case DCCSteps:
            return tr("DCC Steps");
        case Direction:
            return tr("Direction");
        case Throttle:
            return tr("Throttle");
        case F0:
            return tr("F0");
        case F1:
            return tr("F1");
        case F2:
            return tr("F2");
        case F3:
            return tr("F3");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int LocoDriveModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return Z21::MAX_LOCO_SLOTS;
}

int LocoDriveModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return NCols;
}

QVariant LocoDriveModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    LocoSlot& locoSlot = m_locoMgr->loco_slots[idx.row()];

    switch(role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case Address:
            return locoSlot.address();
        case DCCSteps:
            return Z21::speedStepsToInt(locoSlot.getSpeedSteps());
        case Direction:
            if(locoSlot.getDirection())
                return tr("FWD");
            return tr("REV");
        case Throttle:
            return locoSlot.getSpeed();
        case F0:
            return locoSlot.getFunction(0);
        case F1:
            return locoSlot.getFunction(1);
        case F2:
            return locoSlot.getFunction(2);
        case F3:
            return locoSlot.getFunction(3);
        default:
            break;
        }
        break;
    }
    }

    return QVariant();
}

bool LocoDriveModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
//    if (data(idx, role) != value) {
//        // FIXME: Implement me!
//        emit dataChanged(index, index, {role});
//        return true;
//    }
    return false;
}

Qt::ItemFlags LocoDriveModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(idx) | Qt::ItemIsEditable; // FIXME: Implement me!
}

void LocoDriveModel::onLocoSlotChanged(int Slot)
{
    QModelIndex first = index(Slot, 0);
    QModelIndex last = index(Slot, NCols - 1);
    emit dataChanged(first, last);
}

LocoManager *LocoDriveModel::locoMgr() const
{
    return m_locoMgr;
}

void LocoDriveModel::setLocoMgr(LocoManager *newLocoMgr)
{
    if(m_locoMgr)
    {
        disconnect(m_locoMgr, &LocoManager::locoSlotChanged,
                   this, &LocoDriveModel::onLocoSlotChanged);
    }

    m_locoMgr = newLocoMgr;

    if(m_locoMgr)
    {
        connect(m_locoMgr, &LocoManager::locoSlotChanged,
                this, &LocoDriveModel::onLocoSlotChanged);
    }

    //Refresh
    QModelIndex first = index(0, 0);
    QModelIndex last = index(rowCount() - 1, columnCount() - 1);
    emit dataChanged(first, last);
}
