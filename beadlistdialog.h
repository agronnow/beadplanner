#ifndef BEADLISTDIALOG_H
#define BEADLISTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTableView>
#include "tablemodel.h"
#include <QDialogButtonBox>
#include "beadcolortable.h"

class BeadListDialog : public QDialog
{
    Q_OBJECT
public:
    BeadListDialog(BeadColorTable&, int, QWidget*);
private:
    QTableView *beadTableView;
    TableModel *model;
    QDialogButtonBox *buttonClose;
};

#endif // BEADLISTDIALOG_H
