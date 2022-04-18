#include <QtWidgets>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#endif

#include "imageviewer.h"
#include "worker.h"

ImageViewer::ImageViewer(QWidget *parent) try
   : QMainWindow(parent), beadTable("default_colors.xml", "custom_colors.xml"), beadPattern(beadTable, origColorImage, image)
{
    scene = new ImageScene(this);
    view = new QGraphicsView(this);
    statusText = new QLabel(this);
    zoomText = new QLabel(this);
    progressBar = new QProgressBar(this);
    pixelInfo = new Sidebar;

    centralWidget = new QWidget(this);

    auto *Vlayout = new QVBoxLayout;
    centralWidget->setLayout(Vlayout);
    imageTabs = new QTabBar;
    Vlayout->addWidget(imageTabs);
    imageTabs->addTab(tr("Original colours"));
    imageTabs->addTab(tr("Bead colours"));
    imageTabs->setTabEnabled(1, false);

    auto *Hlayout = new QHBoxLayout;
    Hlayout->addWidget(view);
    setWindowTitle(tr("Bead Planner"));
    view->setScene(scene);
    view->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    view->setMouseTracking(true);
    //layout->addWidget(view);
    setCentralWidget(centralWidget);
    Hlayout->setSizeConstraint(QLayout::SetMaximumSize);

    Hlayout->addWidget(pixelInfo);
    Vlayout->addLayout(Hlayout);

    bgbrushImage = new QImage(":/images/bgbrush.png");
    view->setBackgroundBrush(QBrush(QColor(160, 160, 160), Qt::DiagCrossPattern));
    //QImage curbgbrush = bgbrushImage->scaled(bgbrushImage->width()/16, bgbrushImage->height()/16, Qt::KeepAspectRatio, Qt::FastTransformation);
    //view->setBackgroundBrush(QPixmap::fromImage(curbgbrush));

    progressBar->setMinimum(0);
    progressBar->setMaximum(100);

    statusBar()->addPermanentWidget(progressBar);
    statusBar()->addPermanentWidget(zoomText);
    statusBar()->addPermanentWidget(statusText);
    progressBar->setVisible(false);

    createActions();
    connect(scene, &ImageScene::crop, this, &ImageViewer::crop);
    connect(this, &ImageViewer::enterCursorSelectionMode, scene, &ImageScene::onEnterCursorSelectionMode);
    connect(scene, &ImageScene::exitCursorSelectionMode, this, &ImageViewer::onExitCursorSelectionMode);

    connect(scene, &ImageScene::colorPickerClick, this, &ImageViewer::onColorPickerClick);

    connect(scene, &ImageScene::backgroundPickerClick, this, &ImageViewer::setTransparency);
    connect(scene, &ImageScene::pastePickerClick, this, &ImageViewer::pasteAt);

    connect(scene, &ImageScene::coordsChanged, pixelInfo, &Sidebar::onCoordsChanged);
    connect(this, &ImageViewer::updatePixelColorInfo, pixelInfo, &Sidebar::onUpdatePixelColorInfo);
    connect(scene, &ImageScene::clearColorInfo, pixelInfo, &Sidebar::onClearInfo);

    connect(imageTabs, &QTabBar::currentChanged, this, &ImageViewer::onTabChanged);

    thread = new QThread;
    worker = new Worker;
    worker->moveToThread(thread);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &Worker::convertColorsDone, this, &ImageViewer::convertColorsDone);
    connect(worker, &Worker::updateProgress, progressBar, &QProgressBar::setValue);
    thread->start(QThread::IdlePriority);

    resize(QGuiApplication::primaryScreen()->availableSize()*3.0/4.0);

    origBeadTable = beadTable;
    readSettings();
    updateRecentFileActions();
}
catch (const char* errMessage)
{
    QMessageBox::critical(this, tr("Fatal error"), errMessage, QMessageBox::Ok);
}


bool ImageViewer::loadFile(const QString &fileName)
{
    if (fileName.contains(".bptrn"))
    {
        loadBeadPattern(fileName);
        return true;
    }
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull())
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    /*bool same = true;
    for (int y = 0; y < newImage.height(); ++y)
    {
        if (!same) break;
        QRgb* line;
        line = reinterpret_cast<QRgb*>(newImage.scanLine(y));
        for (int x = 0; x < newImage.width(); x+=2)
        {
            if (line[x] != line[x+1])
            {
                same = false;
                break;
            }
        }
    }
    if (same)
    {
        newImage = newImage.scaled(newImage.size()*0.5, Qt::KeepAspectRatio, Qt::FastTransformation);
    }*/

    setImage(newImage);
    setCurrentFile(fileName);

    const QString message = tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);

    return true;
}

void ImageViewer::setImage(const QImage &newImage)
{
    origColorImage = newImage;
    image = newImage;

    updateImage();
    setRecolored(false);

    if (curSettings.autoMirror) horizontalFlip();

    if (QMessageBox::question(this, tr("Transparent background color"), tr("Do you wish to pick a transparent background color?")) == QMessageBox::Yes)
    {
        startCursorSelectionMode(CursorMode::backgroundPick, tr("Left Click to select color to make transparent, right click to cancel"));
    }
    if (curSettings.autoRecolor) convertToBeadColors();
}

void ImageViewer::updateImage()
{
    scene->getCoords().setScaleFactor(1.0);
    view->setBackgroundBrush(QBrush(QColor(160, 160, 160), Qt::DiagCrossPattern));

    scene->setImage(image);
    view->show();

    undoMgr.clearUndo();
    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
    printAct->setEnabled(true);
    updateActions();
    nActions = 0;

    const QString dims = tr("%1x%2").arg(image.width()).arg(image.height());
    statusText->setText(dims);

    if (curSettings.autoZoom) scaleImage(0.0, zoomMode::automatic);
    else scaleImage(1.0, zoomMode::absolute);
    if (!curSettings.savePalette) beadTable = origBeadTable;
}

bool ImageViewer::saveImage(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    nActions = 0;
    return true;
}

void ImageViewer::loadBeadPattern(const QString &fileName)
{
    int pixelsPerBead = 0;
    try
    {
        pixelsPerBead = beadPattern.load(fileName);
    }
    catch (QString errMessage)
    {
        QMessageBox::warning(this, tr("Bead pattern read error"), errMessage, QMessageBox::Ok);
        return;
    }

    updateImage();
    setRecolored(true);
    imageTabs->setCurrentIndex(1);
    setCurrentFile(fileName);
    scene->getCoords().setPixelsPerBead(pixelsPerBead);

    const QString message = tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
}

void ImageViewer::saveBeadPattern(const QString &fileName)
{
    if (recolored)
    {
        beadPattern.save(fileName, scene->getCoords().getPixelsPerBead());
        nActions = 0;
    }
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog)
    {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave) dialog.setDefaultSuffix("png");
}

void ImageViewer::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::openBeadPattern()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open Bead Pattern"), QDir::currentPath(), tr("Bead Pattern Files (*.bptrn)"));
    if (!fileName.isNull()) loadBeadPattern(fileName);
}

void ImageViewer::saveBeadPatternAs()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Save Bead Pattern"), QDir::currentPath(), tr("Bead Pattern Files (*.bptrn)"));
    if (!fileName.isNull()) saveBeadPattern(fileName);
}

void ImageViewer::openRecentFile()
{
    auto *action = qobject_cast<QAction*>(sender());
    if (action) loadFile(action->data().toString());
}

void ImageViewer::saveCurrentFile()
{
    if (!recentFiles.empty() && !image.isNull())
    {
        auto curFile = recentFiles[0];
        if (curFile.contains(".bptrn")) saveBeadPattern(curFile);
        else saveImage(curFile);
    }
}

void ImageViewer::saveImageAs()
{
    const QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Image As"), "",
            tr("PNG (*.png);;All Files (*)"));

    if (fileName.isEmpty()) return;
    else saveImage(fileName);
    setCurrentFile(fileName);
}

void ImageViewer::print()
{
    Q_ASSERT(scene->hasPixmap());
#if QT_CONFIG(printdialog)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec())
    {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = scene->getPixmap().size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(scene->getPixmap().rect());
        painter.drawPixmap(0, 0, scene->getPixmap());
    }
#endif
}

void ImageViewer::updateSceneImage()
{
    if (imageTabs->currentIndex() == 0) zoomedImage = origColorImage.scaled(scene->getScaledImageSize(origColorImage.size()), Qt::KeepAspectRatio, Qt::FastTransformation);
    else zoomedImage = image.scaled(scene->getScaledImageSize(image.size()), Qt::KeepAspectRatio, Qt::FastTransformation);
    scene->setImage(zoomedImage);
}

void ImageViewer::undoRedo(bool undo)
{
    auto prevScaleFactor = undoMgr.undoRedo(image, origColorImage, undo);
    scaleImage(prevScaleFactor, zoomMode::absolute);
    countsReliable = false;
    const QString dims = tr("%2x%3").arg(image.width()).arg(image.height());
    statusText->setText(dims);
    auto undoText = undoAct->toolTip();
    auto redoText = redoAct->toolTip();
    if (!undo && (redoText.size() > 5)) undoAct->setToolTip("Undo "+redoText.mid(5));
    else undoAct->setToolTip("Undo");
    undoAct->setEnabled(!undo);
    if (undo && (undoText.size() > 5)) redoAct->setToolTip("Redo "+undoText.mid(5));
    else redoAct->setToolTip("Redo");
    redoAct->setEnabled(undo);
    if (undo) --nActions;
    else nActions++;
}

void ImageViewer::setUndoableAction(QAction* action)
{
    undoMgr.setUndo(action->text(), image, origColorImage, scene->getCoords().getScaleFactor());
    undoAct->setToolTip("Undo "+action->text());
    undoAct->setEnabled(true);
    if (nActions >=0) nActions++;
    else nActions = 2;  //Negative nActions means that the user saved, then immediately undid. In that case, doing a new action means that they can never get back to the saved image through redo.
                        //Setting nActions to >1 ensures this. There is the edge case where the user undoes an action then performs the same exact same action again,
                        //(but not through redo), which does return to the saved image but checking for this is hard and not worth doing as it just leads to a redundant save prompt.
}

void ImageViewer::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData())
    {
        if (mimeData->hasImage())
        {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull()) return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void ImageViewer::paste()
{
#ifndef QT_NO_CLIPBOARD
    Q_ASSERT(scene->hasPixmap() && !image.isNull());
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) statusBar()->showMessage(tr("No image in clipboard"));
    else
    {
        if ((newImage.width() < image.width()) && (newImage.height() < image.height()))
        {
            scene->setPasteSize(newImage.size());
            startCursorSelectionMode(CursorMode::pastePick, tr("Left click to paste into area, right click to cancel"));
        }
        else pasteAt({0, 0});
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::createFromClipboard()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) statusBar()->showMessage(tr("No image in clipboard"));
    else
    {
        setImage(newImage);
        setWindowFilePath(QString());
        statusBar()->showMessage(tr("Obtained image from clipboard"));
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::pasteAt(QPoint pos)
{
#ifndef QT_NO_CLIPBOARD
    Q_ASSERT(!image.isNull());
    QImage pasteImage = clipboardImage();
    if (!pasteImage.isNull())
    {
        setUndoableAction(pasteAct);
        if ((pasteImage.width() < image.width()) && (pasteImage.height() < image.height()))
        {
            QPainter painterOrig(&origColorImage);
            painterOrig.drawImage(pos, pasteImage);
            painterOrig.end();
            if (recolored)
            {
                //This can be an expensive operation so perform it in the worker thread
                thread->setPriority(QThread::HighPriority);
                worker->convertToBeadColors(pasteImage, beadTable, false, false, false);
                countsReliable = false;
            }
            QPainter painter(&image);
            painter.drawImage(pos, pasteImage);
            painter.end();
        }
        else
        {
            origColorImage = pasteImage;
            if (recolored)
            {
                //This can be an expensive operation so perform it in the worker thread
                thread->setPriority(QThread::HighPriority);
                worker->convertToBeadColors(pasteImage, beadTable, false, false, false);
            }
            image = pasteImage;
            const QString dims = tr("%2x%3").arg(image.width()).arg(image.height());
            statusText->setText(dims);
        }
        updateSceneImage();
        view->show();
        setWindowFilePath(QString());
        statusBar()->showMessage(tr("Obtained image from clipboard"));
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::closeEvent(QCloseEvent *event)
{
    if (fileModified() && !recentFiles.empty())
    {
        auto response = QMessageBox::question(this, tr("Exitting Bead Planner"), tr("%1 has been modified. Do you want to save before exiting?").arg(recentFiles[0]), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (response == QMessageBox::Yes) saveCurrentFile();
        else if (response == QMessageBox::No) event->accept();
        else event->ignore();
    }
    else event->accept();
}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About Bead Planner"),
                           tr("Bead Planner version 1.0.0 (2022)\nBy Asger Grønnow\nasgergronnow@gmail.com\nReleased under GPLv3 licence (for the full licence text see the file COPYING)\nSource code freely available at https://github.com/agronnow/beadplanner"));
}

void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(QIcon(":/icons/tango/16x16/document-open.png"), tr("&Open Image"), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    fileMenu->addAction(QIcon(":/icons/tango/16x16/document-open.png"), tr("Open Bead Pattern"), this, &ImageViewer::openBeadPattern);

    recentMenu = fileMenu->addMenu(tr("&Recent Files"));
    recentMenu->setEnabled(false);

    for (auto& recentAct : recentFilesActs)
    {
        recentAct = recentMenu->addAction({}, this, &ImageViewer::openRecentFile);
        recentAct->setVisible(false);
    }
    fileMenu->addSeparator();

    fileMenu->addAction(tr("Create From Clipboard"), this, &ImageViewer::createFromClipboard);

    saveAct = fileMenu->addAction(QIcon(":/icons/tango/16x16/document-save.png"), tr("&Save"), this, &ImageViewer::saveCurrentFile);
    saveAct->setEnabled(false);

    saveBeadAct = fileMenu->addAction(QIcon(":/icons/tango/16x16/document-save.png"), tr("Save &As Bead Pattern"), this, &ImageViewer::saveBeadPatternAs);
    saveBeadAct->setEnabled(false);

    saveAsAct = fileMenu->addAction(QIcon(":/icons/tango/16x16/document-save.png"), tr("&Export to Image"), this, &ImageViewer::saveImageAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(QIcon(":/icons/tango/16x16/document-print.png"), tr("&Print"), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(QIcon(":/icons/tango/16x16/system-log-out.png"), tr("E&xit"), this, &ImageViewer::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    undoAct = editMenu->addAction(QIcon(":/icons/tango/16x16/edit-undo.png"), tr("&Undo"), this, [this]{ImageViewer::ImageViewer::undoRedo(true);});
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setEnabled(false);

    redoAct = editMenu->addAction(QIcon(":/icons/tango/16x16/edit-redo.png"), tr("&Redo"), this, [this]{ImageViewer::ImageViewer::undoRedo(false);});
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setEnabled(false);

    editMenu->addSeparator();

    copyAct = editMenu->addAction(QIcon(":/icons/tango/16x16/edit-copy.png"), tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    pasteAct = editMenu->addAction(QIcon(":/icons/tango/16x16/edit-paste.png"), tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);
    pasteAct->setEnabled(false);

    editMenu->addSeparator();

    replaceAct = editMenu->addAction(tr("View/Replace Bead Colours"), this, [this]{ImageViewer::startCursorSelectionMode(CursorMode::colorPick, tr("Left click to select a bead, right click to cancel"));});
    replaceAct->setEnabled(false);

    cropAct = editMenu->addAction(tr("Crop"), this, [this]{ImageViewer::startCursorSelectionMode(CursorMode::crop, tr("Left click and drag to select crop area, right click to cancel"));});
    cropAct->setEnabled(false);

    editMenu->addSeparator();

    pixelateAct = editMenu->addAction(tr("Pixelate"), this, &ImageViewer::pixelate);
    pixelateAct->setEnabled(false);

    flipAct = editMenu->addAction(QIcon(":/images/object-flip-horizontal.png"), tr("&Mirror Horizontally"), this, &ImageViewer::horizontalFlip);
    flipAct->setEnabled(false);

    editMenu->addSeparator();

    convertColorsAct = editMenu->addAction(tr("Convert To Bead Colours"), this, &ImageViewer::convertToBeadColors);
    convertColorsAct->setEnabled(false);

    convertColorsRestoreAct = editMenu->addAction(tr("Restore Original Colours"), this, &ImageViewer::restoreOrigColors);
    convertColorsRestoreAct->setEnabled(false);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    showGridAct = viewMenu->addAction(tr("Show &Grid"), scene, [this]{scene->toggleShowGrid(0);});
    showGridAct->setCheckable(true);
    showGridAct->setChecked(true);
    showGridAct->setEnabled(false);
    showDotsAct = viewMenu->addAction(tr("Show &Dots"), scene, [this]{scene->toggleShowGrid(1);});
    showDotsAct->setCheckable(true);
    showDotsAct->setChecked(true);
    showDotsAct->setEnabled(false);
    QAction* configGridAct = viewMenu->addAction(tr("Configure Grids"), this, &ImageViewer::configureGrid);
    configGridAct->setEnabled(true);

    QAction* setBeadSizeAct = viewMenu->addAction(tr("Set Bead Size"), this, &ImageViewer::setBeadSize);
    setBeadSizeAct->setEnabled(true);

    viewMenu->addSeparator();

    zoomMenu = viewMenu->addMenu(tr("&Zoom"));
    zoomMenu->setEnabled(false);

    zoomInAct = viewMenu->addAction(QIcon(":/images/zoom-in.png"), tr("Zoom &In (50%)"), this, [this]{ImageViewer::scaleImage(2.0, zoomMode::relative);});
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(QIcon(":/images/zoom-out.png"), tr("Zoom &Out (50%)"), this, [this]{ImageViewer::scaleImage(0.5, zoomMode::relative);});
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    zoomFitAct = viewMenu->addAction(QIcon(":/images/zoom-fit-best.png"), tr("&Best Fit"), this, [this]{ImageViewer::scaleImage(0.0, zoomMode::automatic);});
    zoomFitAct->setEnabled(false);

    zoomMenu->addSeparator();
    zoomGroup = new QActionGroup(this);

    double factor = zoomMax;
    while (factor > 0.99*zoomMin)
    {
        zoomFactors.push_back(factor);
        zoomActs.emplace_back(new QAction(tr("%1x").arg(factor), this));
        connect(zoomActs.back(), &QAction::triggered, this, [this,factor]{ImageViewer::scaleImage(factor, zoomMode::absolute);});
        zoomActs.back()->setEnabled(true);
        zoomActs.back()->setCheckable(true);
        zoomActs.back()->setData(factor);
        if (std::abs(factor - 1.0) < 0.0001) zoomActs.back()->setChecked(true);
        zoomMenu->addAction(zoomActs.back());
        zoomGroup->addAction(zoomActs.back());
        factor /= 2.0;
    }
    zoomGroup->setExclusive(true);


    QMenu *paletteMenu = menuBar()->addMenu(tr("&Palette"));
    QAction* editPaletteAct = paletteMenu->addAction(tr("Edit Palette"), this, &ImageViewer::editPalette);
    editPaletteAct->setEnabled(true);
    listBeadCountAct = paletteMenu->addAction(tr("Count Beads"), this, &ImageViewer::listBeadCounts);
    listBeadCountAct->setEnabled(false);

    QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction* settingsAct = settingsMenu->addAction(QIcon(":/icons/tango/16x16/preferences-system.png"), tr("Settings"), this, &ImageViewer::settings);
    settingsAct->setEnabled(true);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    //helpMenu->addAction(QIcon(":/icons/tango/16x16/help-browser.png"), tr("&Help"), this, &ImageViewer::help); TODO
    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);

    auto toolbar = addToolBar(tr("File"));
    toolbar->addAction(openAct);
    toolbar->addAction(saveAct);
    //toolbar->addAction(saveAsAct);
    toolbar->addAction(printAct);
    toolbar->addAction(undoAct);
    toolbar->addAction(redoAct);
    toolbar->addAction(copyAct);
    toolbar->addAction(pasteAct);
    toolbar->addAction(zoomInAct);
    toolbar->addAction(zoomOutAct);
    toolbar->addAction(zoomFitAct);
}

//Ensure that image editing actions are only enabled if an image is loaded
void ImageViewer::updateActions()
{
    saveAct->setEnabled(!image.isNull());
    saveAsAct->setEnabled(!image.isNull());
    saveBeadAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    replaceAct->setEnabled(false);
    cropAct->setEnabled(!image.isNull());
    pasteAct->setEnabled(!image.isNull());
    flipAct->setEnabled(!image.isNull());
    pixelateAct->setEnabled(!image.isNull());
    convertColorsAct->setEnabled(!image.isNull());
    convertColorsRestoreAct->setEnabled(!image.isNull());
    showGridAct->setEnabled(!image.isNull());
    showDotsAct->setEnabled(!image.isNull());
    zoomMenu->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!image.isNull());
    zoomOutAct->setEnabled(!image.isNull());
    zoomFitAct->setEnabled(!image.isNull());
    zoomGroup->setEnabled(!image.isNull());

    updateRecentFileActions();
}

void ImageViewer::updateRecentFileActions()
{
    std::size_t numRecentFiles = std::min(std::size_t(recentFiles.size()), maxRecentFiles);

    for (std::size_t i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(recentFiles[int(i)]).fileName());
        recentFilesActs[i]->setText(text);
        recentFilesActs[i]->setData(recentFiles[int(i)]);
        recentFilesActs[i]->setVisible(true);
    }
    for (std::size_t j = numRecentFiles; j < maxRecentFiles; ++j)
    {
        recentFilesActs[j]->setVisible(false);
    }

    recentMenu->setEnabled(numRecentFiles > 0);
}

void ImageViewer::setCurrentFile(const QString &fileName)
{
    setWindowTitle("Bead Planner - " + QFileInfo(fileName).fileName());

    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    while (recentFiles.size() > int(maxRecentFiles))
    {
        recentFiles.removeLast();
    }

    QSettings settings("Gronnow", "BeadPlanner");
    settings.setValue("recentFileList", recentFiles);
    updateRecentFileActions();
}


//Change image scaling without interpolation
//We use this to zoom without blur
void ImageViewer::scaleImage(double factor, zoomMode zoom)
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    double uncorrectedFactor = 1.0;
    double newScaleFactor = 1.0;
    if (zoom == zoomMode::automatic) uncorrectedFactor = std::min(double(view->width())/image.width(), double(view->height())/image.height());
    else uncorrectedFactor = (zoom == zoomMode::relative) ? scene->getCoords().getScaleFactor()*factor : factor;

    //Find closest allowed zoom factor
    auto it = std::min_element(zoomFactors.begin(), zoomFactors.end(), [uncorrectedFactor] (double a, double b)
    {return std::abs(uncorrectedFactor - a) < std::abs(uncorrectedFactor - b);});
    newScaleFactor = *it;
    scene->getCoords().setScaleFactor(newScaleFactor);
    updateSceneImage();
    const QString zoomMessage = tr("Zoom: %1x").arg(newScaleFactor);
    zoomText->setText(zoomMessage);

    if (newScaleFactor < 8.0/scene->getCoords().getPixelsPerBead()) view->setBackgroundBrush(QBrush(QColor(160, 160, 160), Qt::DiagCrossPattern));
    else
    {
        QImage curbgbrush = bgbrushImage->scaled(scene->getCoords().getBeadSize(), Qt::KeepAspectRatio, Qt::FastTransformation);
        view->setBackgroundBrush(QPixmap::fromImage(curbgbrush));
    }

    zoomInAct->setEnabled(newScaleFactor < 0.99*zoomMax);
    zoomOutAct->setEnabled(newScaleFactor > 1.01*zoomMin);

    //Ensure that the corresponding zoom menu item is checked
    zoomActs[std::size_t(std::distance(zoomFactors.begin(), it))]->setChecked(true);
}

void ImageViewer::onCoordsChanged(QPoint pixelCoords, QPoint beadCoords, QRgb pixelRGB)
{
    Q_UNUSED(beadCoords)
    Q_ASSERT(recolored && !origColorImage.isNull());
    std::string matches;
    beadTable.stringOfMatches(pixelRGB, matches);
    QRgb origRGB = origColorImage.pixel(pixelCoords);
    emit updatePixelColorInfo(origRGB, matches);
}

void ImageViewer::startCursorSelectionMode(CursorMode mode, const QString& message)
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    statusBar()->showMessage(message);
    view->setCursor(Qt::CrossCursor);
    emit enterCursorSelectionMode(mode);
}

void ImageViewer::onExitCursorSelectionMode()
{
    statusBar()->clearMessage();
    view->setCursor(Qt::ArrowCursor);
}

void ImageViewer::onColorPickerClick(QPoint pixelCoords, QPoint beadCoords)
{
    Q_ASSERT(!origColorImage.isNull() && !image.isNull() && scene->hasPixmap());
    if (imageTabs->currentIndex() == 0)
    {
        //It shouldn't be possible to be in color pick mode while in the original colors tab,
        //but in case it somehow happened just exit the selection mode instead of proceeding
        onExitCursorSelectionMode();
        return;
    }
    QRgb origRGB = origColorImage.pixel(pixelCoords);
    QRgb curRGB = image.pixel(pixelCoords);
    ImageColorsDialog imgColDlg(beadTable, beadCoords, origRGB, curRGB, this);
    connect(&imgColDlg, &ImageColorsDialog::replaceColor, this, &ImageViewer::onReplaceColor);
    imgColDlg.exec();
}

void ImageViewer::onReplaceColor(QPoint beadCoords, QRgb prevRGB, const BeadID& newColorID, ReplaceColorTarget target, bool floodFill)
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    if (imageTabs->currentIndex() == 0) return;
    setUndoableAction(replaceAct);
    BeadID prevColorID;
    beadTable.findClosestColor(prevRGB, prevColorID);
    QRgb newRGB = beadTable[newColorID].getRGB();
    ColorChange colChange(prevRGB, newRGB, beadCoords*scene->getCoords().getPixelsPerBead(), std::max(scene->getCoords().getPixelsPerBead(), 1), floodFill);

    if (target == ReplaceColorTarget::single)
    {
        colChange.setAction(ColorReplaceAction::replace_single);
        colChange.apply(nullptr, image);
    }
    else
    {
        if (target == ReplaceColorTarget::original) colChange.apply(&origColorImage, image);
        else colChange.apply(nullptr, image);
    }
    beadTable[prevColorID] -= colChange.getNumReplaced()*scene->getCoords().getPixelsPerBead()*scene->getCoords().getPixelsPerBead();
    beadTable[newColorID] += colChange.getNumReplaced()*scene->getCoords().getPixelsPerBead()*scene->getCoords().getPixelsPerBead();
    zoomedImage = image.scaled(scene->getScaledImageSize(image.size()), Qt::KeepAspectRatio, Qt::FastTransformation);
    scene->setImage(zoomedImage);
}

//Pixelate image by downscaling it and then zooming in
void ImageViewer::pixelate()
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    bool ok;
    int factor = QInputDialog::getInt(this, tr("Choose pixelation factor"), tr("Pixelation factor"), 8, 2, std::max(image.width()/2, 2), 1, &ok);
    if (!ok) return;
    setUndoableAction(pixelateAct);
    auto minDimension = std::min(image.width(), image.height());
    if (minDimension < 2) return;
    if (minDimension/factor < 2) factor = 2*minDimension;
    zoomedImage = image.scaled(image.width()/factor,image.height()/factor, Qt::KeepAspectRatio, Qt::FastTransformation);
    image = zoomedImage;
    origColorImage = origColorImage.scaled(origColorImage.width()/factor,origColorImage.height()/factor, Qt::KeepAspectRatio, Qt::FastTransformation);
    scaleImage(factor, zoomMode::relative);
    if (recolored) convertToBeadColors();
    const QString dims = tr("%2x%3").arg(image.width()).arg(image.height());
    statusText->setText(dims);
}

void ImageViewer::setBeadSize()
{
    bool ok;
    int ppb = QInputDialog::getInt(this, tr("Set bead size"), tr("Pixels per bead:"), scene->getCoords().getPixelsPerBead(), 1, 1000, 1, &ok);
    if (ok)
    {
        scene->getCoords().setPixelsPerBead(ppb);
        if (scene->getCoords().getScaleFactor() < 8.0/scene->getCoords().getPixelsPerBead()) view->setBackgroundBrush(QBrush(QColor(160, 160, 160), Qt::DiagCrossPattern));
        else
        {
            QImage curbgbrush = bgbrushImage->scaled(scene->getCoords().getBeadSize(), Qt::KeepAspectRatio, Qt::FastTransformation);
            view->setBackgroundBrush(QPixmap::fromImage(curbgbrush));
        }
    }
    /*BeadSizeDialog beadSizeDlg(scene->getCoords().getPixelsPerBead(), this);
    if (beadSizeDlg.exec() == QDialog::Accepted)
    {
        scene->getCoords().setPixelsPerBead(beadSizeDlg.getPixelsPerBead());
        if (scene->getCoords().getScaleFactor() < 8.0/scene->getCoords().getPixelsPerBead()) view->setBackgroundBrush(QBrush(QColor(160, 160, 160), Qt::BDiagPattern));
        else
        {
            QImage curbgbrush = bgbrushImage->scaled(scene->getCoords().getBeadSize(), Qt::KeepAspectRatio, Qt::FastTransformation);
            view->setBackgroundBrush(QPixmap::fromImage(curbgbrush));
        }
    }*/
}

void ImageViewer::configureGrid()
{
    GridDialog gridDlg(this, scene->getGrid());
    if (gridDlg.exec() == QDialog::Accepted)
    {
        gridDlg.updateGrids();
        scene->update();
    }
}

void ImageViewer::editPalette()
{
    BeadColorTable defaultTable;
    BeadColorTable customTable;
    QStringList keyList;
    beadTable.createKeyList(keyList);
    beadTable.createSeparateTables(defaultTable, customTable);
    PaletteDialog palDlg(defaultTable, customTable, keyList, this);
    if (palDlg.exec() == QDialog::Accepted)
    {
        if (curSettings.savePalette)
        {
            defaultTable.saveXML("default_colors.xml", true);
            customTable.saveXML("custom_colors.xml", false);
        }
        beadTable.mergeTables(defaultTable, customTable);
        if (recolored)
        {
            image = origColorImage;
            convertToBeadColors();
        }
    }
    else beadTable.setVersion(defaultTable.getVersion());
    std::cout << beadTable.getVersion().toStdString() << std::endl;
}

void ImageViewer::listBeadCounts()
{
    if (!countsReliable)
    {
        countsReliable = true;
        recountBeads(true);
    }
    else
    {
        BeadListDialog beadsDlg(beadTable, scene->getCoords().getPixelsPerBead(), this);
        beadsDlg.exec();
    }
}

void ImageViewer::setTransparency(QPoint BGpos, QRgb BGcolor)
{
    BackgroundDialog bgDlg(BGpos, BGcolor, this);
    connect(&bgDlg, &BackgroundDialog::updateTransparency, this, &ImageViewer::updateTransparency);
    connect(&bgDlg, &BackgroundDialog::resetTransparentColor, this, &ImageViewer::resetTransparentColor);
    updateTransparency(bgDlg.getColChange(), true);
    if (bgDlg.exec() == QDialog::Accepted) updateTransparency(bgDlg.getColChange(), false);
    else updateSceneImage();
}

void ImageViewer::updateTransparency(ColorChange& transp, bool preview)
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    image = image.convertToFormat(QImage::Format_ARGB32);
    if (preview)
    {
        previewImage = image.copy();
        previewImage = previewImage.convertToFormat(QImage::Format_ARGB32);
        transp.apply(nullptr, previewImage);
    }
    else transp.apply(nullptr, image);

    if (preview)
    {
        previewImage = previewImage.scaled(scene->getScaledImageSize(image.size()), Qt::KeepAspectRatio, Qt::FastTransformation);
        scene->setImage(previewImage);
    }
    else
    {
        if (!recolored) origColorImage = image;
        zoomedImage = image.scaled(scene->getScaledImageSize(image.size()), Qt::KeepAspectRatio, Qt::FastTransformation);
        scene->setImage(zoomedImage);
    }
}

void ImageViewer::resetTransparentColor()
{
    updateSceneImage();
}

void ImageViewer::convertToBeadColors()
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    centralWidget->setEnabled(false);
    progressBar->setVisible(true);
    statusBar()->showMessage(tr("Converting colours..."));

    //This can be an expensive operation so perform it in the worker thread
    thread->setPriority(QThread::HighPriority);
    worker->convertToBeadColors(image, beadTable, false, true, false);

}

//Update image and remove progress information from status bar
void ImageViewer::convertColorsDone(bool update, bool openList)
{
    thread->setPriority(QThread::IdlePriority);
    if (update)
    {
        setRecolored(true);
        imageTabs->setCurrentIndex(1);
    }
    statusBar()->clearMessage();
    progressBar->setVisible(false);
    progressBar->setValue(0);
    centralWidget->setEnabled(true);
    if (openList) listBeadCounts();
}

//Revert to original color image
void ImageViewer::restoreOrigColors()
{
    image = origColorImage;
    setRecolored(false);
    updateSceneImage();
}

void ImageViewer::setRecolored(bool recolor)
{
    recolored = recolor;
    pixelInfo->setOrigColorInfoVisible(recolor);
    listBeadCountAct->setEnabled(recolor);
    imageTabs->setTabEnabled(1, recolor);
    if (recolor)
    {
        connect(scene, &ImageScene::coordsChanged, this, &ImageViewer::onCoordsChanged);
        replaceAct->setEnabled(true);
    }
    else
    {
        disconnect(scene, &ImageScene::coordsChanged, this, &ImageViewer::onCoordsChanged);
        replaceAct->setEnabled(false);
        beadTable.resetCounts();
        imageTabs->setCurrentIndex(0);
    }
}

void ImageViewer::horizontalFlip()
{
    Q_ASSERT(!image.isNull() && scene->hasPixmap());
    image = image.mirrored(true, false);
    origColorImage = origColorImage.mirrored(true, false);
    updateSceneImage();
}

void ImageViewer::crop(QRect& rect)
{
    setUndoableAction(cropAct);
    double scaleFactor = scene->getCoords().getScaleFactor();
    rect.setRect(int(rect.x()/scaleFactor), int(rect.y()/scaleFactor), int(rect.width()/scaleFactor), int(rect.height()/scaleFactor));
    if (rect.x()+rect.width() > image.width()) rect.setWidth(image.width() - rect.x());
    if (rect.y()+rect.height() > image.height()) rect.setHeight(image.height() - rect.y());
    image = image.copy(rect);
    origColorImage = origColorImage.copy(rect);
    updateSceneImage();
    const QString dims = tr("%2x%3").arg(image.width()).arg(image.height());
    statusText->setText(dims);
    if (curSettings.autoZoom) scaleImage(0.0, zoomMode::automatic);
    if (recolored) recountBeads(false);
}

void ImageViewer::recountBeads(bool openList)
{
    centralWidget->setEnabled(false);
    progressBar->setVisible(true);
    statusBar()->showMessage(tr("Recounting beads..."));
    //This can be an expensive operation so perform it in the worker thread
    thread->setPriority(QThread::HighPriority);
    worker->convertToBeadColors(image, beadTable, true, false, openList);
}

void ImageViewer::onTabChanged(int newIdx)
{
    if ((newIdx == 0) && (!origColorImage.isNull()))
    {
        disconnect(scene, &ImageScene::coordsChanged, this, &ImageViewer::onCoordsChanged);
        replaceAct->setEnabled(false);
        pixelInfo->setOrigColorInfoVisible(false);
        pasteAct->setEnabled(false);
    }
    else if ((recolored) && (!image.isNull()))
    {
        connect(scene, &ImageScene::coordsChanged, this, &ImageViewer::onCoordsChanged);
        replaceAct->setEnabled(true);
        pixelInfo->setOrigColorInfoVisible(true);
        pasteAct->setEnabled(true);
    }
    updateSceneImage();
}

void ImageViewer::settings()
{
    auto prevSavePalette = curSettings.savePalette;
    SettingsDialog settingsDlg(this);
    if (settingsDlg.exec() == QDialog::Accepted)
    {
        curSettings = settingsDlg.getSettings();
        beadTable.setColorDistanceMeasure(curSettings.measure);
        if (!curSettings.savePalette && prevSavePalette) origBeadTable = beadTable;
    }
}

void ImageViewer::readSettings()
{
    QSettings initSettings("Gronnow", "BeadPlanner");
    recentFiles = initSettings.value("recentFileList").toStringList();
    auto measure = initSettings.value("colorconversionmethod", "cielab");
    if (measure == "redmean") curSettings.measure = colordistanceMeasure::redmean;
    else if (measure == "rgbdist") curSettings.measure = colordistanceMeasure::rgbdist;
    else curSettings.measure = colordistanceMeasure::CIEDE2000;
    curSettings.autoZoom = initSettings.value("autozoom", true).toBool();
    curSettings.savePalette = (initSettings.value("palettebehavior", "save").toString() == "save");
    curSettings.autoRecolor = initSettings.value("autoconvertcolors", false).toBool();
    curSettings.autoMirror = initSettings.value("automirrorimage", false).toBool();
}
