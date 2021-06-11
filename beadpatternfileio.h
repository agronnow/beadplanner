/*
 * The Bead Pattern File Format (.bptrn) mainly contains a palette of beads used in the pattern and
 * the bead color at each position in zlib compressed format, as well as the original RGB image in PNG format.
 * Each bead color is assumed to be fully specified by its brand, code, and name, which any valid bead color should have.
 * If multiple bead colors do share all of these properties whichever is first found by the BeadColorTable::find method
 * is used.
 *
 * File data structure:
 * TYPE     NAME                DESCRIPTION
 * --Header--
 * quint32  Magic number        Identifies the file as a valid bead pattern file, must be 0xA0B0C0D0
 * qint32   Version             Identifies the version of the bead pattern format of the file
 * qint32   Pixels per bead     Number of pixels per bead
 * qint32   Width               Width in pixels
 * qint32   Height              Height in pixels
 * quint32  Palette size        Number of different bead colors present in pattern
 * --Palette--
 * "Palette size" number of bead color entries, each containing the following:
 * quint16  Index               Index of palette entry, normally these just increase sequentially
 * QString  Brand               Brand of bead, e.g. "Hama"
 * QString  Code                Bead shorthand code, e.g. "H03"
 * QString  Name                Full bead name, e.g. "Cloudy White"
 * quint32  Count               Number of beads in pattern with this color
 * --zlib compressed bead data--
 * qint32   Compressed size     Number of following bytes of compressed bead data
 * quint16  Bead indexes        Palette index of each bead in pattern, going line by line from pixel position (0,0)
 *                              Note that fully transparent (alpha = 0) pixels have no valid bead color,
 *                              this is indicated with a bead index of 65535 (typically the max allowed value of quint16)
 * --PNG data--
 * Full PNG image data (including any metadata) as serialized by Qt representing the original RGB image before bead color conversion.
*/

#ifndef BEADPATTERNFILEIO_H
#define BEADPATTERNFILEIO_H

#include <QDataStream>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QImage>
#include "beadcolortable.h"

class BeadPatternFileIO : public QObject
{
    Q_OBJECT
public:
    BeadPatternFileIO(BeadColorTable& t, QImage& o, QImage& i) : beadTable{t}, origColorImage{o}, image{i} {}
    void save(const QString&, int);
    int load(const QString&);
private:
    static constexpr quint16 transparent = 65535; //Maximum possible value of unsigned short guaranteed to be at least this, used to indicate transparency

    BeadColorTable& beadTable;
    QImage& origColorImage;
    QImage& image;
};

#endif // BEADPATTERNFILEIO_H
