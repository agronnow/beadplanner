#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), settings("Gronnow", "BeadPlanner")
{
    setWindowTitle(tr("Settings"));
    auto *layout = new QVBoxLayout;
    auto methodGroup = new QGroupBox(tr("Colour conversion method"));
    auto *methodLayout = new QVBoxLayout;
    radbuttonCIE = new QRadioButton(tr("CIELAB 2000 (takes colour perception into account)"));
    radbuttonRedmean = new QRadioButton(tr("Redmean (faster but less accurate)"));
    radbuttonRGBdist = new QRadioButton(tr("RGB distance (no colour perception correction)"));
    methodLayout->addWidget(radbuttonCIE);
    methodLayout->addWidget(radbuttonRedmean);
    methodLayout->addWidget(radbuttonRGBdist);
    methodGroup->setLayout(methodLayout);
    layout->addWidget(methodGroup);
    auto *paletteLayout = new QVBoxLayout;
    auto paletteGroup = new QGroupBox(tr("Bead palette behaviour"));
    radbuttonSavePalette = new QRadioButton(tr("Permanently save bead palette changes"));
    radbuttonRevertPalette = new QRadioButton(tr("Revert bead palette on open image and exit"));
    paletteLayout->addWidget(radbuttonSavePalette);
    paletteLayout->addWidget(radbuttonRevertPalette);
    paletteGroup->setLayout(paletteLayout);
    layout->addWidget(paletteGroup);

    checkboxAutozoom = new QCheckBox(tr("Automatically set zoom to best fit when opening image"));
    layout->addWidget(checkboxAutozoom);
    checkboxAutoconvert = new QCheckBox(tr("Automatically convert colours when opening image"));
    layout->addWidget(checkboxAutoconvert);
    checkboxAutomirror = new QCheckBox(tr("Automatically mirror when opening image"));
    layout->addWidget(checkboxAutomirror);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::okPushed);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
    //setMinimumSize(600, 400);

    auto colorMethod = settings.value("colorconversionmethod", "cielab").toString();
    if (colorMethod == "rgbdist") radbuttonRGBdist->setChecked(true);
    else if (colorMethod == "redmean") radbuttonRedmean->setChecked(true);
    else radbuttonCIE->setChecked(true);

    if (settings.value("palettebehavior", "save").toString() == "revert") radbuttonRevertPalette->setChecked(true);
    else radbuttonSavePalette->setChecked(true);

    checkboxAutozoom->setChecked(settings.value("autozoom", true).toBool());
    checkboxAutoconvert->setChecked(settings.value("autoconvertcolors", false).toBool());
    checkboxAutomirror->setChecked(settings.value("automirrorimage", false).toBool());
}

void SettingsDialog::okPushed()
{
    QString colorMethod("cielab");
    if (radbuttonRedmean->isChecked()) colorMethod = "redmean";
    else if (radbuttonRGBdist->isChecked()) colorMethod = "rgbdist";
    settings.setValue("colorconversionmethod", colorMethod);
    if (radbuttonRevertPalette->isChecked()) settings.setValue("palettebehavior", "revert");
    else settings.setValue("palettebehavior", "save");
    settings.setValue("autozoom", checkboxAutozoom->isChecked());
    settings.setValue("autoconvertcolors", checkboxAutoconvert->isChecked());
    settings.setValue("automirrorimage", checkboxAutomirror->isChecked());

    QDialog::accept();
}

Settings SettingsDialog::getSettings() const
{
    auto measure = colordistanceMeasure::CIEDE2000;
    if (radbuttonRedmean->isChecked()) measure = colordistanceMeasure::redmean;
    else if (radbuttonRGBdist->isChecked()) measure = colordistanceMeasure::rgbdist;
    return {measure, checkboxAutozoom->isChecked(), radbuttonSavePalette->isChecked(),
                checkboxAutoconvert->isChecked(), checkboxAutomirror->isChecked()};
}
