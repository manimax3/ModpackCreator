#include "searchModsDialog.h"
#include "ui_searchModsDialog.h"
#include "modfinder.h"

#include <QLineEdit>
#include <QDebug>

SearchModsDialog::SearchModsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchModsDialog)
{
    ui->setupUi(this);

    connect(ui->searchButton, &QPushButton::clicked, this,
            &SearchModsDialog::Search);
}

SearchModsDialog::~SearchModsDialog() { delete ui; }

void SearchModsDialog::Search()
{
    if (ui->searchEdit->text().isEmpty())
        return;

    const std::string searchterm(ui->searchEdit->text().toStdString());

    ModFinder finder(searchterm);

    connect(&finder, &ModFinder::FoundSearchData, this,
            [&](const ModFinder::SearchData &data) {
                this->ui->modList->addItem(QString(data.projectname.c_str()));
            });

    finder.PerformSearch();
}
