#pragma once
#include "cursemeta.h"

#include <QObject>
#include <optional>
#include <string>

class ModFinder : public QObject {

    Q_OBJECT

public:
    struct SearchData {
        std::string projectname;
        std::string projecturl;
        std::string iconurl;
        std::string summary;
    };

    ModFinder(const std::string &search);
    ModFinder(const std::string &search, const std::string &gameversion);

    void                    PerformSearch(int level = 0);
    std::list<CurseMetaMod> GetFoundMods();
    static CurseMetaMod     GetCurseMod(const SearchData &         data,
                                        std::optional<std::string> version = {});

    static bool SimpleCheckIsMod(const SearchData &data);

signals:
    void FoundSearchData(const SearchData &data);
    void SearchParseFinished();

private:
    void                DownloadSearchSite();
    void                ParseSearchSite(const std::string &data);
    static CurseMetaMod ConvertToMetaMod(const SearchData &object);

    std::string           search, gameversion;
    std::list<SearchData> data_cache;
};
