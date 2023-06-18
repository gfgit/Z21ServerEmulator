#ifndef ACCESSORYMODEL_H
#define ACCESSORYMODEL_H

#include <QAbstractListModel>

#include <QFont>

class AccessoryManager;

class AccessoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AccessoryModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    AccessoryManager *accessoryMgr() const;
    void setAccessoryMgr(AccessoryManager *newRetroAction);

    inline int getRowForAccessory(int module, int port) const
    {
        //8 ports per module + title row
        return module * (8 + 1) + port + 1;
    }

    inline std::pair<int, int> getAccessoryIndexForRow(int row) const
    {
        //8 ports per module + title row
        int module = row / 9;
        int port = (row - 1) % 9;
        if(port == 8) //It's a title row
            return {module, -1};
        return {module, port};
    }

signals:
    void setAccessoryState(int index, int port, bool val);

private slots:
    void onAccessoryStateChanged(int accIndex, int port, bool val);

private:
    AccessoryManager *m_accessoryMgr = nullptr;
    QFont m_boldFont;
};

#endif // ACCESSORYMODEL_H
