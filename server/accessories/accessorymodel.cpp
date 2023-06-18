#include "accessorymodel.h"

#include "accessorymanager.h"


AccessoryModel::AccessoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_boldFont.setBold(true);
}

QVariant AccessoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QAbstractListModel::headerData(section, orientation, role);

    if(orientation == Qt::Horizontal && section == 0)
    {
        return tr("State");
    }
    else if(orientation == Qt::Vertical)
    {
        auto accessory = getAccessoryIndexForRow(section);
        if(accessory.second == -1)
            return QVariant();

        //Start from input 1 to match PC software UIs
        return accessory.first * 8 + accessory.second + 1;
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int AccessoryModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return (Z21::ACCESSORY_COUNT / 8) * 9;
}

QVariant AccessoryModel::data(const QModelIndex &idx, int role) const
{
    if (!m_accessoryMgr || !idx.isValid())
        return QVariant();

    auto accessory = getAccessoryIndexForRow(idx.row());
    if(role == Qt::DisplayRole)
    {
        if(accessory.second == -1)
            return tr("Accessory %1").arg(accessory.first);
        return tr("port %1").arg(accessory.second);
    }
    else if(role == Qt::CheckStateRole && accessory.second != -1)
    {
        bool val = m_accessoryMgr->getAccessoryState(accessory.first, accessory.second);
        return val ? Qt::Checked : Qt::Unchecked;
    }
    else if(role == Qt::TextAlignmentRole && accessory.second == -1)
    {
        return Qt::AlignCenter;
    }
    else if(role == Qt::FontRole && accessory.second == -1)
    {
        return m_boldFont;
    }

    return QVariant();
}

bool AccessoryModel::setData(const QModelIndex &idx, const QVariant& value, int role)
{
    if (!m_accessoryMgr || !idx.isValid() || role != Qt::CheckStateRole)
        return false;

    auto accessory = getAccessoryIndexForRow(idx.row());
    if(accessory.second == -1)
        return false;

    bool val = value.value<Qt::CheckState>() == Qt::Checked;
    emit setAccessoryState(accessory.first, accessory.second, val);
    return true;
}

Qt::ItemFlags AccessoryModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;
    if(!idx.isValid())
        return f;

    f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    auto accessory = getAccessoryIndexForRow(idx.row());
    if(accessory.second != -1)
    {
        f.setFlag(Qt::ItemIsUserCheckable);
        f.setFlag(Qt::ItemIsEditable);
    }
    return f;
}

AccessoryManager *AccessoryModel::accessoryMgr() const
{
    return m_accessoryMgr;
}

void AccessoryModel::setAccessoryMgr(AccessoryManager *newRetroAction)
{
    if(m_accessoryMgr)
    {
        disconnect(m_accessoryMgr, &AccessoryManager::accessoryStateChanged,
                   this, &AccessoryModel::onAccessoryStateChanged);
        disconnect(this, &AccessoryModel::setAccessoryState,
                   m_accessoryMgr, qOverload<int, int, bool>(&AccessoryManager::setAccessoryState));
    }

    m_accessoryMgr = newRetroAction;

    if(m_accessoryMgr)
    {
        connect(m_accessoryMgr, &AccessoryManager::accessoryStateChanged,
                this, &AccessoryModel::onAccessoryStateChanged, Qt::QueuedConnection);
        connect(this, &AccessoryModel::setAccessoryState,
                m_accessoryMgr, qOverload<int, int, bool>(&AccessoryManager::setAccessoryState),
                Qt::QueuedConnection);
    }

    //Refresh
    QModelIndex first = index(0, 0);
    QModelIndex last = index(rowCount() - 1, 0);
    emit dataChanged(first, last);
}

void AccessoryModel::onAccessoryStateChanged(int accIndex, int port, bool val)
{
    const int row = getRowForAccessory(accIndex, port);
    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);
}
