#ifndef PALETTEDIALOG_H
#define PALETTEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include "beadcolortable.h"
#include "customcolorstab.h"
#include "defaultcolorstab.h"

class PaletteDialog : public QDialog
{
    Q_OBJECT
public:
    PaletteDialog(BeadColorTable&, BeadColorTable&, const QStringList&, QWidget*);
    void updatePalette(BeadColorTable&);

private:
    QTabWidget* tabs;
    QDialogButtonBox *buttons;
};

#endif // PALETTEDIALOG_H
