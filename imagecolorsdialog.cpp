#include <QVBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QGroupBox>
#include <QPushButton>
#include "imagecolorsdialog.h"
#include <iostream>

ImageColorsDialog::ImageColorsDialog(BeadColorTable& beadTable, const QPoint p, const QRgb origCol, const QRgb curCol, QWidget* parent)
    : QDialog(parent), pixelColorPixmap(QPixmap(48, 48)), beadPos{p}, origColor{origCol}, curColor{curCol}
{
    setWindowTitle(tr("View/Change Bead Colours"));
    auto *layout = new QVBoxLayout;

    pixelColorImgLabel = new QLabel;
    pixelColorTxtLabel = new QLabel;

    auto painter = new QPainter;
    painter->begin(&pixelColorPixmap);
    painter->fillRect(1,1,46,46,origColor);
    painter->setPen(Qt::black);
    painter->drawRect(0,0,47,47);
    painter->end();
    delete painter;

    pixelColorImgLabel->setPixmap(pixelColorPixmap);
    pixelColorTxtLabel->setText(tr("RGB: %1, %2, %3").arg(qRed(origColor)).arg(qGreen(origColor)).arg(qBlue(origColor)));
    auto *origColGroup = new QGroupBox(tr("Original colour"));
    auto *origColGroupLayout = new QVBoxLayout;
    origColGroupLayout->addWidget(pixelColorImgLabel);
    origColGroupLayout->addWidget(pixelColorTxtLabel);
    origColGroup->setLayout(origColGroupLayout);
    layout->addWidget(origColGroup);

    layout->addWidget(new QLabel(tr("Similarity to bead colours")));

    beadTableDelegate = new TableItemDelegate(curColor);
    model = new TableModel(beadTable, origColor, 1, this);
    beadTableView = new QTableView(this);
    beadTableView->setModel(model);
    beadTableView->setItemDelegate(beadTableDelegate);
    beadTableView->setShowGrid(false);
    beadTableView->horizontalHeader()->setStretchLastSection(true);
    beadTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    beadTableView->setSortingEnabled(true);
    beadTableView->sortByColumn(3, Qt::AscendingOrder);
    beadTableView->resizeColumnsToContents();
    beadTableView->setSelectionMode(QAbstractItemView::SingleSelection);

    buttonReplace = new QPushButton(tr("Replace colour"));
    buttonReplace->setEnabled(false);

    auto *replaceActGroup = new QGroupBox(tr("Replacement action"));
    auto *replaceActLayout = new QVBoxLayout;
    radButtonReplaceSingle = new QRadioButton(tr("Replace only this bead (x: %1 y: %2)").arg(beadPos.x()).arg(beadPos.y()));
    radButtonReplaceAll = new QRadioButton(tr("Replace all"));
    radButtonFloodFill = new QRadioButton(tr("Flood fill"));
    replaceActLayout->addWidget(radButtonReplaceSingle);
    replaceActLayout->addWidget(radButtonReplaceAll);
    replaceActLayout->addWidget(radButtonFloodFill);
    replaceActGroup->setLayout(replaceActLayout);

    auto *targetColGroup = new QGroupBox(tr("Replacement target"));
    auto *targetGroupLayout = new QHBoxLayout;

    radButtonTargetBeadCol = new QRadioButton(tr("This bead's current colour"));
    radButtonTargetOrigCol = new QRadioButton(tr("This pixel's original RGB colour"));
    targetGroupLayout->addWidget(radButtonTargetBeadCol);
    targetGroupLayout->addWidget(radButtonTargetOrigCol);
    targetColGroup->setLayout(targetGroupLayout);

    layout->addWidget(beadTableView);
    layout->addWidget(buttonReplace);
    layout->addWidget(replaceActGroup);
    layout->addWidget(targetColGroup);
    buttonClose = new QDialogButtonBox(QDialogButtonBox::Close);
    layout->addWidget(buttonClose);

    connect(buttonReplace, &QPushButton::released, this, &ImageColorsDialog::replacePushed);
    connect(buttonClose, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto selectionModel = beadTableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ImageColorsDialog::selectionChanged);

    setLayout(layout);
    setMinimumSize(600, 600);
    radButtonReplaceSingle->setChecked(true);
    radButtonTargetBeadCol->setChecked(true);
}

void ImageColorsDialog::replacePushed()
{
    auto index = beadTableView->selectionModel()->currentIndex();
    if (index.isValid())
    {
        QVariant variant = index.sibling(index.row(),0).data(Qt::UserRole);
        auto newColorID = variant.value<BeadID>();
        if (radButtonReplaceSingle->isChecked()) emit replaceColor(beadPos, curColor, newColorID, ReplaceColorTarget::single, radButtonFloodFill->isChecked());
        else if (radButtonTargetBeadCol->isChecked()) emit replaceColor(beadPos, curColor, newColorID, ReplaceColorTarget::all, radButtonFloodFill->isChecked());
        else if (radButtonTargetOrigCol->isChecked()) emit replaceColor(beadPos, origColor, newColorID, ReplaceColorTarget::original, radButtonFloodFill->isChecked());
        beadTableView->viewport()->update();
    }
    QDialog::accept();
}

void ImageColorsDialog::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    const int nSelectedItems = beadTableView->selectionModel()->selectedIndexes().size();
    buttonReplace->setEnabled(nSelectedItems == 1);
}
