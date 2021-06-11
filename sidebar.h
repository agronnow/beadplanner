#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QGroupBox>
#include "imagescene.h"

class Sidebar : public QWidget
{
    Q_OBJECT
public:
    Sidebar(QWidget *parent=nullptr);
    void setOrigColorInfoVisible(bool vis)
    {
        origColorBox->setVisible(vis);
        pixelColorMatchingBeadsLabel->setVisible(vis);
    }

signals:
    void findMatchingBeadColors(std::string&, QRgb);

public slots:
    void onCoordsChanged(QPoint, QPoint, QRgb);
    void onClearInfo();
    void onUpdatePixelColorInfo(QRgb, const std::string&);

private:
    QLabel *pixelColorImgLabel;
    QLabel *pixelColorTxtLabel;
    QLabel *coordsLabel;
    QLabel *pixelColorMatchingBeadsLabel;
    QPixmap pixelColorPixmap;
    QGroupBox *origColorBox;
    QPixmap OrigPixelColorPixmap;
    QLabel *OrigPixelColorImgLabel;
    QLabel *OrigPixelColorTxtLabel;
    QVBoxLayout *layout;
};

#endif // SIDEBAR_H
