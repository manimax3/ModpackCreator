#pragma once
#include "cursemeta.h"
#include "nlohmann/json_fwd.hpp"

struct Modpack {
    std::string name;
    std::string author;
    std::string mcversion;
    std::string version;
    std::string forgeversion;

    std::list<CurseMetaMod> mods;

    nlohmann::json ExportInCurseFormat() const;
};

void to_json(nlohmann::json &j, const Modpack &modpack);
void from_json(const nlohmann::json &j, Modpack &modpack);
