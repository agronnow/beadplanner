#include "griddialog.h"

GridDialog::GridDialog(QWidget *parent, std::vector<Grid>& og)
    : QDialog(parent), oldGrids{og}
{
    setWindowTitle(tr("Configure grid"));

    QVBoxLayout *layout = new QVBoxLayout;

    tabs = new QTabWidget;
    layout->addWidget(tabs);

    tabs->addTab(new GridTab(oldGrids[0]), tr("Gridlines"));
    tabs->addTab(new GridTab(oldGrids[1]), tr("Dots"));

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
}

void GridDialog::updateGrids()
{
    for (std::size_t i = 0; i < oldGrids.size(); ++i)
    {
        GridTab* tab = qobject_cast<GridTab*>(tabs->widget(int(i)));
        if (tab != nullptr)
        {
            oldGrids[i].setDeltaX(tab->gridIntervalXBox->value());
            oldGrids[i].setDeltaY(tab->gridIntervalYBox->value());
            oldGrids[i].setOffsetX(tab->gridOffsetXBox->value());
            oldGrids[i].setOffsetY(tab->gridOffsetYBox->value());
        }
    }
}


GridTab::GridTab(Grid& grid, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layoutTopIntervalBox = new QHBoxLayout;
    QHBoxLayout *layoutBottomIntervalBox = new QHBoxLayout;
    layoutTopIntervalBox->addWidget(new QLabel("X spacing"));
    gridIntervalXBox = new QSpinBox;
    gridIntervalXBox->setRange(1, 4000);
    gridIntervalXBox->setSingleStep(1);
    gridIntervalXBox->setValue(grid.getDeltaX());
    layoutTopIntervalBox->addWidget(gridIntervalXBox);
    layoutTopIntervalBox->addWidget(new QLabel("beads"));

    layoutBottomIntervalBox->addWidget(new QLabel("Y spacing"));
    gridIntervalYBox = new QSpinBox;
    gridIntervalYBox->setRange(1, 4000);
    gridIntervalYBox->setSingleStep(1);
    gridIntervalYBox->setValue(grid.getDeltaY());
    layoutBottomIntervalBox->addWidget(gridIntervalYBox);
    layoutBottomIntervalBox->addWidget(new QLabel("beads"));

    QHBoxLayout *layoutTopOffsetBox = new QHBoxLayout;
    QHBoxLayout *layoutBottomOffsetBox = new QHBoxLayout;
    layoutTopOffsetBox->addWidget(new QLabel("X offset"));
    gridOffsetXBox = new QSpinBox;
    gridOffsetXBox->setRange(0, 4000);
    gridOffsetXBox->setSingleStep(1);
    gridOffsetXBox->setValue(grid.getOffsetX());
    layoutTopOffsetBox->addWidget(gridOffsetXBox);
    layoutTopOffsetBox->addWidget(new QLabel("beads"));

    layoutBottomOffsetBox->addWidget(new QLabel("Y offset"));
    gridOffsetYBox = new QSpinBox;
    gridOffsetYBox->setRange(0, 4000);
    gridOffsetYBox->setSingleStep(1);
    gridOffsetYBox->setValue(grid.getOffsetY());
    layoutBottomOffsetBox->addWidget(gridOffsetYBox);
    layoutBottomOffsetBox->addWidget(new QLabel("beads"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addLayout(layoutTopIntervalBox);
    layout->addLayout(layoutBottomIntervalBox);
    layout->addLayout(layoutTopOffsetBox);
    layout->addLayout(layoutBottomOffsetBox);

    setLayout(layout);
}
