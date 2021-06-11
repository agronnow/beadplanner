#ifndef BACKGROUNDSELECTIONDIALOG_H
#define BACKGROUNDSELECTIONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPainter>
#include <colorchange.h>

class BackgroundDialog : public QDialog
{
    Q_OBJECT
public:
    BackgroundDialog(QPoint, QRgb, QWidget *parent = nullptr);
    ColorChange& getColChange() {return colChange;}
signals:
    void updateTransparency(ColorChange&, bool);
    void resetTransparentColor();

private:
    const int defaultTolerance = 4;
    ColorChange colChange;
    QLabel *BGcolorImgLabel;
    QLabel *BGcolorTxtLabel;
    QPixmap BGcolorPixmap;
    QSlider *slider;
    QCheckBox *previewCheckBox;
    QRadioButton *radButtonReplaceAll;
    QRadioButton *radButtonFloodFill;
    QDialogButtonBox *buttons;

private slots:
     void onValueChanged(int);
     void onPreviewToggle(int);
     void onFloodFillToggle(bool);

};

#endif // BACKGROUNDSELECTIONDIALOG_H
