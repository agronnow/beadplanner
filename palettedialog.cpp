#include "palettedialog.h"
#include "treemodel.h"

PaletteDialog::PaletteDialog(BeadColorTable& defaultBeadTable, BeadColorTable& customBeadTable, const QStringList& keyList, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Edit bead palette"));
    auto *layout = new QVBoxLayout;
    defaultModel = new TreeModel(defaultBeadTable, this);
    defaultColorsTree = new QTreeView(this);
    defaultColorsTree->setModel(defaultModel);

    tabs = new QTabWidget;

    tabs->addTab(defaultColorsTree, tr("Default"));
    auto customTab = new CustomColorsTab(customBeadTable, keyList, tabs);
    tabs->addTab(customTab, tr("Custom"));

    layout->addWidget(tabs);
    defaultColorsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, defaultModel, &TreeModel::onAccept);
    connect(buttons, &QDialogButtonBox::accepted, customTab->getModel(), &TreeModel::onAccept);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
    setMinimumSize(720, 600);
}


CustomColorsTab::CustomColorsTab(BeadColorTable& table, const QStringList& keys, QWidget *parent)
    : QWidget(parent), beadTable{table}, keyList{keys}
{
    auto *layout = new QVBoxLayout;
    customModel = new TreeModel(table, this);
    customColorsTree = new QTreeView(this);
    customColorsTree->setModel(customModel);
    layout->addWidget(customColorsTree);
    auto *buttonLayout = new QHBoxLayout;
    buttonAdd = new QPushButton(tr("&Add colour"));
    buttonEdit = new QPushButton(tr("&Edit colour"));
    buttonRemove = new QPushButton(tr("&Remove colour"));
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonEdit);
    buttonLayout->addWidget(buttonRemove);
    layout->addLayout(buttonLayout);

    connect(buttonAdd, &QPushButton::released, this, &CustomColorsTab::addPushed);
    connect(buttonEdit, &QPushButton::released, this, &CustomColorsTab::editPushed);
    connect(buttonRemove, &QPushButton::released, this, &CustomColorsTab::removePushed);

    connect(this, &CustomColorsTab::dataChanged, customColorsTree, &QTreeView::dataChanged);
    auto selectionModel = customColorsTree->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &CustomColorsTab::selectionChanged);

    buttonEdit->setEnabled(false);
    buttonRemove->setEnabled(false);

    customColorsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setLayout(layout);
}

void CustomColorsTab::addPushed()
{
    BeadColor newBead(true);
    auto editDlg = new CustomColorDialog(newBead, {}, keyList, this);
    if (editDlg->exec() == QDialog::Accepted)
    {
        auto newKey = editDlg->updateItem();
        beadTable.insert(newKey, std::move(newBead));
        if (!keyList.contains(QString::fromStdString(newKey))) keyList.append(QString::fromStdString(newKey));
        const BeadID id({newKey, beadTable[newKey].size()-1});
        customModel->addItem(newBead, id);
    }
}

void CustomColorsTab::editPushed()
{
    auto index = customColorsTree->selectionModel()->currentIndex();
    if (index.isValid())
    {
        BeadColorItem *item = static_cast<BeadColorItem*>(index.internalPointer());
        std::string oldKey(item->getKey());
        auto editDlg = new CustomColorDialog(beadTable[item], oldKey, keyList, this);
        if (editDlg->exec() == QDialog::Accepted)
        {
            auto newKey = editDlg->updateItem();
            if (newKey != oldKey)
            {
                if (!keyList.contains(QString::fromStdString(newKey))) keyList.append(QString::fromStdString(newKey));
                std::size_t newIdx = beadTable.changeKey(item->getID(), newKey);
                customModel->changeItemParent(index, item, {newKey, newIdx});
            }
        }
    }
}

void CustomColorsTab::removePushed()
{
    auto index = customColorsTree->selectionModel()->currentIndex();
    if (index.isValid())
    {
        auto parent = customModel->parent(index);
        if (!customModel->removeRow(index.row(), parent)) std::cout << "Could not remove child" << std::endl;
    }
}

void CustomColorsTab::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    const int nSelectedItems = customColorsTree->selectionModel()->selectedIndexes().size();
    if (nSelectedItems == 0)
    {
        buttonEdit->setEnabled(false);
        buttonRemove->setEnabled(false);
    }
    else
    {
        auto sel = customColorsTree->selectionModel()->selectedIndexes()[0];
        auto item = static_cast<BeadColorItem*>(sel.internalPointer());
        buttonEdit->setEnabled((nSelectedItems > 0) && (item->childCount() == 0));
        buttonRemove->setEnabled((nSelectedItems > 0) && (item->childCount() == 0));
    }
}
