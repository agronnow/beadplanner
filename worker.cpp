#include "worker.h"

void Worker::convertToBeadColors(QImage& recoloredImage, BeadColorTable& beadTable, bool countOnly, bool triggerUpdate, bool openList)
{
    int linesdone = 0;
    int progress = 0;
    beadTable.resetCounts();
    BeadID mindistID;
#pragma omp parallel for private(mindistID)
    for (int y = 0; y < recoloredImage.height(); ++y)
    {
        QRgb* line;
        #pragma omp critical
        line = reinterpret_cast<QRgb*>(recoloredImage.scanLine(y));
        BeadID prevID;

        QRgb prevRGB(0);
        for (int x = 0; x < recoloredImage.width(); ++x)
        {
            QRgb pixelRGB = line[x];
            if (qAlpha(pixelRGB) == 0) continue;
            if ((x > 0) && (pixelRGB == prevRGB)) mindistID = prevID;
            else beadTable.findClosestColor(pixelRGB, mindistID);
            #pragma omp critical
            {
                if (!countOnly) line[x] = beadTable[mindistID].getRGB();
                beadTable[mindistID].increment();
            }

            prevID = mindistID;
            prevRGB = pixelRGB;
        }
        if (omp_get_thread_num() == 0)
        {
            linesdone++;
            if (linesdone >= recoloredImage.height()/omp_get_num_threads()/20)
            {
                progress += int(100*double(linesdone*omp_get_num_threads())/recoloredImage.height());
                emit updateProgress(progress);
                linesdone = 0;
            }
        }
    }
    emit convertColorsDone(triggerUpdate, openList);
}
