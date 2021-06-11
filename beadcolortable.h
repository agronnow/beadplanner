#ifndef BEADCOLORTABLE_H
#define BEADCOLORTABLE_H

#include <map>
#include <vector>
#include <cmath>
#include <optional>
#include <QtXml>
#include <QFile>
#include "beadcolor.h"
#include "beadcoloritem.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <stdexcept>

enum class colordistanceMeasure {CIEDE2000, redmean, rgbdist};

//CIELAB L*a*b* color
struct CIELAB
{
    CIELAB(QRgb); //Create CIELAB from RGB
    double L;
    double a;
    double b;
};

using BeadColorMap = std::map<std::string, std::vector<BeadColor>>;

//Database class for bead colors initialized from an XML file
//Functions as a wrapper for std::map with bead colors organized by brand
//as well as providing methods for comparing bead colors to RGB colors
//and creating tables, trees, and color palettes from the map, etc.
class BeadColorTable
{
public:
    BeadColorTable() {}
    BeadColorTable(const QString&, const QString&);
    std::vector<BeadColor>& operator[](const std::string& key) {return map.at(key);}
    BeadColor& operator[](const BeadColorItem* item) {return map.at(item->getKey())[item->getIdx()];}
    const BeadColor& operator[](const BeadColorItem* item) const {return map.at(item->getKey())[item->getIdx()];}
    BeadColor& operator[](const BeadID& id) {return map.at(id.brand)[id.idx];}
    const BeadColor& operator[](const BeadID& id) const {return map.at(id.brand)[id.idx];}
    void createSeparateTables(BeadColorTable&, BeadColorTable&) const;
    void mergeTables(BeadColorTable&, BeadColorTable&);
    void loadXML(const QString&, bool);
    void saveXML(const QString&) const;
    double findClosestColor(QRgb, BeadID&) const;
    void stringOfMatches(QRgb, std::string&) const;
    void createTree(BeadColorItem*) const;
    void createTable(BeadColorItem*) const;
    void createTable(BeadColorItem*, QRgb) const;
    void createPalette(std::map<BeadID, int>&) const;
    std::size_t changeKey(const BeadID&, const std::string&);
    bool find(BeadID&, const std::string&, const std::string&, const std::string&) const;
    void insert(const std::string& key, BeadColor col) {map[key].push_back(col);}
    void remove(const BeadID& id);
    void resetCounts();
    bool hasDefault(const std::string& key) const {return bool(keysContainingDefault.count(key));}
    bool hasCustom(const std::string& key) const {return bool(keysContainingCustom.count(key));}
    void createKeyList(QStringList& list) const {for(auto const& cols: map)  list.push_back(QString::fromStdString(cols.first));}
    void setColorDistanceMeasure(colordistanceMeasure m) {measure = m;}

private:
    double colorDifference(const BeadColor&, QRgb) const;
    double getCIEDE2000(const CIELAB&, const CIELAB&) const;
    BeadColorMap map; //Map of vectors whose keys are different bead brands
    std::unordered_set<std::string> keysContainingCustom;
    std::unordered_set<std::string> keysContainingDefault;
    colordistanceMeasure measure = colordistanceMeasure::CIEDE2000;
};

#endif // BEADCOLORTABLE_H
