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
    std::string       iconurl;

    bool modvalid = false;
};

void cursemeta_resolve(CurseMetaMod &mod);
