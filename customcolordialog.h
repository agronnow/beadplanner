#ifndef CUSTOMCOLORDIALOG_H
#define CUSTOMCOLORDIALOG_H
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QColorDialog>
#include <QPainter>
//#include "beadcoloritem.h"
#include "beadcolortable.h"

class CustomColorDialog : public QDialog
{
    Q_OBJECT
public:
    CustomColorDialog(BeadColor&, const std::string&, const QStringList&, QWidget* parent = nullptr);
    std::string updateItem();

private slots:
    void specifyColor();
    void onTextChanged(const QString&);

private:
    void updateColor(QRgb);
    BeadColor& item;
    QLineEdit *editName;
    QLineEdit *editCode;
    QLineEdit *editSource;
    QComboBox *comboboxType;
    QLabel *labelColor;
    QLabel *labelRed;
    QLabel *labelGreen;
    QLabel *labelBlue;
    QPushButton *buttonColor;
    QDialogButtonBox *buttons;
    QRgb color;
    QPixmap colorPixmap;
};

#endif // CUSTOMCOLORDIALOG_H
