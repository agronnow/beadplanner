#ifndef IMAGESCENE_H
#define IMAGESCENE_H

#include <QGraphicsScene>
#include <QVarLengthArray>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsRectItem>
#include <QRubberBand>
#include <QGraphicsView>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "grid.h"

enum class CursorMode{normal, crop, colorPick, backgroundPick, pastePick};

class ImageScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ImageScene(QObject* parent = nullptr);
    void drawForeground(QPainter*, const QRectF&) override;
    void setImage(const QImage&);
    void setPasteSize(const QSize& size) {pasteSize = size;}
    void toggleShowGrid(std::size_t idx)
    {
        grids[idx].setVisible(!grids[idx].isVisible());
        update();  //Force grid redraw
    }
    BeadPixelCoordTransform& getCoords() {return coords;}
    QSize getScaledImageSize(const QSize& s) const {return coords.nonzeroSize(s*coords.getScaleFactor());}
    std::vector<Grid>& getGrid() {return grids;}
    bool hasPixmap() const {return (pixmapItemMain != nullptr);}
    const QPixmap getPixmap() const {return pixmapItemMain->pixmap();}
signals:
    void crop(QRect&);  // A signal that transmits the cut out area to the application window to install it in the label
    void exitCursorSelectionMode();
    void coordsChanged(QPoint, QPoint, QRgb);
    void clearColorInfo();
    void colorPickerClick(QPoint, QPoint);
    void backgroundPickerClick(QPoint, QRgb);
    void pastePickerClick(QPoint);

public slots:
    void onEnterCursorSelectionMode(CursorMode mode) {cursorMode = mode;}

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
private:
    QGraphicsPixmapItem *pixmapItemMain;
    QGraphicsPixmapItem *pixmapItemOrig;
    BeadPixelCoordTransform coords;
    std::vector<Grid> grids;
    CursorMode cursorMode = CursorMode::normal;
    QSize pasteSize;

    //QGraphicsRectItem* selection = nullptr;
    QRubberBand *rubberBand = nullptr;
    QPoint origin;
    bool leftMouseButtonPressed = false;
};

#endif // IMAGESCENE_H
