#include "modpack.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

bool Modpack::AddMod(const CurseMetaMod &mod)
{
    if (HasMod(mod.addonid))
        return true;

    CurseMetaMod copy(mod);

    if (!mod.modvalid)
        cursemeta_resolve(copy);

    copy.fileids.erase(std::remove_if(std::begin(copy.fileids),
                                      std::end(copy.fileids),
                                      [&](const auto &fileid) {
                                          const auto &[id, version] = fileid;
                                          return version == mcversion;
                                      }),
                       std::end(copy.fileids));

    if (copy.fileids.size() == 0) {
        // No file with the correct version found
        return false;
    }

    const auto dependencies = cursemeta_file_dependencies(
        copy.addonid, std::get<0>(copy.fileids.front()));

    for (const auto& dep : dependencies) {
        CurseMetaMod depmod;
        depmod.addonid = dep;
        if (!AddMod(depmod)) // Infinite cycle possible?
            return false; // Couldnt add dependencie 
    }

    mods.push_back(std::move(copy));
    return true;
}

bool Modpack::HasMod(int addonid) const
{
    for (const auto &m : mods) {
        if (m.addonid == addonid)
            return true;
    }

    return false;
}

void to_json(nlohmann::json &j, const Modpack &modpack)
{
    j                 = json();
    j["name"]         = modpack.name;
    j["author"]       = modpack.author;
    j["mcversion"]    = modpack.mcversion;
    j["version"]      = modpack.version;
    j["forgeversion"] = modpack.forgeversion;
    j["mods"]         = modpack.mods;
}

void from_json(const nlohmann::json &j, Modpack &modpack)
{
    j["name"].get_to(modpack.name);
    j["author"].get_to(modpack.author);
    j["mcversion"].get_to(modpack.mcversion);
    j["version"].get_to(modpack.version);
    j["forgeversion"].get_to(modpack.forgeversion);
    j["mods"].get_to(modpack.mods);
}
