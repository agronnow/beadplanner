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
#include <set>
#include "grid.h"

class QTimer;

enum class CursorMode{normal, crop, colorPick, backgroundPick, pastePick};
enum class CropHandle{none, topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left, inside};

class ImageScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ImageScene(QObject* parent = nullptr);
    void drawForeground(QPainter*, const QRectF&) override;
    void setImage(const QImage&);
    void setPasteSize(const QSize& size) {pasteSize = size;}
    //The dots grid is always kept as the last element of grids, all preceding elements are line grids (see GridDialog)
    void setLineGridsVisible(bool visible)
    {
        for (std::size_t i = 0; i + 1 < grids.size(); ++i) grids[i].setVisible(visible);
        update();  //Force grid redraw
    }
    void toggleShowDotsGrid()
    {
        if (grids.empty()) return;
        grids.back().setVisible(!grids.back().isVisible());
        update();  //Force grid redraw
    }
    BeadPixelCoordTransform& getCoords() {return coords;}
    //Keeps a pending/in-progress crop selection aligned with the same image region after the zoom level changes
    void rescaleCropSelection(double oldScaleFactor, double newScaleFactor);
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
    void confirmCropSelection();
    void cancelCursorSelection();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) override;
private slots:
    void onViewScrolled();
    void onAutoScrollTimeout();

private:
    CropHandle hitTestCropHandle(const QPoint&, const QRect&) const;
    void ensureRubberBand();
    //Rubber band geometry is in viewport widget coordinates while all other selection state (cropRect, origin, etc.)
    //is kept in scene coordinates, which only coincide with viewport coordinates when the view is unscrolled
    QRect viewRectFromScene(const QRect&) const;
    //Recomputes cropRect (and the rubber band) for the drag in progress, given the current pointer position in scene coordinates
    void updateCropDrag(const QPointF&);
    //Starts/stops auto-scrolling the view when the given scene position is near or past the viewport edge during a crop drag
    void updateAutoScroll(const QPointF&);
    void stopAutoScroll();

    QGraphicsPixmapItem *pixmapItemMain;
    BeadPixelCoordTransform coords;
    std::vector<Grid> grids;
    CursorMode cursorMode = CursorMode::normal;
    QSize pasteSize;

    //QGraphicsRectItem* selection = nullptr;
    QRubberBand *rubberBand = nullptr;
    QPoint origin;
    bool leftMouseButtonPressed = false;

    //State for adjusting a crop selection after the initial drag has been released, but before it is confirmed
    QRect cropRect;
    bool cropSelectionActive = false;
    CropHandle activeHandle = CropHandle::none;
    QPoint dragStartPos;
    QRect dragStartRect;
    static constexpr int cropHandleMargin = 6;

    //Auto-scrolling the view while dragging a crop selection near/past the viewport edge
    QTimer *autoScrollTimer = nullptr;
    QPoint autoScrollSpeed;
    static constexpr int autoScrollMargin = 30;
    static constexpr int autoScrollMaxSpeed = 25;
    static constexpr int autoScrollInterval = 16;
};

#endif // IMAGESCENE_H
