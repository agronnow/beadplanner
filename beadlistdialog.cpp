#include "beadlistdialog.h"
#include <QVBoxLayout>
#include <QHeaderView>

BeadListDialog::BeadListDialog(BeadColorTable& beadTable, int pixelsPerBead, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Count beads"));
    QVBoxLayout *layout = new QVBoxLayout;

    model = new TableModel(beadTable, pixelsPerBead, this);
    beadTableView = new QTableView(this);
    beadTableView->setModel(model);
    beadTableView->setShowGrid(false);
    beadTableView->horizontalHeader()->setStretchLastSection(true);
    beadTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    beadTableView->setSortingEnabled(true);
    beadTableView->sortByColumn(3, Qt::DescendingOrder);
    beadTableView->resizeColumnsToContents();

    layout->addWidget(beadTableView);
    buttonClose = new QDialogButtonBox(QDialogButtonBox::Close);
    layout->addWidget(buttonClose);

    connect(buttonClose, &QDialogButtonBox::rejected, this, &QDialog::reject);
    setLayout(layout);
    setMinimumSize(600, 600);
}
