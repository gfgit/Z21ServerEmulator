#ifndef RBUSINPUTMODEL_H
#define RBUSINPUTMODEL_H

#include <QAbstractListModel>

#include <QFont>

class RBusRetroaction;

class RBusInputModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RBusInputModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    RBusRetroaction *retroAction() const;
    void setRetroAction(RBusRetroaction *newRetroAction);

    inline int getRowForModule(int module, int port) const
    {
        //8 ports per module + title row
        return module * (8 + 1) + port + 1;
    }

    inline std::pair<int, int> getModuleForRow(int row) const
    {
        //8 ports per module + title row
        int module = row / 9;
        int port = (row - 1) % 9;
        if(port == 8) //It's a title row
            return {module, -1};
        return {module, port};
    }

private slots:
    void onInputStateChanged(int module, int port, bool val);

private:
    RBusRetroaction *m_retroAction = nullptr;
    QFont m_boldFont;
};

#endif // RBUSINPUTMODEL_H
