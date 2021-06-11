#include "beadpatternfileio.h"

int BeadPatternFileIO::load(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    quint32 magic;
    in >> magic;
    if (magic != 0xA0B0C0D0) throw(tr("Cannot open %1, not a valid bead pattern file").arg(QDir::toNativeSeparators(fileName)));

    //Read header
    qint32 version;
    qint32 pixelsPerBead;
    qint32 width;
    qint32 height;
    quint32 paletteSize;
    in >> version >> pixelsPerBead >> width >> height >> paletteSize;

    if (pixelsPerBead <= 0 || width <= 0 || height <= 0 || paletteSize <= 0) throw(tr("Cannot open %1, invalid header data").arg(QDir::toNativeSeparators(fileName)));

    QImage newImage(width, height, QImage::Format_ARGB32);
    std::map<quint16, BeadID> paletteMap;

    //Read bead palette
    for (quint32 i = 0 ; i < paletteSize ; ++i)
    {
        quint16 idx;
        QString brand;
        QString code;
        QString name;
        quint32 count;
        in >> idx >> brand >> code >> name >> count;
        BeadID id;
        beadTable.find(id, brand.toStdString(), code.toStdString(), name.toStdString());
        beadTable[id].setCount(int(count));
        paletteMap[idx] = id;
    }
    //Read compressed bead data
    qint32 compressedSize;
    in >> compressedSize;
    QByteArray compressedImgData;
    compressedImgData.resize(compressedSize);
    int bytesRead = in.readRawData(compressedImgData.data(), compressedSize);
    if (bytesRead != compressedSize) throw(tr("Cannot open %1, compressed bead data not of expected length").arg(QDir::toNativeSeparators(fileName)));
    //Create bead image from uncompressed bead data
    auto imgData = qUncompress(compressedImgData);
    QDataStream imgStream(imgData);
    imgStream.setByteOrder(QDataStream::LittleEndian);
    for (int y = 0; y < newImage.height(); ++y)
    {
        auto line = reinterpret_cast<QRgb*>(newImage.scanLine(y));
        for (int x = 0; x < newImage.width(); ++x)
        {
            quint16 beadIdx;
            imgStream >> beadIdx;
            if (beadIdx == transparent) line[x] = qRgba(0,0,0,0);
            else line[x] = beadTable[paletteMap[beadIdx]].getRGB();
        }
    }
    //Read original RGB color PNG image
    in >> origColorImage;
    image = newImage.copy();

    return pixelsPerBead;
}

void BeadPatternFileIO::save(const QString &fileName, int pixelsPerBead)
{
    if (image.isNull() || origColorImage.isNull()) return;
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // Write header
    out << quint32(0xA0B0C0D0); //"Magic" number identifying file format
    out << qint32(100); //File format version

    out.setVersion(QDataStream::Qt_5_0);

    out << qint32(pixelsPerBead) << qint32(image.width()) << qint32(image.height());

    //Create and write bead palette
    std::map<BeadID, int> beadPalette;
    beadTable.createPalette(beadPalette);
    out << quint32(beadPalette.size());
    for (const auto& beads : beadPalette)
    {
        out << quint16(beads.second) << QString::fromStdString(beads.first.brand) << beadTable[beads.first]; //outputs index, brand, code, name, and count
    }
    //Write bead data to byte array and compress
    QByteArray imgData;
    QDataStream imgStream(&imgData, QIODevice::WriteOnly);
    imgStream.setByteOrder(QDataStream::LittleEndian);
    for (int y = 0; y < image.height(); ++y)
    {
        auto line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x)
        {
            BeadID matchID;
            if (qAlpha(line[x]) == 0)
            {
                imgStream << quint16(transparent); //Fully transparent pixel
            }
            else
            {
                beadTable.findClosestColor(line[x], matchID);
                imgStream << quint16(beadPalette[matchID]);
            }
        }
    }
    auto compressedImgData = qCompress(imgData);
    out << qint32(compressedImgData.size());
    out.writeRawData(compressedImgData.data(), compressedImgData.size());
    //Write original RGB color PNG image
    out << origColorImage;
}
