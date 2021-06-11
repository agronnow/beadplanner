/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#include <QProgressBar>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QActionGroup>
#include <QTabBar>
#include <string>
#include <stack>
#include <iostream>
#include <vector>
#include "worker.h"

#include "beadcolortable.h"
#include "imagescene.h"
#include "griddialog.h"
#include "beadsizedialog.h"
#include "palettedialog.h"
#include "beadlistdialog.h"
#include "imagecolorsdialog.h"
#include "backgrounddialog.h"
#include "settingsdialog.h"
#include "sidebar.h"
#include "beadpatternfileio.h"
#include "colorchange.h"
#include "undomanager.h"

class QAction;
class QLabel;
class QMenu;

enum class zoomMode {absolute, relative, automatic};

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer()
    {
        thread->quit();
        thread->deleteLater();
    }
    bool loadFile(const QString&);

signals:
    void enterCursorSelectionMode(CursorMode);
    void updatePixelColorInfo(QRgb, const std::string&);

private slots:
    void open();
    void openRecentFile();
    void saveImageAs();
    void saveCurrentFile();
    void openBeadPattern();
    void loadBeadPattern(const QString&);
    void saveBeadPattern(const QString&);
    void saveBeadPatternAs();
    void print();
    void undoRedo(bool);
    void copy();
    void paste();
    void createFromClipboard();
    void pasteAt(QPoint);
    void configureGrid();
    void setBeadSize();
    void editPalette();
    void listBeadCounts();
    void scaleImage(double, zoomMode);
    void pixelate();
    void about();
    void convertToBeadColors();
    void restoreOrigColors();
    void convertColorsDone(bool, bool);
    void horizontalFlip();
    void crop(QRect&);
    void onExitCursorSelectionMode();
    void onCoordsChanged(QPoint, QPoint, QRgb);
    void onColorPickerClick(QPoint, QPoint);
    void setTransparency(QPoint, QRgb);
    void updateTransparency(ColorChange&, bool);
    void onReplaceColor(QPoint, QRgb, const BeadID&, ReplaceColorTarget, bool);
    void resetTransparentColor();
    void settings();
    void onTabChanged(int);
    void closeEvent(QCloseEvent*) override;

private:
    void createActions();
    void createMenus();
    void updateActions();
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    bool saveImage(const QString&);
    void setImage(const QImage&);
    void updateImage();
    void startCursorSelectionMode(CursorMode, const QString&);
    void setRecolored(bool);
    void recountBeads(bool);
    void readSettings();
    void updateSceneImage();
    void setUndoableAction(QAction*);
    bool fileModified() const {return (nActions != 0);}

    QThread* thread;
    Worker* worker;
    QImage image;
    QImage origColorImage;
    QImage zoomedImage;
    QImage previewImage;
    QWidget* centralWidget;
    QTabBar *imageTabs;
    ImageScene *scene;
    QGraphicsView *view;
    Sidebar *pixelInfo;
    QImage *bgbrushImage;
    QLabel *statusText;
    QLabel *zoomText;
    QProgressBar *progressBar;
    BeadColorTable beadTable;
    BeadColorTable origBeadTable;
    ColorChange transp;
    bool recolored = false;
    bool countsReliable = true;
    int nActions = 0;
    Settings curSettings;
    const double zoomMin = 0.25;
    const double zoomMax = 32.0;
    std::vector<double> zoomFactors;
    BeadPatternFileIO beadPattern;
    UndoManager undoMgr;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QMenu *zoomMenu;
    static constexpr std::size_t maxRecentFiles = 10;
    std::array<QAction*, maxRecentFiles> recentFilesActs;
    QStringList recentFiles;
    QMenu *recentMenu;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *saveBeadAct;
    QAction *printAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *setTransparentColorAct;
    QAction *replaceAct;
    QAction *flipAct;
    QAction *cropAct;
    QAction *pixelateAct;
    QAction *convertColorsAct;
    QAction *convertColorsRedmeanAct;
    QAction *convertColorsRestoreAct;
    QAction *listBeadCountAct;
    QAction *showGridAct;
    QAction *showDotsAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *zoomFitAct;
    std::vector<QAction*> zoomActs;
    QActionGroup *zoomGroup;
};


#endif
