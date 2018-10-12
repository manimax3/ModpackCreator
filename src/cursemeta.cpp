#include "cursemeta.h"
#include "networkhelper.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

McVersionFinder::McVersionFinder(QObject *parent)
    : QObject(parent)
{
}

McVersionFinder::~McVersionFinder() {}

void McVersionFinder::StartSearching()
{
    auto &                   networkmanager = GetNetworkManager();
    static const std::string VERSIONS_API
        = "https://staging_cursemeta.dries007.net/api/v3/direct/minecraft/"
          "version";
    static const std::string FORGE_VERSIONS_API
        = "https://staging_cursemeta.dries007.net/api/v3/direct/minecraft/"
          "modloader";

    auto request = GetDefaultRequest(QUrl(VERSIONS_API.c_str()));

    mcconn = connect(&networkmanager, &QNetworkAccessManager::finished, this,
                     &McVersionFinder::RequestFinished);

    forgeconn = connect(&networkmanager, &QNetworkAccessManager::finished, this,
                        &McVersionFinder::RequestFinished);

    auto *mcrep = networkmanager.get(request);
    mcrep->setProperty("mcversion", QVariant(true));

    request.setUrl(QUrl(FORGE_VERSIONS_API.c_str()));
    auto *forgerep = networkmanager.get(request);
    forgerep->setProperty("forgeversion", QVariant(true));
}

void McVersionFinder::RequestFinished(QNetworkReply *reply)
{
    if (!(reply->property("mcversion").isValid()
          || reply->property("forgeversion").isValid()))
        return;

    json j;

    try {
        j = json::parse(reply->readAll());
    } catch (const json::parse_error &e) {
        qCritical()
            << "Couldnt download minecraft or forge version information";
        return;
    }

    if (reply->property("mcversion").isValid()) {
        for (auto &v : j) {
            CurseMetaMcVersion version;
            v.get_to(version);
            emit McVersionFound(version);
        }
        disconnect(mcconn);
    }

    if (reply->property("forgeversion").isValid()) {

        for (auto &v : j) {
            CurseMetaForge forge;
            v.get_to(forge);
            emit ForgeVersionFound(forge);
        }

        disconnect(forgeconn);
    }

    reply->deleteLater();
}

void cursemeta_resolve(CurseMetaMod &mod)
{
    auto &                   networkmanager = GetNetworkManager();
    static const std::string ADDON_SEARCH_URL
        = "https://staging_cursemeta.dries007.net/api/v3/direct/addon/";

    const std::string url(ADDON_SEARCH_URL + std::to_string(mod.addonid));
    const auto        request = GetDefaultRequest(url);

    QEventLoop     pause;
    QNetworkReply *reply = nullptr;

    auto conn = QObject::connect(
        &networkmanager, &QNetworkAccessManager::finished,
        [&](QNetworkReply *r) {
            if (!(r->property("cursemeta_resolve").isValid()))
                return;
            reply = r;
            pause.quit();
        });

    auto *r = networkmanager.get(request);
    r->setProperty("cursemeta_resolve", QVariant(true));

    pause.exec();
    QObject::disconnect(conn);

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

std::list<int> cursemeta_file_dependencies(int addonid, int fileid)
{
    auto &               networkmanager = GetNetworkManager();
    static const QString API("https://staging_cursemeta.dries007.net/api/v3/"
                             "direct/addon/%1/file/%2");
    const auto           strUrl  = API.arg(addonid).arg(fileid);
    auto                 request = GetDefaultRequest(strUrl);

    QEventLoop     pause;
    std::list<int> addonids;

    auto connection = QObject::connect(
        &networkmanager, &QNetworkAccessManager::finished,
        [&](QNetworkReply *reply) {
            if (!(reply->property("cursemeta_file_dependencies").isValid()))
                return;
            json j = json::parse(reply->readAll());
            for (const auto &d : j["dependencies"]) {
                addonids.push_back(d["addonId"].get<int>());
            }
            reply->deleteLater();
            pause.quit();
        });

    networkmanager.get(request)->setProperty("cursemeta_file_dependencies",
                                             QVariant(true));
    pause.exec();
    QObject::disconnect(connection);

    return addonids;
}

void to_json(nlohmann::json &j, const CurseMetaMod &mod)
{
    j                = json();
    j["addonname"]   = mod.addonname;
    j["addonid"]     = mod.addonid;
    j["fileids"]     = mod.fileids;
    j["description"] = mod.description;
    j["iconurl"]     = mod.iconurl;
    j["modvalid"]    = mod.modvalid;
}

void from_json(const nlohmann::json &j, CurseMetaMod &mod)
{
    j["addonname"].get_to(mod.addonid);
    j["addonid"].get_to(mod.addonid);
    j["fileids"].get_to(mod.fileids);
    j["description"].get_to(mod.description);
    j["iconurl"].get_to(mod.iconurl);
    j["modvalid"].get_to(mod.modvalid);
}

void from_json(const nlohmann::json &j, CurseMetaMcVersion &mcversion)
{
    j["versionString"].get_to(mcversion.version);
    j["approved"].get_to(mcversion.approved);
    j["dateModified"].get_to(mcversion.dateModified);
}

void from_json(const nlohmann::json &j, CurseMetaForge &forgeversion)
{
    j["name"].get_to(forgeversion.name);
    j["gameVersion"].get_to(forgeversion.mcversion);
    j["dateModified"].get_to(forgeversion.dateModified);
    j["latest"].get_to(forgeversion.latest);
    j["recommended"].get_to(forgeversion.recommended);
}
