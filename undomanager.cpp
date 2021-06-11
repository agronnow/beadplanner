#include "undomanager.h"

UndoManager::UndoManager()
{
}

double UndoManager::undoRedo(QImage& img, QImage& origImg, bool undo)
{
    if (!image.isNull())
    {
        auto tmpImg = img.copy();
        auto tmpOrigImg = origImg.copy();
        img = image.copy();
        origImg = origImage.copy();
        image = tmpImg.copy();
        origImage = tmpOrigImg.copy();
        inRedoMode = undo;
    }
    return scaleFactor;
}
void UndoManager::setUndo(const QString& undoLabel, const QImage& img, const QImage& origImg, double scalefac)
{
    label = undoLabel;
    image = img.copy();
    origImage = origImg.copy();
    scaleFactor = scalefac;
    inRedoMode = false;
}
