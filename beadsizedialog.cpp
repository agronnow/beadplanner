#include "beadsizedialog.h"

BeadSizeDialog::BeadSizeDialog(double pixelsPerBead, QWidget *parent)
: QDialog(parent)
{
    setWindowTitle(tr("Set bead size"));
    int nPixels = 1;
    int nBeads = 1;
    QString pixelsText(tr("pixels ="));
    QString beadsText(tr("beads"));
    if (pixelsPerBead > 1.0)
    {
        nPixels = int(pixelsPerBead);
        nBeads = 1;
        beadsText = tr("bead");
    }
    else
    {
        nPixels = 1;
        nBeads = int(1.0/pixelsPerBead);
        pixelsText = tr("pixel =");
        if (pixelsPerBead == 1.0) beadsText = tr("bead");
    }
    QHBoxLayout *Hlayout = new QHBoxLayout;
    pixelsPerBeadBox = new QSpinBox;
    pixelsPerBeadBox->setRange(1, 4000);
    pixelsPerBeadBox->setSingleStep(1);
    pixelsPerBeadBox->setValue(nPixels);
    Hlayout->addWidget(pixelsPerBeadBox);
    pixelsLabel = new QLabel(pixelsText);
    Hlayout->addWidget(pixelsLabel);
    beadsPerPixelBox = new QSpinBox;
    beadsPerPixelBox->setRange(1, 4000);
    beadsPerPixelBox->setSingleStep(1);
    beadsPerPixelBox->setValue(nBeads);
    Hlayout->addWidget(beadsPerPixelBox);
    beadsLabel = new QLabel(beadsText);
    Hlayout->addWidget(beadsLabel);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addLayout(Hlayout);
    layout->addWidget(buttons);

    connect(pixelsPerBeadBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &BeadSizeDialog::onPixelsValueChanged);
    connect(beadsPerPixelBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &BeadSizeDialog::onBeadsValueChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
}

void BeadSizeDialog::onPixelsValueChanged(int newValue)
{
    if (newValue > 1) pixelsLabel->setText(tr("pixels ="));
    else pixelsLabel->setText(tr("pixel ="));
}

void BeadSizeDialog::onBeadsValueChanged(int newValue)
{
    if (newValue > 1) beadsLabel->setText(tr("beads"));
    else beadsLabel->setText(tr("bead"));
}

double BeadSizeDialog::getPixelsPerBead()
{
    int nPixels = pixelsPerBeadBox->value();
    int nBeads = beadsPerPixelBox->value();
    return double(nPixels)/nBeads;
}
