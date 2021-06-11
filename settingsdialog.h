#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>
#include "beadcolortable.h"

struct Settings
{
    colordistanceMeasure measure = colordistanceMeasure::CIEDE2000;
    bool autoZoom = true;
    bool savePalette = true;
    bool autoRecolor = false;
    bool autoMirror = false;
};

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    Settings getSettings() const;

private slots:
    void okPushed();

private:
    QRadioButton *radbuttonCIE;
    QRadioButton *radbuttonRedmean;
    QRadioButton *radbuttonRGBdist;
    QRadioButton *radbuttonSavePalette;
    QRadioButton *radbuttonRevertPalette;
    QCheckBox *checkboxAutozoom;
    QCheckBox *checkboxAutoconvert;
    QCheckBox *checkboxAutomirror;
    QDialogButtonBox *buttons;
    QSettings settings;
};

#endif // SETTINGSDIALOG_H
