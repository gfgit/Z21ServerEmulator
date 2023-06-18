#include "rbusinputmodel.h"

#include "rbusretroaction.h"


RBusInputModel::RBusInputModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_boldFont.setBold(true);
}

QVariant RBusInputModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QAbstractListModel::headerData(section, orientation, role);

    if(orientation == Qt::Horizontal && section == 0)
    {
        return tr("State");
    }
    else if(orientation == Qt::Vertical)
    {
        auto module = getModuleForRow(section);
        if(module.second == -1)
            return QVariant();

        //Start from input 1 to match PC software UIs
        return module.first * 8 + module.second + 1;
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int RBusInputModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return Z21::S88_MODULE_COUNT * 9;
}

QVariant RBusInputModel::data(const QModelIndex &idx, int role) const
{
    if (!m_retroAction || !idx.isValid())
        return QVariant();

    auto module = getModuleForRow(idx.row());
    if(role == Qt::DisplayRole)
    {
        if(module.second == -1)
            return tr("Module %1").arg(module.first);
        return tr("port %1").arg(module.second);
    }
    else if(role == Qt::CheckStateRole && module.second != -1)
    {
        bool val = m_retroAction->getInputState(module.first, module.second);
        return val ? Qt::Checked : Qt::Unchecked;
    }
    else if(role == Qt::TextAlignmentRole && module.second == -1)
    {
        return Qt::AlignCenter;
    }
    else if(role == Qt::FontRole && module.second == -1)
    {
        return m_boldFont;
    }

    return QVariant();
}

bool RBusInputModel::setData(const QModelIndex &idx, const QVariant& value, int role)
{
    if (!m_retroAction || !idx.isValid() || role != Qt::CheckStateRole)
        return false;

    auto module = getModuleForRow(idx.row());
    if(module.second == -1)
        return false;

    bool val = value.value<Qt::CheckState>() == Qt::Checked;
    emit setInputState(module.first, module.second, val);
    return true;
}

Qt::ItemFlags RBusInputModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;
    if(!idx.isValid())
        return f;

    f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    auto module = getModuleForRow(idx.row());
    if(module.second != -1)
    {
        f.setFlag(Qt::ItemIsUserCheckable);
        f.setFlag(Qt::ItemIsEditable);
    }
    return f;
}

RBusRetroaction *RBusInputModel::retroAction() const
{
    return m_retroAction;
}

void RBusInputModel::setRetroAction(RBusRetroaction *newRetroAction)
{
    if(m_retroAction)
    {
        disconnect(m_retroAction, &RBusRetroaction::inputStateChanged,
                   this, &RBusInputModel::onInputStateChanged);
        disconnect(this, &RBusInputModel::setInputState,
                   m_retroAction, &RBusRetroaction::setInputState);
    }

    m_retroAction = newRetroAction;

    if(m_retroAction)
    {
        connect(m_retroAction, &RBusRetroaction::inputStateChanged,
                this, &RBusInputModel::onInputStateChanged, Qt::QueuedConnection);
        connect(this, &RBusInputModel::setInputState,
                m_retroAction, &RBusRetroaction::setInputState,
                Qt::QueuedConnection);
    }

    //Refresh
    QModelIndex first = index(0, 0);
    QModelIndex last = index(rowCount() - 1, 0);
    emit dataChanged(first, last);
}

void RBusInputModel::onInputStateChanged(int module, int port, bool val)
{
    const int row = getRowForModule(module, port);
    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);
}
