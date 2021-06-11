#ifndef COLORCHANGE_H
#define COLORCHANGE_H

#include <QImage>
#include <QPainter>
#include <stack>
#include <optional>

enum class ColorReplaceAction{replace_single, replace_all, floodfill};

//Represents a single color replacement action which is performed on a given image by calling "apply"
//Can replace the color of a single bead at given position, replace all instances of a color within some tolerance,
//or flood fill starting at a given position with some color tolerance
//The new color is an optional parameter, if this isn't specified on construction the target pixels will be made transparent instead
//The change in the number of different bead colors caused by this replacement is kept track of
//For replace all and flood fill the QImage* parameter can be nullptr in which case the QImage& parameter is used for both color comparison
//and replacement. If QImage* is not nullptr this image is instead used for color comparison while only the QImage& is recolored.
class ColorChange
{
public:
    ColorChange() {}
    ColorChange(QRgb ocol, QPoint orig) : origColor{ocol}, origin{orig} {}
    ColorChange(QRgb ocol, QPoint orig, int tol) : origColor{ocol}, origin{orig}, tolerance{tol} {}
    ColorChange(QRgb ocol, QRgb ncol, QPoint orig, int ppb, bool floodfill) : origColor{ocol}, newColor{ncol}, origin{orig}, pixelsperbead{ppb}
    {
        if (floodfill) action = ColorReplaceAction::floodfill;
    }
    void apply(const QImage*, QImage&);
    void resetColorChange();
    QRgb getOrigColor() const {return origColor;}
    int getNumReplaced() const {return numReplaced;}
    void setTolerance(int tol) {tolerance = tol;}
    void setFloodFill(bool ff)
    {
        if (ff) action = ColorReplaceAction::floodfill;
        else action = ColorReplaceAction::replace_all;
    }
    void setAction(ColorReplaceAction act) {action = act;}

private:
    struct Span
    {
        Span operator+(int other)
        {
            return {x, y+other};
        }
        Span operator-(int other)
        {
            return {x, y-other};
        }
        int x;
        int y;
    };

    bool withinTolerance(QRgb) const;
    void changeBead(QImage&);
    void replaceColor(const QImage*, QImage&);
    void floodFill(const QImage*, QImage&);
    void checkSpan(const QImage*, std::stack<Span>&, Span, const std::vector<bool>&, bool&) const;

    ColorReplaceAction action = ColorReplaceAction::replace_all;
    QRgb origColor;
    std::optional<QRgb> newColor;
    QPoint origin;
    int pixelsperbead = 1;
    int numReplaced = 0;
    int tolerance = 0;
};

#endif // COLORCHANGE_H
