#ifndef WORKER_H
#define WORKER_H
#include <QImage>
#include <omp.h>
#include "beadcolortable.h"

//Worker for performing (potentially) expensive color conversion
//Meant to be used in a separate thread such that the main window's progress bar correctly updates
//Uses openmp to convert parts of the image in parallel
class Worker : public QObject
{
    Q_OBJECT

public:
    Worker() {}

public slots:
    void convertToBeadColors(QImage&, BeadColorTable&, bool, bool, bool);

signals:
    void updateProgress(int);
    void convertColorsDone(bool, bool);
};

#endif // WORKER_H
