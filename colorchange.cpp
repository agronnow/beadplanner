#include "colorchange.h"

void ColorChange::apply(const QImage* imgOrig, QImage& imgDest)
{
    if (newColor)
    {
        if (newColor.value() == origColor) return;
    }
    if (action == ColorReplaceAction::floodfill) floodFill(imgOrig, imgDest);
    else if (action == ColorReplaceAction::replace_single) changeBead(imgDest);
    else replaceColor(imgOrig, imgDest);
}

bool ColorChange::withinTolerance(QRgb otherCol) const
{
    return ((std::abs(qRed(origColor) - qRed(otherCol)) <= tolerance) && (std::abs(qGreen(origColor) - qGreen(otherCol)) <= tolerance) &&
            (std::abs(qBlue(origColor) - qBlue(otherCol)) <= tolerance) && (qAlpha(otherCol)>0));
}

//Replace color of a single bead (this can be multiple pixels)
void ColorChange::changeBead(QImage& img)
{
    if (!newColor) return;
    auto painter = new QPainter;
    painter->begin(&img);
    painter->setBrush(QBrush(newColor.value()));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRect(origin, QSize(pixelsperbead, pixelsperbead)));
    painter->end();
    delete painter;
    numReplaced++;
}

//Replace all colors within tolerance
void ColorChange::replaceColor(const QImage* imgOrig, QImage& imgDest)
{
    for (int y = 0; y < imgDest.height(); ++y)
    {
        QRgb* line;
        line = reinterpret_cast<QRgb*>(imgDest.scanLine(y));
        const QRgb* lineCompare = imgOrig ? reinterpret_cast<const QRgb*>(imgOrig->scanLine(y)) : line;
        for (int x = 0; x < imgDest.width(); ++x)
        {
            QRgb pixelRGB = lineCompare[x];
            if (withinTolerance(pixelRGB))
            {
                line[x] = newColor.value_or(qRgba(qRed(pixelRGB), qGreen(pixelRGB), qBlue(pixelRGB), 0));
                numReplaced++;
            }
        }
    }
}

//Flood fill using scan line and stack based algorithm
void ColorChange::floodFill(const QImage* imgOrig, QImage& imgDest)
{
    auto line = reinterpret_cast<QRgb*>(imgDest.scanLine(origin.y()));
    const QRgb* lineCompare = imgOrig ? reinterpret_cast<const QRgb*>(imgOrig->scanLine(origin.y())) : line;
    QRgb pixelRGB = lineCompare[origin.x()];
    line[origin.x()] = newColor.value_or(qRgba(qRed(pixelRGB), qGreen(pixelRGB), qBlue(pixelRGB), 0));

    std::stack<Span> stack;
    stack.push({origin.x(), origin.y()});

    //It's necessary to keep track of recolored pixels when the image being recolored isn't the same as the image used for color
    //comparison or if the new color is within the tolerance of the old color, otherwise we will get stuck in an infinite loop
    std::vector<bool> recolored(std::size_t(imgDest.width()*imgDest.height()), false);

    do
    {
        auto span = std::move(stack.top());
        stack.pop();

        line = reinterpret_cast<QRgb*>(imgDest.scanLine(span.y));
        const QRgb* lineCompare = imgOrig ? reinterpret_cast<const QRgb*>(imgOrig->scanLine(span.y)) : line;
        const QImage* imgCheck = imgOrig ? imgOrig : &imgDest;
        while(span.x > 0 && withinTolerance(lineCompare[span.x-1]) && !recolored[(span.x-1)+span.y*imgDest.width()]) span.x--;
        bool spanAbove = false;
        bool spanBelow = false;
        while(span.x < imgDest.width() && withinTolerance(lineCompare[span.x]) && !recolored[span.x+span.y*imgDest.width()])
        {
            line[span.x] = newColor.value_or(qRgba(qRed(lineCompare[span.x]), qGreen(lineCompare[span.x]), qBlue(lineCompare[span.x]), 0));
            recolored[span.x+span.y*imgDest.width()] = true;
            numReplaced++;
            if (span.y > 0) checkSpan(imgCheck, stack, span-1, recolored, spanAbove);
            if (span.y < imgDest.height() - 1) checkSpan(imgCheck, stack, span+1, recolored, spanBelow);
            span.x++;
        }
    } while (!stack.empty());
}

//Add new line segment to flood fill stack if necessary
void ColorChange::checkSpan(const QImage* img, std::stack<Span>& stack, Span span, const std::vector<bool>& recolored, bool& nextLine) const
{
    auto line_m = reinterpret_cast<const QRgb*>(img->scanLine(span.y));
    if(!nextLine && (withinTolerance(line_m[span.x]) && !recolored[span.x+span.y*img->width()]))
    {
      stack.push(span);
      nextLine = true;
    }
    else if(nextLine && !(withinTolerance(line_m[span.x]) && !recolored[span.x+span.y*img->width()]))
    {
      nextLine = false;
    }
}
