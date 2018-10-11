#include "cursemeta.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

void cursemeta_resolve(CurseMetaMod &mod)
{
    static QNetworkAccessManager networkmanager;
    static const std::string     ADDON_SEARCH_URL
        = "https://staging_cursemeta.dries007.net/api/v3/direct/addon/";

    const std::string url(ADDON_SEARCH_URL + std::to_string(mod.addonid));
    QNetworkRequest   request(QUrl(url.c_str()));
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QEventLoop     pause;
    QNetworkReply *reply = nullptr;

    QObject::connect(&networkmanager, &QNetworkAccessManager::finished,
                     [&](QNetworkReply *r) {
                         reply = r;
                         pause.quit();
                     });

    networkmanager.get(request);
    pause.exec();

    if (!reply) {
        mod.modvalid = false;
        return;
    }

    const std::string data(reply->readAll());

    json j = json::parse(data);

    try {
        mod.addonname   = j["name"];
        mod.description = j["summary"];

        if (j["sectionName"].get<std::string>() != "mc-mods") {
            mod.modvalid = false;
            return;
        }

        for (auto &f : j["gameVersionLatestFiles"]) {
            mod.fileids.push_back({ f["projectFileId"].get<int>(),
                                    f["gameVersion"].get<std::string>() });
        }

        mod.modvalid = true;
    } catch (const json::type_error &e) {
        qWarning()
            << "Couldnt resolve a mod because of missing fields. Skipping...";
        mod.modvalid = false;
    }
}
