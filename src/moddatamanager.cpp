#include "moddatamanager.h"
#include "iconcache.h"
#include "modfinder.h"
#include "networkhelper.h"

#include "nlohmann/json.hpp"
#include <QNetworkReply>

using json = nlohmann::json;

ModDataManager &ModDataManager::Get()
{
    static ModDataManager instance;
    return instance;
}

ModDataManager::ModDataManager()
{
    auto &networkmanager = GetNetworkManager();
    connect(&networkmanager, &QNetworkAccessManager::finished, this,
            &ModDataManager::HandleNetworkReply);
}

void ModDataManager::RegisterMod(AddonId addonid)
{
    if (IsModRegistered(addonid))
        return;

    auto &networkmanager = GetNetworkManager();

    static const QString URL(
        "https://staging_cursemeta.dries007.net/api/v3/direct/addon/%1");
    const auto request = GetDefaultRequest(URL.arg(addonid));
    auto *     reply   = networkmanager.get(request);
    reply->setProperty("moddatamanager_parseaddondata", true);
    reply->setProperty("moddatamanager_parseaddondata_addonid",
                       addonid); // Maybe we will need it
}

void ModDataManager::RegisterMod(const CurseMetaMod &mod)
{
    if (!mod.addonid)
        return;

    if (IsModRegistered(mod.addonid)) {
        auto *m = GetMod(mod.addonid);
        if (m->iconurl.empty())
            m->iconurl = mod.iconurl;

        return;
    }

    if (mod.modvalid) {
        moddata.push_back(mod);
        emit ModAdded(mod.addonid);
        return;
    }

    auto &networkmanager = GetNetworkManager();

    static const QString URL(
        "https://staging_cursemeta.dries007.net/api/v3/direct/addon/%1");
    const auto request = GetDefaultRequest(URL.arg(mod.addonid));
    auto *     reply   = networkmanager.get(request);
    reply->setProperty("moddatamanager_parseaddondata", true);
    reply->setProperty("moddatamanager_parseaddondata_addonid",
                       mod.addonid); // Maybe we will need it

    if (!mod.iconurl.empty()) // We might not have it available later
        reply->setProperty("moddatamanager_parseaddondata_iconurl",
                           mod.iconurl.c_str()); // Maybe we will need it
}

void ModDataManager::RegisterMod(const ModFinder::SearchData &data)
{
    CurseMetaMod mod;
    mod.addonname   = data.projectname;
    mod.description = data.summary;
    mod.iconurl     = data.iconurl;
    mod.modvalid    = false;

    auto projectid = data.projecturl.find("projectID");

    if (projectid == std::string::npos) {
        qCritical() << "Provided search data didnt have a projectID in its url";
    } else {
        projectid += 10;
        std::string projectid_v(data.projecturl.c_str() + projectid);

        mod.addonid = std::stoi(projectid_v);

        RegisterMod(mod);
    }
}

void ModDataManager::SearchForMods(const QString &search)
{
    if (search.isEmpty())
        return;

    auto &networkmanager = GetNetworkManager();

    static const QString URL(
        "https://minecraft.curseforge.com/search?search=%1");
    const auto request = GetDefaultRequest(URL.arg(search));

    networkmanager.get(request)->setProperty("moddatamanager_searchformod",
                                             true);
}

CurseMetaMod *ModDataManager::GetMod(AddonId addonid)
{
    const auto it
        = std::find_if(std::begin(moddata), std::end(moddata),
                       [&](const auto &mod) { return mod.addonid == addonid; });

    return it != std::end(moddata) ? &(*it) : nullptr;
}

void ModDataManager::DependenciesLookup(AddonId addonid, FileId fileid)
{
    if (!addonid || !fileid)
        return;

    auto &networkmanager = GetNetworkManager();

    static const QString URL("https://staging_cursemeta.dries007.net/api/v3/"
                             "direct/addon/%1/file/%2");
    const auto request = GetDefaultRequest(URL.arg(addonid).arg(fileid));

    auto *reply = networkmanager.get(request);
    reply->setProperty("moddatamanager_dependencieslookup", true);
    reply->setProperty("moddatamanager_dependencieslookup_addonid", addonid);
    reply->setProperty("moddatamanager_dependencieslookup_fileid", fileid);
}

std::string ModDataManager::GetModIconUrl(AddonId addonid)
{
    const auto *mod = GetMod(addonid);
    return mod ? mod->iconurl : "";
}

void ModDataManager::HandleNetworkReply(QNetworkReply *reply)
{
    if (ParseSearchData(reply))
        reply->deleteLater();
    else if (ParseAddonData(reply))
        reply->deleteLater();
    else if (ParseFileData(reply))
        reply->deleteLater();
}

bool ModDataManager::ParseSearchData(QNetworkReply *const reply)
{
    if (!reply->property("moddatamanager_searchformod").isValid())
        return false;

    ModFinder modFinder("");

    auto connection = connect(
        &modFinder, &ModFinder::FoundSearchData, this,
        [&](const ModFinder::SearchData &data) {
            RegisterMod(data);
            auto projectid = data.projecturl.find("projectID");

            if (projectid != std::string::npos) {
                projectid += 10;
                std::string projectid_v(data.projecturl.c_str() + projectid);
                auto        addonid = std::stoi(projectid_v);
                if (addonid)
                    emit ModSearchResult(addonid);
            }
        });

    modFinder.ParseSearchSite(reply->readAll().toStdString());
    disconnect(connection);

    return true;
}

bool ModDataManager::ParseAddonData(QNetworkReply *const reply)
{
    if (!reply->property("moddatamanager_parseaddondata").isValid())
        return false;

    CurseMetaMod mod;

    mod.addonid
        = reply->property("moddatamanager_parseaddondata_addonid").toInt();

    if (reply->property("moddatamanager_parseaddondata_iconurl").isValid())
        mod.iconurl = reply->property("moddatamanager_parseaddondata_iconurl")
                          .toString()
                          .toStdString();

    try {
        json j          = json::parse(reply->readAll());
        mod.addonname   = j["name"];
        mod.description = j["summary"];

        if (j["sectionName"].get<std::string>() != "mc-mods") {
            mod.modvalid = false;
        }

        for (auto &f : j["gameVersionLatestFiles"]) {
            mod.fileids.push_back({ f["projectFileId"].get<int>(),
                                    f["gameVersion"].get<std::string>() });
        }

        mod.modvalid = true;
    } catch (const json::type_error &e) {
        qWarning()
            << "Couldnt resolve a mod because of missing fields. Skipping...";
    } catch (const json::parse_error &e) {
        qWarning() << "Couldnt parse json data from provided addon data";
    } catch (const json::out_of_range &e) {
        qWarning() << "ModDataManager:213: " << e.what();
    }

    if (mod.modvalid) {
        moddata.push_back(mod);
        emit ModAdded(mod.addonid);
    }

    return true;
}

bool ModDataManager::ParseFileData(QNetworkReply *const reply)
{
    if (!reply->property("moddatamanager_dependencieslookup").isValid())
        return false;

    const auto addonid
        = reply->property("moddatamanager_dependencieslookup_addonid").toInt();
    const auto fileid
        = reply->property("moddatamanager_dependencieslookup_fileid").toInt();

    try {
        json j = json::parse(reply->readAll());
        for (const auto &d : j["dependencies"]) {
            emit DependencyFound(addonid, fileid, d["addonId"].get<int>());
        }
    } catch (const json::parse_error &e) {
        qWarning() << "Couldnt parse data from file api result" << e.what();
    }

    return true;
}

bool ModDataManager::IsModRegistered(AddonId addonid) const
{
    const auto it
        = std::find_if(std::cbegin(moddata), std::cend(moddata),
                       [&](const auto &mod) { return mod.addonid == addonid; });

    return it != std::end(moddata);
}
