#include "customcolordialog.h"

CustomColorDialog::CustomColorDialog(BeadColor& curItem, const std::string& key, const QStringList& keys, QWidget* parent)
    : QDialog(parent), item{curItem}, color{0}, colorPixmap(48, 48)
{
    setWindowTitle(tr("Custom bead color"));
    auto mainLayout = new QVBoxLayout;
    auto formLayout = new QFormLayout;
    editName = new QLineEdit;
    editCode = new QLineEdit;
    comboboxType = new QComboBox;
    editSource = new QLineEdit;
    auto colorGroup = new QGroupBox(tr("Colour"));
    labelColor = new QLabel;
    labelRed = new QLabel;
    labelGreen = new QLabel;
    labelBlue = new QLabel;
    buttonColor = new QPushButton(tr("Set"));
    formLayout->addRow(new QLabel(tr("Name")), editName);
    formLayout->addRow(new QLabel(tr("Code")), editCode);
    formLayout->addRow(new QLabel(tr("Brand")), comboboxType);
    formLayout->addRow(new QLabel(tr("Source (optional)")), editSource);
    mainLayout->addLayout(formLayout);
    auto colorLayout = new QGridLayout;
    colorLayout->addWidget(labelColor, 0, 0, 3, 1, Qt::AlignCenter);
    colorLayout->addWidget(labelRed, 0, 1);
    colorLayout->addWidget(labelGreen, 1, 1);
    colorLayout->addWidget(labelBlue, 2, 1);
    colorLayout->addWidget(buttonColor, 3, 0, 1, 2);
    colorGroup->setLayout(colorLayout);
    mainLayout->addWidget(colorGroup);

    comboboxType->addItems(keys);
    comboboxType->setEditable(true);

    if (!item.getName().empty())
    {
        editName->setText(QString::fromStdString(item.getName()));
        editCode->setText(QString::fromStdString(item.getCode()));
        if (!key.empty()) comboboxType->setEditText(QString::fromStdString(key));
        else comboboxType->setCurrentIndex(0);
        editSource->setText(QString::fromStdString(item.getSource()));
        color = item.getRGB();
    }
    updateColor(color);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);

    connect(editName, &QLineEdit::textChanged, this, &CustomColorDialog::onTextChanged);
    connect(comboboxType, &QComboBox::currentTextChanged, this, &CustomColorDialog::onTextChanged);
    connect(editCode, &QLineEdit::textChanged, this, &CustomColorDialog::onTextChanged);
    connect(buttonColor, &QPushButton::released, this, &CustomColorDialog::specifyColor);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (item.getName().empty() || item.getCode().empty()) buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

    setLayout(mainLayout);
    this->setFixedSize(320, 320);
}

void CustomColorDialog::specifyColor()
{
    auto newColor = QColorDialog::getColor(color, this, tr("Select Colour"), QColorDialog::DontUseNativeDialog);
    if (newColor.isValid())
    {
        updateColor(newColor.rgb());
    }
}

void CustomColorDialog::updateColor(QRgb newcolor)
{
    color = newcolor;
    labelRed->setText(tr("Red: ")+QString::number(qRed(color)));
    labelGreen->setText(tr("Green: ")+QString::number(qGreen(color)));
    labelBlue->setText(tr("Blue: ")+QString::number(qBlue(color)));
    auto painter = new QPainter;
    painter->begin(&colorPixmap);
    painter->fillRect(1,1,46,46,color);
    painter->setPen(Qt::black);
    painter->drawRect(0,0,47,47);
    painter->end();
    labelColor->setPixmap(colorPixmap);
    delete painter;
}

void CustomColorDialog::onTextChanged(const QString& newText)
{
    Q_UNUSED(newText)
    buttons->button(QDialogButtonBox::Ok)->setEnabled(((!editName->text().isEmpty()) && (!editCode->text().isEmpty()) && (!comboboxType->currentText().isEmpty())));
}

std::string CustomColorDialog::updateItem()
{
    item.setRGB(color);
    item.setName(editName->text().toStdString());
    item.setCode(editCode->text().toStdString());
    item.setSource(editSource->text().toStdString());
    return comboboxType->currentText().toStdString();
}
