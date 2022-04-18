#include "beadcolortable.h"

CIELAB::CIELAB(QRgb color)
{
    const double X_ref = 95.047;
    const double Y_ref = 100.0;
    const double Z_ref = 108.883;

    double var_R = qRed(color)/255.0;
    double var_G = qGreen(color)/255.0;
    double var_B = qBlue(color)/255.0;

    if ( var_R > 0.04045 ) var_R = std::pow( ( var_R + 0.055 ) / 1.055, 2.4);
    else                   var_R = var_R / 12.92;
    if ( var_G > 0.04045 ) var_G = std::pow( ( var_G + 0.055 ) / 1.055, 2.4);
    else                   var_G = var_G / 12.92;
    if ( var_B > 0.04045 ) var_B = std::pow( ( var_B + 0.055 ) / 1.055, 2.4);
    else                   var_B = var_B / 12.92;

    var_R *= 100;
    var_G *= 100;
    var_B *= 100;

    double var_X = (var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805)/X_ref;
    double var_Y = (var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722)/Y_ref;
    double var_Z = (var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505)/Z_ref;

    if ( var_X > 0.008856 ) var_X = std::pow(var_X, 0.33333);
    else                    var_X = 7.787*var_X + 0.13793103448;
    if ( var_Y > 0.008856 ) var_Y = std::pow(var_Y, 0.33333);
    else                    var_Y = 7.787*var_Y + 0.13793103448;
    if ( var_Z > 0.008856 ) var_Z = std::pow(var_Z, 0.33333);
    else                    var_Z = 7.787*var_Z + 0.13793103448;

    L = 116*var_Y - 16.0;
    a = 500*(var_X - var_Y);
    b = 200*(var_Y - var_Z);
}

BeadColorTable::BeadColorTable(const QString& defaultXmlFile, const QString& customXmlFile)
{
    loadXML(defaultXmlFile, false, false);
    loadXML(customXmlFile, true, false);
}

//Construct table of bead colors from XML file
//This table is a map of vectors whose keys are different bead brands
void BeadColorTable::loadXML(const QString& xmlInfo, bool custom, bool update)
{
    QXmlStreamReader xmlReader;
    QFile xmlFile;
    BeadColorMap newmap;
    if (update) xmlReader.addData(xmlInfo);
    else
    {
        xmlFile.setFileName(xmlInfo);
        std::cout << xmlInfo.toStdString() << std::endl;
        if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
                throw("Failed to open file "+xmlInfo);
        }
        xmlReader.setDevice(&xmlFile);
    }

    //Parse the XML until we reach end of it
    while(!xmlReader.atEnd() && !xmlReader.hasError())
    {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if (token == QXmlStreamReader::StartDocument) continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xmlReader.name() == "Version") {version = xmlReader.readElementText();std::cout << "read version: " << version.toStdString() << std::endl;}
            if (xmlReader.name() == "Color")
            {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                if (attributes.hasAttribute("name") && attributes.hasAttribute("type") && attributes.hasAttribute("code") &&
                        attributes.hasAttribute("red") && attributes.hasAttribute("green") && attributes.hasAttribute("blue"))
                {
                    auto brand = attributes.value("type").toString().toStdString();
                    bool enabled = false;
                    if (attributes.hasAttribute("enabled")) enabled = (attributes.value("enabled").toString().toStdString() == "true");
                    if (update)
                    {
                        bool found = false;
                        auto code = attributes.value("code").toString().toStdString();
                        auto name = attributes.value("name").toString().toStdString();
                        auto oldBrand = map.find(brand);
                        if (oldBrand != map.end())
                        {
                            auto match = std::find_if(oldBrand->second.begin(), oldBrand->second.end(), [brand,code,name](BeadColor col){return (col.getCode() == code && col.getName() == name);});
                            if (match != oldBrand->second.end())
                            {
                                newmap[brand].emplace_back(BeadColor(code, name, attributes.value("source").toString().toStdString(),
                                                                  qRgb(attributes.value("red").toInt(), attributes.value("green").toInt(), attributes.value("blue").toInt()),
                                                                  match->isEnabled(), custom, match->getOwned(), match->getUsed()));
                                found = true;
                            }
                        }
                        if (!found) newmap[brand].emplace_back(BeadColor(code, name, attributes.value("source").toString().toStdString(),
                                                                      qRgb(attributes.value("red").toInt(), attributes.value("green").toInt(), attributes.value("blue").toInt()),
                                                                      enabled, custom, attributes.value("owned").toInt(), attributes.value("used").toInt()));
                    }
                    else
                    {
                        map[brand].emplace_back(BeadColor(attributes.value("code").toString().toStdString(), attributes.value("name").toString().toStdString(),
                                                          attributes.value("source").toString().toStdString(),
                                                          qRgb(attributes.value("red").toInt(), attributes.value("green").toInt(), attributes.value("blue").toInt()),
                                                          enabled, custom, attributes.value("owned").toInt(), attributes.value("used").toInt()));
                    }
                    //if (custom) keysContainingCustom.emplace(brand);
                    //else keysContainingDefault.emplace(brand);
               }
            }
        }
    }
    if (update) map = newmap;
    else xmlFile.close();

    if(xmlReader.hasError())
    {
        throw("XML Parse Error: "+xmlReader.errorString());
    }
}

//Save table of bead colors to XML file
void BeadColorTable::saveXML(const QString& xmlFileName, bool writeVersion) const
{
    QFile xmlFile(xmlFileName);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
            throw("Failed to open file "+xmlFileName);
    }

    QXmlStreamWriter xmlWriter(&xmlFile);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Table");
    if (writeVersion) xmlWriter.writeTextElement("Version", version);

    xmlWriter.writeStartElement("Colors");

    for (const auto& cols : map)
    {
        for (const auto& col : cols.second)
        {
            xmlWriter.writeStartElement("Color");
            xmlWriter.writeAttribute("code", QString::fromStdString(col.getCode()));
            xmlWriter.writeAttribute("name", QString::fromStdString(col.getName()));
            xmlWriter.writeAttribute("red", QString::number(qRed(col.getRGB())));
            xmlWriter.writeAttribute("green", QString::number(qGreen(col.getRGB())));
            xmlWriter.writeAttribute("blue", QString::number(qBlue(col.getRGB())));
            xmlWriter.writeAttribute("type", QString::fromStdString(cols.first));
            xmlWriter.writeAttribute("owned", QString::number(col.getOwned()));
            xmlWriter.writeAttribute("used", QString::number(col.getUsed()));
            xmlWriter.writeAttribute("enabled", col.isEnabled() ? "true" : "false");
            xmlWriter.writeAttribute("source", QString::fromStdString(col.getSource()));
            xmlWriter.writeEndElement();
        }
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    xmlFile.close();
}

//Return squared color difference between two colors using chosen algorithm
double BeadColorTable::colorDifference(const BeadColor& beadCol, QRgb color2) const
{
    double dist = 0.0;
    QRgb color1 = beadCol.getRGB();
    if (color1 != color2)
    {
        if (measure == colordistanceMeasure::CIEDE2000) dist = getCIEDE2000(color1, color2);
        else if (measure == colordistanceMeasure::redmean)
        {
            const double redmean = (qRed(color1) + qRed(color2))/2.0;
            dist = (2.0 + redmean/256.0)*(qRed(color2)-qRed(color1))*(qRed(color2)-qRed(color1)) +
                    4*(qGreen(color2)-qGreen(color1))*(qGreen(color2)-qGreen(color1)) +
                    (2.0 + (255.0 - redmean)/256.0)*(qBlue(color2)-qBlue(color1))*(qBlue(color2)-qBlue(color1));
        }
        else //rgbdist
        {
            dist = (qRed(color1) - qRed(color2))*(qRed(color1) - qRed(color2)) +
                    (qGreen(color1) - qGreen(color2))*(qGreen(color1) - qGreen(color2)) + (qBlue(color1) - qBlue(color2))*(qBlue(color1) - qBlue(color2));
        }
    }
    return dist;
}

//Return square of CIEDE2000 DeltaE_00 color difference of two CIELAB colors
double BeadColorTable::getCIEDE2000(const CIELAB& lab1, const CIELAB& lab2) const
{
    constexpr double pi = 3.14159265359;
    const double Cbarast7 = std::pow((std::sqrt(lab1.a*lab1.a+lab1.b*lab1.b) + std::sqrt(lab2.a*lab2.a+lab2.b*lab2.b))/2.0, 7.0);
    const double G = 0.5*(1.0 - std::sqrt(Cbarast7/(Cbarast7 + 6103515625.0)));
    const double aprime1 = (1.0 + G)*lab1.a;
    const double aprime2 = (1.0 + G)*lab2.a;
    const double Cprime1 = std::sqrt(aprime1*aprime1 + lab1.b*lab1.b);
    const double Cprime2 = std::sqrt(aprime2*aprime2 + lab2.b*lab2.b);
    double h1 = (lab1.b == lab1.a) && (lab1.a == 0.0) ? 0.0 : std::atan2(lab1.b,aprime1)*180.0/pi;
    double h2 = (lab2.b == lab2.a) && (lab2.a == 0.0) ? 0.0 : std::atan2(lab2.b,aprime2)*180.0/pi;
    if (h1 < 0.0) h1 += 360.0;
    if (h2 < 0.0) h2 += 360.0;
    double deltah = 0.0;
    if (Cprime1*Cprime2 != 0.0)
    {
        if (std::abs(h2-h1) <= 180.0) deltah = h2 - h1;
        else if (h2 - h1 > 180.0) deltah = h2 - h1 - 360.0;
        else deltah = h2 - h1 + 360.0;
    }
    const double deltaHue = 2.0*std::sqrt(Cprime1*Cprime2)*std::sin(deltah/2.0*(pi/180.0));
    double hbar = (h1 + h2)/2.0;
    if (Cprime1*Cprime2 != 0.0)
    {
        if ((std::abs(h1 - h2) > 180.0) && (h1 + h2 < 360.0)) hbar += 180.0;
        else if ((std::abs(h1 - h2) > 180.0) && (h1 + h2 >= 360.0)) hbar -= 180.0;
    }
    else hbar *= 2.0;
    const double T = 1.0 - 0.17*std::cos((hbar - 30.0)*pi/180.0) + 0.24*std::cos(2.0*hbar*pi/180.0) + 0.32*std::cos((3.0*hbar + 6.0)*pi/180.0) - 0.2*std::cos((4.0*hbar - 63.0)*pi/180.0);
    const double Lbar = (lab1.L + lab2.L)/2.0;
    const double Cbar = (Cprime1+Cprime2)/2.0;
    const double Lm50sq = (Lbar - 50.0)*(Lbar - 50.0);
    const double Cbar7 = std::pow(Cbar, 7.0);
    const double RC = 2.0*std::sqrt(Cbar7/(Cbar7 + 6103515625.0));
    const double SL = 1.0 + (0.015*Lm50sq)/std::sqrt(20.0+Lm50sq);
    const double SC = 1.0 + 0.045*Cbar;
    const double SH = 1.0 + 0.015*Cbar*T;
    const double RT = -std::sin(60.0*std::exp(-((hbar-275.0)/25.0)*((hbar-275.0)/25.0))*pi/180.0)*RC;
    const double distsq = ((lab2.L - lab1.L)/SL)*((lab2.L - lab1.L)/SL) + ((Cprime2 - Cprime1)/SC)*((Cprime2 - Cprime1)/SC) + (deltaHue/SH)*(deltaHue/SH) + RT*((Cprime2 - Cprime1)/SC)*(deltaHue/SH);

    return distsq;
}

double BeadColorTable::findClosestColor(QRgb pixelRGB, BeadID& mindist) const
{
    bool exactMatchFound = false;
    double mindistval = 1.e10;
    for (const auto &cols : map)
    {
        if (exactMatchFound) break;
        for (std::size_t i = 0; i != cols.second.size() ; ++i)
        {
            if (cols.second[i].isEnabled())
            {
                double dist = colorDifference(cols.second[i], pixelRGB);
                if (dist < mindistval)
                {
                    mindistval = dist;
                    mindist.brand = cols.first;
                    mindist.idx = i;
                    if (dist == 0.0)
                    {
                        exactMatchFound = true;
                        break;
                    }
                }
            }
        }
    }
    return mindistval;
}

void BeadColorTable::stringOfMatches(QRgb pixelRGB, std::string& matches) const
{
    for (const auto& cols : map)
    {
        for (const auto& col : cols.second)
        {
            if (col == pixelRGB) matches += cols.first + ": " + col.getName() + "\n";
        }
    }
    if (!matches.empty()) matches = matches.substr(0,matches.size()-1); //remove trailing newline
}

//Creates two tables containing only the default and custom bead colors
void BeadColorTable::createSeparateTables(BeadColorTable& defaultTable, BeadColorTable& customTable) const
{
    defaultTable.setVersion(version);
    for (const auto& cols : map)
    {
        for (const auto& col : cols.second)
        {
            if (col.isCustom()) customTable.map[cols.first].push_back(col);
            else defaultTable.map[cols.first].push_back(col);
        }
    }
}

//Sets this table to be the union of two given tables with the version set to that of the first table
void BeadColorTable::mergeTables(BeadColorTable& firstTable, BeadColorTable& secondTable)
{
    version = firstTable.getVersion();
    map = firstTable.map;
    for (auto& col : secondTable.map)
    {
        auto ins = map.insert(col); //This will insert the vector with key if this key is not already in the map
        if (!ins.second) //Key is already in the map, append to the existing vector instead
        {
            ins.first->second.insert(ins.first->second.end(),
                                     std::make_move_iterator(col.second.begin()), std::make_move_iterator(col.second.end()));
        }
    }
}

//Fill rootItem with tree of bead colors
//This is a 2-level tree with bead colors organized by bead brands
void BeadColorTable::createTree(BeadColorItem* rootItem) const
{
    for (const auto& cols : map)
    {
        rootItem->appendChild(new BeadColorItem(cols.first, 0, Qt::Unchecked, rootItem));
        auto parentItem = rootItem->child(rootItem->childCount()-1);
        std::size_t nChecked = 0;
        for (auto& col: cols.second)
        {
                if (col.isEnabled()) nChecked++;
        }
        if (nChecked == cols.second.size()) parentItem->setChecked(Qt::Checked);
        else if (nChecked == 0) parentItem->setChecked(Qt::Unchecked);
        else parentItem->setChecked(Qt::PartiallyChecked);

        for (std::size_t i = 0; i != cols.second.size(); ++i)
        {
            parentItem->appendChild(new BeadColorItem(cols.first, i, cols.second[i].isEnabled() ? Qt::Checked : Qt::Unchecked, parentItem));
        }
    }
}


//Fill rootItem with table of bead counts
void BeadColorTable::createTable(BeadColorItem* rootItem) const
{
    for (const auto& cols : map)
    {
        for (std::size_t i = 0; i != cols.second.size(); ++i)
        {
            if (cols.second[i].isEnabled()) rootItem->appendChild(new BeadColorItem(cols.first, i, rootItem));
        }
    }
}

//Fill rootItem with table of bead color differences from pixelCol
void BeadColorTable::createTable(BeadColorItem* rootItem, QRgb pixelCol) const
{
    for (const auto& cols : map)
    {
        for (std::size_t i = 0; i != cols.second.size(); ++i)
        {
            if (cols.second[i].isEnabled())
            {
                double dist = std::sqrt(colorDifference(cols.second[i], pixelCol));
                rootItem->appendChild(new BeadColorItem(cols.first, i, dist, rootItem));
            }
        }
    }
}

//Creates a map associating the ID of each bead color present in the image with a unique integer index
void BeadColorTable::createPalette(std::map<BeadID, int>& palette) const
{
    int nBeads = 0;
    for (const auto& cols : map)
    {
        for (std::size_t i = 0; i != cols.second.size(); ++i)
        {
            if (cols.second[i].getCount() > 0)
            {
                palette[{cols.first, i}] = nBeads;
                ++nBeads;
            }
        }
    }
}

//Searches for bead color based on brand, code, and name. If found, returns true and sets "id" to the ID of the first match, otherwise returns false.
bool BeadColorTable::find(BeadID& id, const std::string& brand, const std::string& code, const std::string& name) const
{
    for (const auto& cols : map)
    {
        auto match = std::find_if(cols.second.begin(), cols.second.end(), [cols,brand,code,name](BeadColor col){return (cols.first == brand && col.getCode() == code && col.getName() == name);});
        if (match != cols.second.end())
        {
            id = {cols.first, std::size_t(std::distance(cols.second.begin(), match))};
            return true;
        }
    }
    return false;
}

//Changes the key of the element (i.e. the bead's brand), removing it from the old key vector and appending to, or creating a new, vector for new key
//Returns index of element within new key vector
std::size_t BeadColorTable::changeKey(const BeadID& id, const std::string& newKey)
{
    if (newKey == id.brand) return id.idx;
    auto oldKeyVector = map.find(id.brand);
    if (oldKeyVector != map.end())
    {
        //if (oldKeyVector->second[id.idx].isCustom()) keysContainingCustom.emplace(newKey);
        //else keysContainingDefault.emplace(newKey);
        map[newKey].push_back(oldKeyVector->second[id.idx]); //Insert under new key
        oldKeyVector->second.erase(oldKeyVector->second.begin() + int(id.idx)); //Remove from old key
        if (oldKeyVector->second.empty()) map.erase(id.brand); //Remove old key entirely if old key vector is now empty

        return std::size_t(std::max(int(map[newKey].size())-1, 0));
    }
    else return 0;
}

void BeadColorTable::remove(const BeadID& id)
{
    auto keyVector = map.find(id.brand);
    if (keyVector != map.end())
    {
        //bool custom = keyVector->second[id.idx].isCustom();
        keyVector->second.erase(keyVector->second.begin() + int(id.idx)); //Remove from old key
        if (keyVector->second.empty())
        {
            map.erase(id.brand); //Remove old key entirely if old key vector is now empty
            //if (custom) keysContainingCustom.erase(id.brand);
            //else keysContainingDefault.erase(id.brand);
        }
    }
}

void BeadColorTable::resetCounts()
{
    for (auto& cols : map)
    {
        for (auto& col : cols.second)
        {
            col.resetCount();
        }
    }
}
