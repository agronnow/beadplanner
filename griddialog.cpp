#include "griddialog.h"

//The dots grid is always kept as the last element of oldGrids/tabs, all preceding elements are line grids
GridDialog::GridDialog(QWidget *parent, std::vector<Grid>& og)
    : QDialog(parent), oldGrids{og}
{
    setWindowTitle(tr("Configure grid"));

    QVBoxLayout *layout = new QVBoxLayout;

    tabs = new QTabWidget;
    tabs->setTabsClosable(true);
    layout->addWidget(tabs);

    for (std::size_t i = 0; i + 1 < oldGrids.size(); ++i)
        tabs->addTab(new GridTab(oldGrids[i]), tr("Gridlines %1").arg(i+1));
    tabs->addTab(new GridTab(oldGrids.back()), tr("Dots"));
    tabs->tabBar()->setTabButton(tabs->count()-1, QTabBar::RightSide, nullptr); //Dots grid cannot be removed

    auto *addButtonLayout = new QHBoxLayout;
    buttonAddGrid = new QPushButton(tr("&Add Grid"));
    addButtonLayout->addWidget(buttonAddGrid);
    addButtonLayout->addStretch();
    layout->addLayout(addButtonLayout);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonAddGrid, &QPushButton::released, this, &GridDialog::addGrid);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &GridDialog::removeGrid);

    setLayout(layout);
}

void GridDialog::addGrid()
{
    int newIndex = tabs->count()-1; //Insert before the Dots tab, which stays last
    tabs->insertTab(newIndex, new GridTab(Grid(GridStyle::dashed)), tr("Gridlines %1").arg(newIndex+1));
    tabs->setCurrentIndex(newIndex);
}

void GridDialog::removeGrid(int index)
{
    if (index < 0 || index >= tabs->count()-1) return; //Cannot remove the Dots tab
    QWidget* tab = tabs->widget(index);
    tabs->removeTab(index);
    delete tab;
}

void GridDialog::updateGrids()
{
    std::vector<Grid> newGrids;
    for (int i = 0; i < tabs->count(); ++i)
    {
        GridTab* tab = qobject_cast<GridTab*>(tabs->widget(i));
        if (tab != nullptr) newGrids.push_back(tab->getGrid());
    }
    oldGrids = newGrids;
}

Grid GridTab::getGrid() const
{
    Grid g = grid;
    g.setDeltaX(gridIntervalXBox->value());
    g.setDeltaY(gridIntervalYBox->value());
    g.setOffsetX(gridOffsetXBox->value());
    g.setOffsetY(gridOffsetYBox->value());
    return g;
}

GridTab::GridTab(const Grid& grid, QWidget *parent)
    : QWidget(parent), grid{grid}
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
