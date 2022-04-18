#include "defaultcolorstab.h"

DefaultColorsTab::DefaultColorsTab(BeadColorTable& table, QWidget *parent) : QWidget(parent), beadTable{table}
{
    auto *layout = new QVBoxLayout;
    defaultModel = new TreeModel(table, this);
    defaultColorsTree = new QTreeView(this);
    defaultColorsTree->setModel(defaultModel);
    defaultColorsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(defaultColorsTree);

    buttonUpdate = new QPushButton(tr("&Update table from webpage"));
    layout->addWidget(buttonUpdate);
    setLayout(layout);

    connect(buttonUpdate, &QPushButton::released, this, &DefaultColorsTab::update);
}

void DefaultColorsTab::update()
{
    QString url = "https://raw.githubusercontent.com/agronnow/beadplanner/master/default_colors.xml";
    QNetworkAccessManager manager;
    QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop event;
    connect(response, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();
    QString content = response->readAll();

    auto idxbeg = content.indexOf("<Version>");
    auto idxend = content.indexOf("</Version>");
    if ((idxbeg >= 0) && (idxend > 0) && (idxend > idxbeg+9))
    {
        idxbeg += 9;
        auto version = content.mid(idxbeg, idxend - idxbeg);
        //std::cout << beadTable.getVersion().toStdString() << " online: " << version.toStdString() << std::endl;
        if (version != beadTable.getVersion())
        {
            if (QMessageBox::question(this, tr("Bead table update"), tr("There is a newer bead table available. Do you wish to update?")) == QMessageBox::Yes)
            {
                beadTable.loadXML(content.toUtf8(), false, true); //Load in new default bead colors
                beadTable.saveXML("default_colors.xml", true);
                defaultModel->rebuild();
            }
        }
        else QMessageBox::information(this, tr("Bead table update"), tr("The current bead table is already up to date"));
    }
    else QMessageBox::information(this, tr("Bead table update error"), tr("Downloaded bead table is invalid"));
}
