#include "backgrounddialog.h"

BackgroundDialog::BackgroundDialog(QPoint pos, QRgb col, QWidget *parent)
    : QDialog(parent), colChange(col, pos, defaultTolerance), BGcolorPixmap(48,48)
{
    setWindowTitle(tr("Set transparent background color"));
    auto *layout = new QVBoxLayout;

    BGcolorImgLabel = new QLabel;
    BGcolorTxtLabel = new QLabel;

    auto painter = new QPainter;
    painter->begin(&BGcolorPixmap);
    painter->fillRect(1,1,46,46,colChange.getOrigColor());
    painter->setPen(Qt::black);
    painter->drawRect(0,0,47,47);
    painter->end();
    delete painter;

    auto *BGcolorGroup = new QGroupBox(tr("Background color"));
    auto *BGcolorGroupLayout = new QVBoxLayout;

    BGcolorGroupLayout->addWidget(BGcolorImgLabel);
    BGcolorGroupLayout->addWidget(BGcolorTxtLabel);
    BGcolorGroup->setLayout(BGcolorGroupLayout);
    layout->addWidget(BGcolorGroup);

    BGcolorImgLabel->setPixmap(BGcolorPixmap);
    BGcolorTxtLabel->setText(tr("RGB: %1, %2, %3").arg(qRed(colChange.getOrigColor())).arg(qGreen(colChange.getOrigColor())).arg(qBlue(colChange.getOrigColor())));

    layout->addWidget(new QLabel(tr("Tolerance")));
    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(1);
    slider->setMaximum(100);
    slider->setValue(defaultTolerance);
    layout->addWidget(slider);
    previewCheckBox = new QCheckBox(tr("Preview"));
    previewCheckBox->setChecked(true);
    layout->addWidget(previewCheckBox);

    auto fillTypeGroup = new QGroupBox(tr("Fill type"));
    auto *fillGroupLayout = new QHBoxLayout;
    radButtonReplaceAll = new QRadioButton(tr("Replace all"));
    radButtonFloodFill = new QRadioButton(tr("Flood fill"));
    fillGroupLayout->addWidget(radButtonReplaceAll);
    fillGroupLayout->addWidget(radButtonFloodFill);
    radButtonReplaceAll->setChecked(true);
    fillTypeGroup->setLayout(fillGroupLayout);
    layout->addWidget(fillTypeGroup);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(slider, &QSlider::valueChanged, this, &BackgroundDialog::onValueChanged);
    connect(previewCheckBox, &QCheckBox::stateChanged, this, &BackgroundDialog::onPreviewToggle);
    connect(radButtonFloodFill, &QRadioButton::toggled, this, &BackgroundDialog::onFloodFillToggle);

    setLayout(layout);
    setMinimumWidth(400);
}

void BackgroundDialog::onValueChanged(int value)
{
    if (previewCheckBox->isChecked())
    {
        colChange.setTolerance(value);
        colChange.setFloodFill(radButtonFloodFill->isChecked());
        emit updateTransparency(colChange, true);
    }
}

void BackgroundDialog::onPreviewToggle(int state)
{
    if (state == Qt::Checked)
    {
        colChange.setTolerance(slider->value());
        colChange.setFloodFill(radButtonFloodFill->isChecked());
        emit updateTransparency(colChange, true);
    }
    else emit resetTransparentColor();
}

void BackgroundDialog::onFloodFillToggle(bool checked)
{
    if (previewCheckBox->isChecked())
    {
        colChange.setTolerance(slider->value());
        colChange.setFloodFill(checked);
        emit updateTransparency(colChange, true);
    }
}
