#include "imagescene.h"

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
                    QVarLengthArray<QLineF, 100> linesX;
                    for (double x = left + gridOffset.x(); x < pixmapItemMain->pixmap().width(); x += gridInterval.width())
                        linesX.append(QLineF(x, rect.top(), x, pixmapItemMain->pixmap().height()));

                    QVarLengthArray<QLineF, 100> linesY;
                    for (double y = top + gridOffset.y(); y < pixmapItemMain->pixmap().height(); y += gridInterval.height())
                            linesY.append(QLineF(rect.left(), y, pixmapItemMain->pixmap().width(), y));

                    painter->drawLines(linesX.data(), linesX.size());
                    painter->drawLines(linesY.data(), linesY.size());
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
                // With the left mouse button pressed, remember the position
                leftMouseButtonPressed = true;

                // Create a selection square
                if (!rubberBand) rubberBand = new QRubberBand(QRubberBand::Rectangle, views()[0]);
                origin = coords.snapToBeadCoord(event->scenePos());
                rubberBand->setGeometry(QRect(origin, QSize()));
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
        cursorMode = CursorMode::normal;
        if (rubberBand) rubberBand->hide();
        emit exitCursorSelectionMode();
    }

    QGraphicsScene::mousePressEvent(event);
}

void ImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (pixmapItemMain == nullptr) return;
    if (leftMouseButtonPressed && (cursorMode == CursorMode::crop))
    {
        // Form the selection area when moving with the mouse while pressing the LMB
        auto curPos = coords.snapToBeadCoord(event->scenePos());
        if (curPos.x() > pixmapItemMain->pixmap().width()) curPos.setX(pixmapItemMain->pixmap().width());
        if (curPos.y() > pixmapItemMain->pixmap().height()) curPos.setY(pixmapItemMain->pixmap().height());
        QRect cropRect(QRect(origin, curPos).normalized());
        if (cropRect.x() < 0) cropRect.setX(0);
        if (cropRect.y() < 0) cropRect.setY(0);
        rubberBand->setGeometry(cropRect);
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
            if (!rubberBand) rubberBand = new QRubberBand(QRubberBand::Rectangle, views()[0]);
            auto curPos = coords.snapToBeadCoord(event->scenePos());
            if (curPos.x() > pixmapItemMain->pixmap().width()) curPos.setX(pixmapItemMain->pixmap().width());
            if (curPos.y() > pixmapItemMain->pixmap().height()) curPos.setY(pixmapItemMain->pixmap().height());
            QRect cropRect(QRect(curPos, coords.beadToPixelCoord(pasteSize)).normalized());
            if (cropRect.x() < 0) cropRect.setX(0);
            if (cropRect.y() < 0) cropRect.setY(0);
            rubberBand->setGeometry(cropRect);
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

            // When releasing the LMB, we form the cropping area
            if (rubberBand->isVisible())
            {
                QRect selectionRect = rubberBand->geometry();
                const double scaleFactor = coords.getScaleFactor();
                if ((std::floor(selectionRect.width()/scaleFactor) > 1.0) && (std::floor(selectionRect.height()/scaleFactor) > 1.0))
                {
                    emit crop(selectionRect);
                    rubberBand->hide();
                    cursorMode = CursorMode::normal;
                    emit exitCursorSelectionMode();
                }
            }
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


void ImageScene::setImage(const QImage& image)
{
    if (pixmapItemMain) pixmapItemMain->setPixmap(QPixmap::fromImage(image));
    else pixmapItemMain = addPixmap(QPixmap::fromImage(image));
    pixmapItemMain->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setSceneRect(image.rect());
}
