#include "imagescene.h"
#include <QScrollBar>
#include <QTimer>
#include <QCursor>

ImageScene::ImageScene(QObject* parent) : QGraphicsScene(parent), pixmapItemMain{nullptr}, coords{},
    grids({GridStyle::dashed, GridStyle::dots})
{}

void ImageScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    //Draw grids
    if (pixmapItemMain == nullptr) return;
    QPen pen(Qt::DashLine);
    painter->setPen(pen);
    painter->setCompositionMode(QPainter::RasterOp_NotDestination);  //Ensure that grid lines are visible against background image

    //Line positions are collected across all grids and drawn once each, since drawing the same position twice under
    //RasterOp_NotDestination inverts it back to its original colour, making coinciding lines from overlapping grids disappear
    std::set<int> linePositionsX;
    std::set<int> linePositionsY;

    for (const auto& grid: grids)
    {
        auto gridInterval = coords.beadToPixelCoord(grid.getDelta());
        if ((grid.isVisible()) && (gridInterval.width() > grid.getMinSize()) && (gridInterval.height() > grid.getMinSize()))
        {
            painter->setWorldMatrixEnabled(true);

            double left = int(rect.left()) - (int(rect.left()) % gridInterval.width());
            double top = int(rect.top()) - (int(rect.top()) % gridInterval.height());

            auto gridOffset = coords.beadToPixelCoord(grid.getOffset());

            switch (grid.getStyle())
            {
                case (GridStyle::dashed):
                {
                    for (double x = left + gridOffset.x(); x < pixmapItemMain->pixmap().width(); x += gridInterval.width())
                        linePositionsX.insert(int(x));

                    for (double y = top + gridOffset.y(); y < pixmapItemMain->pixmap().height(); y += gridInterval.height())
                        linePositionsY.insert(int(y));
                    break;
                }
                case (GridStyle::dots):
                {
                    QVarLengthArray<QPointF, 100> points;
                    for (double x = left + gridOffset.x(); x < pixmapItemMain->pixmap().width(); x += gridInterval.width())
                    {
                        for (double y = top + gridOffset.y(); y < pixmapItemMain->pixmap().height(); y += gridInterval.height())
                        {
                            points.append(QPointF(x, y));
                        }
                    }
                    painter->drawPoints(points.data(), points.size());
                    break;
                }
            }
        }
    }

    QVarLengthArray<QLineF, 100> linesX;
    for (int x : linePositionsX) linesX.append(QLineF(x, rect.top(), x, pixmapItemMain->pixmap().height()));
    QVarLengthArray<QLineF, 100> linesY;
    for (int y : linePositionsY) linesY.append(QLineF(rect.left(), y, pixmapItemMain->pixmap().width(), y));
    painter->drawLines(linesX.data(), linesX.size());
    painter->drawLines(linesY.data(), linesY.size());

    //Dashed lines marking the border of the image
    painter->drawLine(QLineF(rect.left(), pixmapItemMain->pixmap().height(), pixmapItemMain->pixmap().width(), pixmapItemMain->pixmap().height()));
    painter->drawLine(QLineF(pixmapItemMain->pixmap().width(), rect.top(), pixmapItemMain->pixmap().width(), pixmapItemMain->pixmap().height()));
}

void ImageScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if ((event->button() & Qt::LeftButton) && (itemAt(event->scenePos(),QTransform()) != nullptr))
    {
        switch(cursorMode)
        {
            case CursorMode::crop:
            {
                QPoint pos = event->scenePos().toPoint();

                // If a selection is already pending, check whether the click grabs a resize handle
                // or the inside of the selection (to move it) instead of starting a brand new one
                if (cropSelectionActive)
                {
                    activeHandle = hitTestCropHandle(pos, cropRect);
                    if (activeHandle != CropHandle::none)
                    {
                        leftMouseButtonPressed = true;
                        // Snap to the same bead-aligned grid as the drag position computed in mouseMoveEvent,
                        // otherwise the delta between them drifts the selection off the pixel grid
                        dragStartPos = coords.snapToBeadCoord(event->scenePos());
                        dragStartRect = cropRect;
                        break;
                    }
                }

                // With the left mouse button pressed, remember the position
                leftMouseButtonPressed = true;
                cropSelectionActive = false;
                activeHandle = CropHandle::none;

                // Create a selection square
                ensureRubberBand();
                origin = coords.snapToBeadCoord(event->scenePos());
                cropRect = QRect(origin, QSize());
                rubberBand->setGeometry(viewRectFromScene(cropRect));
                rubberBand->show();
                break;
            }
            case CursorMode::colorPick:
            {
                QPoint rescaledCoords(int(event->scenePos().x()/coords.getScaleFactor()), int(event->scenePos().y()/coords.getScaleFactor()));
                emit colorPickerClick(rescaledCoords, coords.pixelToBeadCoord(event->scenePos()));
                cursorMode = CursorMode::normal;
                emit exitCursorSelectionMode();
                break;
            }
            case CursorMode::backgroundPick:
            {
                QPoint rescaledCoords(int(event->scenePos().x()/coords.getScaleFactor()), int(event->scenePos().y()/coords.getScaleFactor()));
                QRgb pixelCol = pixmapItemMain->pixmap().toImage().pixel(int(event->scenePos().x()), int(event->scenePos().y()));
                emit backgroundPickerClick(rescaledCoords, pixelCol);
                cursorMode = CursorMode::normal;
                emit exitCursorSelectionMode();
                break;
            }
            default: break;
        }
    }
    else if ((event->button() & Qt::RightButton) && (cursorMode != CursorMode::normal))
    {
        cancelCursorSelection();
    }

    QGraphicsScene::mousePressEvent(event);
}

void ImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (pixmapItemMain == nullptr) return;
    if (cursorMode == CursorMode::crop)
    {
        if (leftMouseButtonPressed)
        {
            updateCropDrag(event->scenePos());
            updateAutoScroll(event->scenePos());
        }
        else if (cropSelectionActive)
        {
            // Not dragging: show an appropriate cursor when hovering over the selection or a resize handle
            QPoint pos = event->scenePos().toPoint();
            switch (hitTestCropHandle(pos, cropRect))
            {
                case CropHandle::topLeft:
                case CropHandle::bottomRight: views()[0]->setCursor(Qt::SizeFDiagCursor); break;
                case CropHandle::topRight:
                case CropHandle::bottomLeft: views()[0]->setCursor(Qt::SizeBDiagCursor); break;
                case CropHandle::left:
                case CropHandle::right: views()[0]->setCursor(Qt::SizeHorCursor); break;
                case CropHandle::top:
                case CropHandle::bottom: views()[0]->setCursor(Qt::SizeVerCursor); break;
                case CropHandle::inside: views()[0]->setCursor(Qt::SizeAllCursor); break;
                default: views()[0]->setCursor(Qt::CrossCursor); break;
            }
        }
    }
    auto x = event->scenePos().x();
    auto y = event->scenePos().y();
    if ((x < pixmapItemMain->pixmap().width()) && (y < pixmapItemMain->pixmap().height()) && (x > 0) && (y > 0))
    {
        QRgb pixelCol = pixmapItemMain->pixmap().toImage().pixel(int(x), int(y));
        if (qAlpha(pixelCol) == 0) emit clearColorInfo();
        else
        {
            //We cannot use QPoint's overloaded division operator because it rounds to nearest integer and we need to floor
            QPoint rescaledCoords(int(event->scenePos().x()/coords.getScaleFactor()), int(event->scenePos().y()/coords.getScaleFactor()));
            emit coordsChanged(rescaledCoords, coords.pixelToBeadCoord(event->scenePos()), pixelCol);
        }
        if (cursorMode == CursorMode::pastePick)
        {
            ensureRubberBand();
            auto curPos = coords.snapToBeadCoord(event->scenePos());
            if (curPos.x() > pixmapItemMain->pixmap().width()) curPos.setX(pixmapItemMain->pixmap().width());
            if (curPos.y() > pixmapItemMain->pixmap().height()) curPos.setY(pixmapItemMain->pixmap().height());
            QRect pasteRect(QRect(curPos, coords.beadToPixelCoord(pasteSize)).normalized());
            if (pasteRect.x() < 0) pasteRect.setX(0);
            if (pasteRect.y() < 0) pasteRect.setY(0);
            rubberBand->setGeometry(viewRectFromScene(pasteRect));
            rubberBand->show();
        }
    }
    else emit clearColorInfo();
    QGraphicsScene::mouseMoveEvent(event);
}

void ImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() & Qt::LeftButton)
    {
        if (cursorMode == CursorMode::crop)
        {
            leftMouseButtonPressed = false;
            stopAutoScroll();

            // When releasing the LMB, the selection area becomes pending: the user can still
            // move or resize it before confirming the crop (Enter/double-click) or cancelling it (Esc/right click)
            if (rubberBand->isVisible())
            {
                const double scaleFactor = coords.getScaleFactor();
                if ((std::floor(cropRect.width()/scaleFactor) > 1.0) && (std::floor(cropRect.height()/scaleFactor) > 1.0))
                {
                    cropSelectionActive = true;
                }
                else
                {
                    cropSelectionActive = false;
                    rubberBand->hide();
                }
            }
            activeHandle = CropHandle::none;
        }
        else if (cursorMode == CursorMode::pastePick)
        {
            rubberBand->hide();
            QPoint rescaledCoords(int(event->scenePos().x()/coords.getScaleFactor()), int(event->scenePos().y()/coords.getScaleFactor()));
            emit pastePickerClick(rescaledCoords);
            cursorMode = CursorMode::normal;
            emit exitCursorSelectionMode();
        }
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void ImageScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if ((event->button() & Qt::LeftButton) && (cursorMode == CursorMode::crop) && cropSelectionActive &&
        (hitTestCropHandle(event->scenePos().toPoint(), cropRect) == CropHandle::inside))
    {
        confirmCropSelection();
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

CropHandle ImageScene::hitTestCropHandle(const QPoint& pos, const QRect& rect) const
{
    if (!rect.adjusted(-cropHandleMargin, -cropHandleMargin, cropHandleMargin, cropHandleMargin).contains(pos)) return CropHandle::none;

    bool onLeft = std::abs(pos.x() - rect.left()) <= cropHandleMargin;
    bool onRight = std::abs(pos.x() - rect.right()) <= cropHandleMargin;
    bool onTop = std::abs(pos.y() - rect.top()) <= cropHandleMargin;
    bool onBottom = std::abs(pos.y() - rect.bottom()) <= cropHandleMargin;

    if (onLeft && onTop) return CropHandle::topLeft;
    if (onRight && onTop) return CropHandle::topRight;
    if (onLeft && onBottom) return CropHandle::bottomLeft;
    if (onRight && onBottom) return CropHandle::bottomRight;
    if (onLeft) return CropHandle::left;
    if (onRight) return CropHandle::right;
    if (onTop) return CropHandle::top;
    if (onBottom) return CropHandle::bottom;
    if (rect.contains(pos)) return CropHandle::inside;
    return CropHandle::none;
}

void ImageScene::confirmCropSelection()
{
    if ((cursorMode != CursorMode::crop) || !cropSelectionActive) return;

    QRect selectionRect = cropRect;
    cropSelectionActive = false;
    activeHandle = CropHandle::none;
    if (rubberBand) rubberBand->hide();
    cursorMode = CursorMode::normal;
    emit crop(selectionRect);
    emit exitCursorSelectionMode();
}

void ImageScene::cancelCursorSelection()
{
    if (cursorMode == CursorMode::normal) return;

    cursorMode = CursorMode::normal;
    cropSelectionActive = false;
    activeHandle = CropHandle::none;
    leftMouseButtonPressed = false;
    stopAutoScroll();
    if (rubberBand) rubberBand->hide();
    emit exitCursorSelectionMode();
}

void ImageScene::rescaleCropSelection(double oldScaleFactor, double newScaleFactor)
{
    if ((cursorMode != CursorMode::crop) || (!cropSelectionActive && !leftMouseButtonPressed)) return;
    if ((oldScaleFactor <= 0.0) || (newScaleFactor <= 0.0) || (oldScaleFactor == newScaleFactor)) return;

    // Convert through bead coordinates (scale-independent) rather than scaling pixel coordinates directly,
    // so the selection stays exactly aligned to the same beads instead of drifting from rounding error
    const double oldUnit = coords.getPixelsPerBead()*oldScaleFactor;
    const double newUnit = coords.getPixelsPerBead()*newScaleFactor;
    auto rescalePoint = [oldUnit, newUnit](const QPoint& p)
    {
        return QPoint(int(int(p.x()/oldUnit)*newUnit), int(int(p.y()/oldUnit)*newUnit));
    };
    auto rescaleRect = [&rescalePoint](const QRect& r) {return QRect(rescalePoint(r.topLeft()), rescalePoint(r.bottomRight()));};

    cropRect = rescaleRect(cropRect);
    origin = rescalePoint(origin);
    dragStartRect = rescaleRect(dragStartRect);
    dragStartPos = rescalePoint(dragStartPos);

    if (pixmapItemMain)
    {
        const int maxWidth = pixmapItemMain->pixmap().width();
        const int maxHeight = pixmapItemMain->pixmap().height();
        if (cropRect.right() > maxWidth) cropRect.moveRight(maxWidth);
        if (cropRect.bottom() > maxHeight) cropRect.moveBottom(maxHeight);
        if (cropRect.left() < 0) cropRect.moveLeft(0);
        if (cropRect.top() < 0) cropRect.moveTop(0);
    }

    if (rubberBand && rubberBand->isVisible()) rubberBand->setGeometry(viewRectFromScene(cropRect));
}

void ImageScene::ensureRubberBand()
{
    if (rubberBand) return;
    QGraphicsView* view = views()[0];
    rubberBand = new QRubberBand(QRubberBand::Rectangle, view->viewport());
    // The rubber band is a plain widget and does not follow the view when it is panned/scrolled,
    // so its geometry must be recomputed from scene coordinates whenever the scroll position changes
    connect(view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &ImageScene::onViewScrolled, Qt::UniqueConnection);
    connect(view->verticalScrollBar(), &QScrollBar::valueChanged, this, &ImageScene::onViewScrolled, Qt::UniqueConnection);
}

QRect ImageScene::viewRectFromScene(const QRect& sceneRect) const
{
    QGraphicsView* view = views()[0];
    return QRect(view->mapFromScene(sceneRect.topLeft()), view->mapFromScene(sceneRect.bottomRight()));
}

void ImageScene::onViewScrolled()
{
    if (!rubberBand || !rubberBand->isVisible()) return;
    if ((cursorMode == CursorMode::crop) && (cropSelectionActive || leftMouseButtonPressed))
    {
        rubberBand->setGeometry(viewRectFromScene(cropRect));
    }
}

void ImageScene::updateCropDrag(const QPointF& scenePos)
{
    if (!pixmapItemMain || !rubberBand) return;

    auto curPos = coords.snapToBeadCoord(scenePos);
    if (curPos.x() > pixmapItemMain->pixmap().width()) curPos.setX(pixmapItemMain->pixmap().width());
    if (curPos.y() > pixmapItemMain->pixmap().height()) curPos.setY(pixmapItemMain->pixmap().height());
    if (curPos.x() < 0) curPos.setX(0);
    if (curPos.y() < 0) curPos.setY(0);

    if (activeHandle == CropHandle::none)
    {
        // Form the selection area when drawing a brand new selection with the mouse while pressing the LMB
        QRect newRect(QRect(origin, curPos).normalized());
        if (newRect.x() < 0) newRect.setX(0);
        if (newRect.y() < 0) newRect.setY(0);
        cropRect = newRect;
    }
    else if (activeHandle == CropHandle::inside)
    {
        // Move the whole pending selection, keeping it within the image bounds
        QRect moved = dragStartRect.translated(curPos - dragStartPos);
        if (moved.left() < 0) moved.moveLeft(0);
        if (moved.top() < 0) moved.moveTop(0);
        if (moved.right() > pixmapItemMain->pixmap().width()) moved.moveRight(pixmapItemMain->pixmap().width());
        if (moved.bottom() > pixmapItemMain->pixmap().height()) moved.moveBottom(pixmapItemMain->pixmap().height());
        cropRect = moved;
    }
    else
    {
        // Resize the pending selection by dragging one of its handles
        QRect resized = dragStartRect;
        switch (activeHandle)
        {
            case CropHandle::topLeft: resized.setTopLeft(curPos); break;
            case CropHandle::top: resized.setTop(curPos.y()); break;
            case CropHandle::topRight: resized.setTopRight(curPos); break;
            case CropHandle::right: resized.setRight(curPos.x()); break;
            case CropHandle::bottomRight: resized.setBottomRight(curPos); break;
            case CropHandle::bottom: resized.setBottom(curPos.y()); break;
            case CropHandle::bottomLeft: resized.setBottomLeft(curPos); break;
            case CropHandle::left: resized.setLeft(curPos.x()); break;
            default: break;
        }
        cropRect = resized.normalized();
    }
    rubberBand->setGeometry(viewRectFromScene(cropRect));
}

void ImageScene::updateAutoScroll(const QPointF& scenePos)
{
    QGraphicsView* view = views()[0];
    const QPoint viewportPos = view->mapFromScene(scenePos);
    const QRect vpRect = view->viewport()->rect();

    auto edgeSpeed = [](int pos, int nearMin, int nearMax) -> int
    {
        if (pos < nearMin) return -std::min(autoScrollMaxSpeed, (nearMin - pos)/2 + 4);
        if (pos > nearMax) return std::min(autoScrollMaxSpeed, (pos - nearMax)/2 + 4);
        return 0;
    };

    autoScrollSpeed.setX(edgeSpeed(viewportPos.x(), vpRect.left() + autoScrollMargin, vpRect.right() - autoScrollMargin));
    autoScrollSpeed.setY(edgeSpeed(viewportPos.y(), vpRect.top() + autoScrollMargin, vpRect.bottom() - autoScrollMargin));

    if (autoScrollSpeed.isNull())
    {
        stopAutoScroll();
        return;
    }
    if (!autoScrollTimer)
    {
        autoScrollTimer = new QTimer(this);
        connect(autoScrollTimer, &QTimer::timeout, this, &ImageScene::onAutoScrollTimeout);
    }
    if (!autoScrollTimer->isActive()) autoScrollTimer->start(autoScrollInterval);
}

void ImageScene::stopAutoScroll()
{
    autoScrollSpeed = QPoint();
    if (autoScrollTimer) autoScrollTimer->stop();
}

void ImageScene::onAutoScrollTimeout()
{
    if ((cursorMode != CursorMode::crop) || !leftMouseButtonPressed)
    {
        stopAutoScroll();
        return;
    }

    QGraphicsView* view = views()[0];
    QScrollBar* hBar = view->horizontalScrollBar();
    QScrollBar* vBar = view->verticalScrollBar();
    hBar->setValue(hBar->value() + autoScrollSpeed.x());
    vBar->setValue(vBar->value() + autoScrollSpeed.y());

    // The mouse may not have moved at all (the view is scrolling underneath a stationary cursor),
    // so re-derive its current scene position rather than relying on the last mouse move event
    const QPoint viewportPos = view->viewport()->mapFromGlobal(QCursor::pos());
    updateCropDrag(view->mapToScene(viewportPos));

    // Stop scrolling in any direction where the scrollbar has reached its limit
    if (((autoScrollSpeed.x() > 0) && (hBar->value() >= hBar->maximum())) ||
        ((autoScrollSpeed.x() < 0) && (hBar->value() <= hBar->minimum())))
        autoScrollSpeed.setX(0);
    if (((autoScrollSpeed.y() > 0) && (vBar->value() >= vBar->maximum())) ||
        ((autoScrollSpeed.y() < 0) && (vBar->value() <= vBar->minimum())))
        autoScrollSpeed.setY(0);
    if (autoScrollSpeed.isNull()) stopAutoScroll();
}

void ImageScene::setImage(const QImage& image)
{
    if (pixmapItemMain) pixmapItemMain->setPixmap(QPixmap::fromImage(image));
    else pixmapItemMain = addPixmap(QPixmap::fromImage(image));
    pixmapItemMain->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setSceneRect(image.rect());
}
