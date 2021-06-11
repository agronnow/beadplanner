#ifndef BEADSIZEDIALOG_H
#define BEADSIZEDIALOG_H
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>

class BeadSizeDialog : public QDialog
{
    Q_OBJECT
public:
    BeadSizeDialog(double, QWidget* parent = nullptr);
    double getPixelsPerBead();

private:
    QLabel *pixelsLabel;
    QLabel *beadsLabel;
    QSpinBox *pixelsPerBeadBox;
    QSpinBox *beadsPerPixelBox;
    QDialogButtonBox *buttons;
private slots:
    void onPixelsValueChanged(int);
    void onBeadsValueChanged(int);
};

#endif // BEADSIZEDIALOG_H
