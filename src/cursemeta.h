#pragma once
#include <list>
#include <string>
#include <tuple>

struct CurseMetaMod {
    // Fileid, version
    using FileId = std::tuple<int, std::string>;

    std::string       addonname;
    int               addonid;
    std::list<FileId> fileids;
    std::string       description;

    bool modvalid = false;
};

void cursemeta_resolve(CurseMetaMod &mod);
