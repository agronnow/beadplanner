#include "sidebar.h"

Sidebar::Sidebar(QWidget *parent) : QWidget(parent), pixelColorPixmap(QPixmap(48, 48)), OrigPixelColorPixmap(QPixmap(48, 48))
{
    layout = new QVBoxLayout;
    coordsLabel = new QLabel;
    QGroupBox *currentColorBox = new QGroupBox(tr("Colour at cursor"));
    QVBoxLayout *currColLayout = new QVBoxLayout;
    pixelColorImgLabel = new QLabel;
    pixelColorTxtLabel = new QLabel;
    pixelColorMatchingBeadsLabel = new QLabel;
    OrigPixelColorImgLabel = new QLabel;
    OrigPixelColorTxtLabel = new QLabel;
    layout->addWidget(coordsLabel);
    currColLayout->addWidget(pixelColorImgLabel);
    currColLayout->addWidget(pixelColorTxtLabel);
    currColLayout->addWidget(pixelColorMatchingBeadsLabel);
    currentColorBox->setLayout(currColLayout);
    layout->addWidget(currentColorBox);
    origColorBox = new QGroupBox(tr("Original colour"));
    QVBoxLayout *origColLayout = new QVBoxLayout;
    origColLayout->addWidget(OrigPixelColorImgLabel);
    origColLayout->addWidget(OrigPixelColorTxtLabel);
    origColorBox->setLayout(origColLayout);
    layout->addWidget(origColorBox);
    origColorBox->setVisible(false);
    pixelColorMatchingBeadsLabel->setVisible(false);
    layout->setAlignment(Qt::AlignTop);
    setLayout(layout);
    setFixedWidth(300);
}

void Sidebar::onCoordsChanged(QPoint pixelCoords, QPoint beadCoords, QRgb RGBPixel)
{
    Q_UNUSED(pixelCoords)
    auto painter = new QPainter;
    painter->begin(&pixelColorPixmap);
    painter->fillRect(1,1,46,46,RGBPixel);
    painter->setPen(Qt::black);
    painter->drawRect(0,0,47,47);
    painter->end();

    coordsLabel->setText(tr("x: %1 y: %2").arg(beadCoords.x()).arg(beadCoords.y()));
    pixelColorImgLabel->setPixmap(pixelColorPixmap);
    pixelColorTxtLabel->setText(tr("RGB: %1, %2, %3").arg(qRed(RGBPixel)).arg(qGreen(RGBPixel)).arg(qBlue(RGBPixel)));
    delete painter;
}

void Sidebar::onUpdatePixelColorInfo(QRgb origPixelRGB, const std::string& matches)
{
    pixelColorMatchingBeadsLabel->setText(QString::fromStdString(matches));
    auto painter = new QPainter;
    painter->begin(&OrigPixelColorPixmap);
    painter->fillRect(1,1,46,46,origPixelRGB);
    painter->setPen(Qt::black);
    painter->drawRect(0,0,47,47);
    painter->end();
    delete painter;

    OrigPixelColorImgLabel->setPixmap(OrigPixelColorPixmap);
    OrigPixelColorTxtLabel->setText(tr("RGB: %1, %2, %3").arg(qRed(origPixelRGB)).arg(qGreen(origPixelRGB)).arg(qBlue(origPixelRGB)));
}

void Sidebar::onClearInfo()
{
    coordsLabel->clear();
    pixelColorImgLabel->clear();
    pixelColorTxtLabel->clear();
    pixelColorMatchingBeadsLabel->clear();
    OrigPixelColorImgLabel->clear();
    OrigPixelColorTxtLabel->clear();
}
