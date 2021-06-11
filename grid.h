#ifndef GRID_H
#define GRID_H

#include "beadpixelcoordtransform.h"

enum class units {pixels, beads};
enum class GridStyle {dots, dashed};

class Grid
{
public:
    Grid(const GridStyle s) : style{s}
    {
        if (style == GridStyle::dots)
        {
            deltax = 1;
            deltay = 1;
        }
    }
    QSize getDelta() const {return QSize(deltax, deltay);}
    QPoint getOffset() const {return QPoint(offsetx, offsety);}
    int getDeltaX() const {return deltax;}
    int getDeltaY() const {return deltay;}
    int getOffsetX() const {return offsetx;}
    int getOffsetY() const {return offsety;}
    void setDeltaX(int dx) {deltax = dx;}
    void setDeltaY(int dy) {deltay = dy;}
    void setOffsetX(int ox) {offsetx = ox;}
    void setOffsetY(int oy) {offsety = oy;}
    bool isVisible() const {return visible;}
    void setVisible(bool v) {visible = v;}
    GridStyle getStyle() const {return style;}
    int getMinSize() const {return minSize;}
private:
    int deltax = 29; //Size of a standard midi bead plate
    int deltay = 29; //Size of a standard midi bead plate
    int offsetx = 0;
    int offsety = 0;
    int minSize = 4;
    GridStyle style;
    bool visible = true;
};

#endif // GRID_H
