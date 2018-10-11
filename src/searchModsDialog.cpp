#include "searchModsDialog.h"
#include "modfinder.h"
#include "ui_searchModsDialog.h"

#include <QDebug>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace {
class ModListWidgetItem : public QListWidgetItem {
public:
    ModListWidgetItem(const ModFinder::SearchData &data,
                      QListWidget *                parent = nullptr)
        : QListWidgetItem(data.projectname.c_str(), parent)
    {

        setToolTip(data.summary.c_str());

        if (data.iconurl.length() > 0) {

            static QNetworkAccessManager networkmanager;

            // Download icon
            const QUrl      url(data.iconurl.c_str());
            QNetworkRequest request(url);
            request.setSslConfiguration(
                QSslConfiguration::defaultConfiguration());

            icondown_conn = QObject::connect(
                &networkmanager, &QNetworkAccessManager::finished,
                [=](QNetworkReply *reply) {
                    auto req = reply->request();

                    // Not our request
                    if (req.url() != url)
                        return;

                    QImage image;
                    image.loadFromData(reply->readAll());

                    auto pixm = QPixmap::fromImage(image);
                    if (!pixm.isNull())
                        this->setIcon(pixm);

                    reply->deleteLater();
                    QObject::disconnect(icondown_conn);
                });

            networkmanager.get(request);
        }
    }

private:
    QMetaObject::Connection icondown_conn;
};
}

SearchModsDialog::SearchModsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchModsDialog)
{
    ui->setupUi(this);

    ui->modList->setIconSize(QSize(48, 48));

    connect(ui->searchButton, &QPushButton::clicked, this,
            &SearchModsDialog::Search);
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
