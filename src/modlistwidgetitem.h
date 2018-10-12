#pragma once
#include "iconcache.h"
#include "modfinder.h"

#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class ModListWidgetItem : public QListWidgetItem {
public:
    ModListWidgetItem(const ModFinder::SearchData &data,
                      QListWidget *                parent = nullptr)
        : QListWidgetItem(data.projectname.c_str(), parent)
        , data(data)
    {

        setToolTip(data.summary.c_str());

        if (data.iconurl.length() > 0) {

            if (IconCache::Get().HasIcon(data.iconurl)) {
                this->setIcon(IconCache::Get().GetIcon(data.iconurl));
            } else {
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
                        if (!pixm.isNull()) {
                            this->setIcon(pixm);
                            IconCache::Get().StoreIcon(data.iconurl, pixm);
                        }

                        reply->deleteLater();
                        QObject::disconnect(icondown_conn);
                    });

                networkmanager.get(request);
            }
        }
    }

    ModListWidgetItem(const CurseMetaMod &mod, QListWidget *parent = nullptr)
        : ModListWidgetItem(ModFinder::SearchData{ mod.addonname, "",
                                                   mod.iconurl,
                                                   mod.description },
                            parent)
    {
    }

    ModFinder::SearchData data;

private:
    QMetaObject::Connection icondown_conn;
};
