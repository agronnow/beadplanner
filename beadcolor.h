#ifndef BEADCOLOR_H
#define BEADCOLOR_H

#include <string>
#include <QRgb>
#include <QVariant>
#include <QDataStream>

//using BeadID = std::pair<std::string, std::size_t>;

//Identifies bead color by brand and index within the vector of that brand's colors
struct BeadID
{
    bool operator<(const BeadID &rhs) const
    {
       return ((brand <= rhs.brand) && (idx < rhs.idx));
    }
    std::string brand;
    std::size_t idx = 0;
};

Q_DECLARE_METATYPE(BeadID)

class BeadColor
{
public:
    BeadColor() {}
    BeadColor(bool cus) : custom{cus} {}
    BeadColor(std::string c, std::string n, std::string s, QRgb rgb, bool e, bool cus) :
        code{c}, name{n}, source{s}, colorRGB{rgb}, enabled{e}, custom{cus}, owned{0}, used{0} {}
    BeadColor(std::string c, std::string n, std::string s, QRgb rgb, bool e, bool cus, int o, int u) :
    code{c}, name{n}, source{s}, colorRGB{rgb}, enabled{e}, custom{cus}, owned{o}, used{u} {}
    bool operator==(const BeadColor& other)
    {
        return (name == other.name &&
        code == other.code &&
        source == other.source &&
        colorRGB == other.colorRGB &&
        custom == other.custom);
    }
    friend bool operator==(const BeadColor&, QRgb);
    friend bool operator==(QRgb, const BeadColor&);
    bool operator > (const BeadColor& other) const {return (count > other.count);}
    bool operator < (const BeadColor& other) const {return (count < other.count);}
    BeadColor& operator++()
    {
        ++count;
        return *this;
    }
    BeadColor& operator--()
    {
        count = std::max(count-1, 0);
        return *this;
    }
    BeadColor& operator+= (int num)
    {
        count += num;

        return *this;
    }
    BeadColor& operator-= (int num)
    {
        count = std::max(count-num, 0);

        return *this;
    }
    BeadColor& operator*= (int factor)
    {
        count *= factor;

        return *this;
    }
    friend QDataStream &operator<<(QDataStream &out, const BeadColor &rhs){
        out << QString::fromStdString(rhs.code) << QString::fromStdString(rhs.name) << quint32(rhs.count);
        return out;
    }
    QRgb getRGB() const {return colorRGB;}
    void setRGB(QRgb rgb) {colorRGB = rgb;}
    const std::string& getCode() const {return code;}
    void setCode(const std::string& c) {code = c;}
    const std::string& getName() const {return name;}
    void setName(const std::string& n) {name = n;}
    const std::string& getSource() const {return source;}
    void setSource(const std::string& s) {source = s;}
    bool isEnabled() const {return enabled;}
    void setEnabled(bool en) {enabled = en;}
    bool isCustom() const {return custom;}
    void setCount(int c) {count = c;}
    void increment() {count++;}
    void decrement() {if (count > 0) count--;}
    void resetCount() {count = 0;}
    int getCount() const {return count;}
    int getOwned() const {return owned;}
    int getUsed() const {return used;}
private:
    std::string code;
    std::string name;
    std::string source;
    QRgb colorRGB = 0;
    bool enabled = true;
    bool custom = false;
    int count = 0;
    int owned = 0; //Not currently used, only there to save extra info from XML file
    int used = 0;  //Not currently used, only there to save extra info from XML file
};

inline bool operator==(const BeadColor& lhs, QRgb rhs) {return lhs.colorRGB == rhs;}

inline bool operator==(QRgb lhs, const BeadColor& rhs) {return lhs == rhs.colorRGB;}

Q_DECLARE_METATYPE(BeadColor)


#endif // BEADCOLOR_H
