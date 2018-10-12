#pragma once
#include "cursemeta.h"
#include "nlohmann/json_fwd.hpp"

struct Modpack {
    std::string name;
    std::string author;
    std::string mcversion;
    std::string version;
    std::string forgeversion;

    nlohmann::json ExportInCurseFormat() const;

    bool AddMod(const CurseMetaMod &mod);
    bool HasMod(int addonid) const;
    const std::list<CurseMetaMod> &GetMods() const { return mods; };

    friend void to_json(nlohmann::json &j, const Modpack &modpack);
    friend void from_json(const nlohmann::json &j, Modpack &modpack);

private:
    std::list<CurseMetaMod> mods;
};

void to_json(nlohmann::json &j, const Modpack &modpack);
void from_json(const nlohmann::json &j, Modpack &modpack);
