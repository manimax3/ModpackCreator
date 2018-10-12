#include "modpack.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

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
