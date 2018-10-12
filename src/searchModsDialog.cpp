#include "searchModsDialog.h"
#include "iconcache.h"
#include "modfinder.h"
#include "ui_searchModsDialog.h"
#include "modlistwidgetitem.h"

#include <QDebug>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>

SearchModsDialog::SearchModsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchModsDialog)
{
    ui->setupUi(this);

    ui->modList->setIconSize(QSize(48, 48));
    ui->modList->setSelectionMode(
        QAbstractItemView::SelectionMode::ExtendedSelection);

    connect(ui->searchButton, &QPushButton::clicked, this,
            &SearchModsDialog::Search);

    connect(ui->addSelectedButton, &QPushButton::clicked, this,
            &SearchModsDialog::HandleAddModsRequest);
}

SearchModsDialog::~SearchModsDialog() { delete ui; }

void SearchModsDialog::Search()
{
    if (ui->searchEdit->text().isEmpty())
        return;

    const std::string searchterm(ui->searchEdit->text().toStdString());

    finder = new ModFinder(searchterm);

    ui->modList->clear();
    connect(finder, &ModFinder::FoundSearchData, this,
            &SearchModsDialog::AddFoundMond);

    connect(finder, &ModFinder::SearchParseFinished, this,
            [&] { finder->deleteLater(); });

    finder->PerformSearch();
}

void SearchModsDialog::AddFoundMond(const ModFinder::SearchData &data)
{
    auto *item = new ModListWidgetItem(data);
    ui->modList->addItem(item);
}

void SearchModsDialog::HandleAddModsRequest()
{
    auto selected = ui->modList->selectedItems();

    if (selected.empty())
        return;

    std::list<CurseMetaMod> meta;

    for (auto *s : selected) {
        auto *       mls  = static_cast<ModListWidgetItem *>(s);
        const auto & data = mls->data;
        CurseMetaMod mod  = ModFinder::GetCurseMod(data);
        if (mod.modvalid)
            meta.push_back(mod);
    }

    if (meta.size() == 0)
        return;

    emit ModsExport(std::move(meta));
    close();
}
