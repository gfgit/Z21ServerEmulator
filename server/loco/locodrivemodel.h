#ifndef LOCODRIVEMODEL_H
#define LOCODRIVEMODEL_H

#include <QAbstractTableModel>

class LocoManager;

class LocoDriveModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        Address = 0,
        DCCSteps,
        Direction,
        Throttle,
        F0,
        F1,
        F2,
        F3,
        NCols
    };

    explicit LocoDriveModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    LocoManager *locoMgr() const;
    void setLocoMgr(LocoManager *newLocoMgr);

private slots:
    void onLocoSlotChanged(int Slot);

private:
    LocoManager *m_locoMgr = nullptr;
};

#endif // LOCODRIVEMODEL_H
