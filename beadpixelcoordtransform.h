#ifndef BEADPIXELCOORDTRANSFORM_H
#define BEADPIXELCOORDTRANSFORM_H

#include <cmath>
#include <QPoint>
#include <QSize>
#include <QRect>

//Transforms bead coordinates to and from zoomed pixel coordinates
//We cannot use the overloaded multiplication and division operators of QPoint and QSize because it rounds to nearest integer and we need to floor
class BeadPixelCoordTransform
{
public:
    int pixelToBeadCoord(int x) const {return int(x/(pixelsPerBead*scaleFactor));}
    int beadToPixelCoord(int x) const {return int(x*pixelsPerBead*scaleFactor);}
    QPoint pixelToBeadCoord(const QPoint& p) const {return QPoint(int(p.x()/(pixelsPerBead*scaleFactor)), int(p.y()/(pixelsPerBead*scaleFactor)));}
    QPoint pixelToBeadCoord(const QPointF& p) const {return QPoint(int(p.x()/(pixelsPerBead*scaleFactor)), int(p.y()/(pixelsPerBead*scaleFactor)));}
    QPoint beadToPixelCoord(const QPoint& p) const {return QPoint(int(p.x()*pixelsPerBead*scaleFactor), int(p.y()*pixelsPerBead*scaleFactor));}
    QSize pixelToBeadCoord(const QSize& s) const {return QSize(int(s.width()/(pixelsPerBead*scaleFactor)), int(s.height()/(pixelsPerBead*scaleFactor)));}
    QSize beadToPixelCoord(const QSize& s) const {return QSize(int(s.width()*pixelsPerBead*scaleFactor), int(s.height()*pixelsPerBead*scaleFactor));}
    //Returns pixel coordinates scaled to snap to bead coordinates
    QPoint snapToBeadCoord(const QPointF& p) const {return QPoint(int(int(p.x()/(pixelsPerBead*scaleFactor))*pixelsPerBead*scaleFactor),
                                                                  int(int(p.y()/(pixelsPerBead*scaleFactor))*pixelsPerBead*scaleFactor));}
    QSize getBeadSize() const {return nonzeroSize(QSize(int(pixelsPerBead*scaleFactor), int(pixelsPerBead*scaleFactor)));}
    QSize nonzeroSize(const QSize& s) const {return (s.width() > 0 && s.height() > 0) ? s : QSize(1,1);}
    QPoint getFlooredPoint(const QPointF& p) const {return QPoint(int(p.x()), int(p.y()));}
    int getConversionFactor() const {return int(pixelsPerBead*scaleFactor);}
    double getScaleFactor() const {return scaleFactor;}
    int getPixelsPerBead() const {return pixelsPerBead;}
    void setPixelsPerBead(int ppb) {pixelsPerBead = std::max(ppb, 1);}
    void setScaleFactor(double sf) {scaleFactor = sf;}
private:
    int pixelsPerBead = 1;
    double scaleFactor = 1.0;
};

#endif // BEADPIXELCOORDTRANSFORM_H
