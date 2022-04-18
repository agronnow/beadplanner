#ifndef DEFAULTCOLORSTAB_H
#define DEFAULTCOLORSTAB_H

#include <QWidget>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "beadcolortable.h"
#include "treemodel.h"

class DefaultColorsTab : public QWidget
{
    Q_OBJECT
public:
    explicit DefaultColorsTab(BeadColorTable&, QWidget *parent = nullptr);
    TreeModel* getModel() const {return defaultModel;}

signals:

private slots:
    void update();

private:
    BeadColorTable& beadTable;
    QPushButton *buttonUpdate;
    TreeModel *defaultModel;
    QTreeView *defaultColorsTree;
};

#endif // DEFAULTCOLORSTAB_H
