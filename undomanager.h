#ifndef UNDO_H
#define UNDO_H
#include <QWidget>
#include <QString>

class UndoManager : public QObject
{
    Q_OBJECT
public:
    UndoManager();
    void clearUndo() {image = QImage();}
    double undoRedo(QImage&, QImage&, bool);
    void setUndo(const QString&, const QImage&, const QImage&, double);
    bool enabled() const {return !image.isNull();}
private:
    QImage image;
    QImage origImage;
    QString label;
    bool inRedoMode = false;
    double scaleFactor;
};

#endif // UNDO_H
