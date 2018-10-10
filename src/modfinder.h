#pragma once
#include "cursemeta.h"

#include <QObject>
#include <string>

class ModFinder : public QObject {

    Q_OBJECT

public:
    ModFinder(const std::string &search);
    ModFinder(const std::string &search, const std::string &gameversion);

    void                    PerformSearch(int level = 0);
    std::list<CurseMetaMod> GetFoundMods();

private:
    struct SearchData {
        std::string projectname;
        std::string projecturl;
        std::string summary;
    };

    void                  DownloadSearchSite();
    std::list<SearchData> ParseSearchSite(const std::string &data);
    CurseMetaMod          ConvertToMetaMod(const SearchData &object);

    std::string            search, gameversion;
    std::list<std::string> data_cache;
};
