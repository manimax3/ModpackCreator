#pragma once
#include "cursemeta.h"
#include "modfinder.h"

#include <QObject>
#include <vector>

class ModDataManager : public QObject {
    Q_OBJECT

public:
    using AddonId = int;
    using FileId  = int;

    static ModDataManager &Get();

    void RegisterMod(AddonId addonid);
    void RegisterMod(const CurseMetaMod &mod);
    void RegisterMod(const ModFinder::SearchData &mod);

    void SearchForMods(const QString &search);

    bool IsModRegistered(AddonId addonid) const;

    void DependenciesLookup(AddonId addonid, FileId fileid);

    std::string GetModIconUrl(AddonId addonid);

    CurseMetaMod *GetMod(AddonId addonid);
signals:
    void ModAdded(AddonId addonid);
    void ModSearchResult(AddonId addonid);
    void DependencyFound(AddonId addonid, FileId fileid, AddonId dependency);

private:
    std::vector<CurseMetaMod> moddata;

    ModDataManager();

    void HandleNetworkReply(class QNetworkReply *reply);
    bool ParseSearchData(class QNetworkReply *const reply);
    bool ParseAddonData(class QNetworkReply *const reply);
    bool ParseFileData(class QNetworkReply *const reply);
};
