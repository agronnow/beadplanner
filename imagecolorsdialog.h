#ifndef IMAGECOLORSDIALOG_H
#define IMAGECOLORSDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTableView>
#include <QLabel>
#include <QRadioButton>
#include <QDialogButtonBox>
#include "tablemodel.h"
#include "tableitemdelegate.h"
#include "beadcolortable.h"

enum class ReplaceColorTarget {single, all, original};

class ImageColorsDialog : public QDialog
{
    Q_OBJECT
public:
    ImageColorsDialog(BeadColorTable&, const QPoint, QRgb, QRgb, QWidget*);

signals:
    void replaceColor(QPoint, QRgb, const BeadID&, ReplaceColorTarget, bool);
private slots:
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void replacePushed();

private:
    QLabel *pixelColorImgLabel;
    QLabel *pixelColorTxtLabel;
    QPixmap pixelColorPixmap;
    QTableView *beadTableView;
    TableItemDelegate *beadTableDelegate;
    TableModel *model;
    QPushButton *buttonReplace;
    QRadioButton *radButtonReplaceSingle;
    QRadioButton *radButtonTargetBeadCol;
    QRadioButton *radButtonTargetOrigCol;
    QRadioButton *radButtonReplaceAll;
    QRadioButton *radButtonFloodFill;
    QDialogButtonBox *buttonClose;
    const QPoint beadPos;
    const QRgb origColor;
    QRgb curColor;
};

#endif // IMAGECOLORSDIALOG_H
