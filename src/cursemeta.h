#pragma once
#include <list>
#include <string>

struct CurseMetaMod {
    std::string    addonname;
    int            addonid;
    std::list<int> fileids;
    std::string    version;
    std::string    description;
};

inline void cursemeta_resolve(CurseMetaMod &mod){}
