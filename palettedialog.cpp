#include "palettedialog.h"
#include "treemodel.h"

PaletteDialog::PaletteDialog(BeadColorTable& defaultBeadTable, BeadColorTable& customBeadTable, const QStringList& keyList, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Edit bead palette"));
    auto *layout = new QVBoxLayout;

    tabs = new QTabWidget;
    auto defaultTab = new DefaultColorsTab(defaultBeadTable, tabs);
    tabs->addTab(defaultTab, tr("Default"));
    auto customTab = new CustomColorsTab(customBeadTable, keyList, tabs);
    tabs->addTab(customTab, tr("Custom"));

    layout->addWidget(tabs);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, defaultTab->getModel(), &TreeModel::onAccept);
    connect(buttons, &QDialogButtonBox::accepted, customTab->getModel(), &TreeModel::onAccept);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
    setMinimumSize(720, 600);
}
