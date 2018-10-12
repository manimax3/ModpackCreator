#include "modfinder.h"
#include "networkhelper.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <tidy.h>
#include <tidybuffio.h>

#include "tinyxml2.h"

namespace {
using namespace tinyxml2;
template<typename Matcher>
struct XMLRecursiveSearcher {

    XMLRecursiveSearcher(XMLElement *root, Matcher m)
        : matcher(m)
        , root(root)
    {
    }

    void StartSearch() { Walker(root); }

    std::list<XMLElement *> &GetResult() { return results; }

private:
    void Walker(XMLElement *element)
    {
        if (!element)
            return;

        if (matcher(element)) {
            results.push_back(element);
        }

        for (auto child = element->FirstChildElement(); child;
             child      = child->NextSiblingElement()) {
            Walker(child);
        }
    }

    Matcher     matcher;
    XMLElement *root;

    std::list<XMLElement *> results;
};
}

ModFinder::ModFinder(const std::string &search)
    : search(search)
    , gameversion("unspecified")
{
}

ModFinder::ModFinder(const std::string &search, const std::string &gameversion)
    : ModFinder(search)
{
    this->gameversion = gameversion;
}

void ModFinder::PerformSearch(int level) { DownloadSearchSite(); }

std::list<CurseMetaMod> ModFinder::GetFoundMods()
{
    std::list<CurseMetaMod> mods;

    for (const auto &search : data_cache) {
        mods.push_back(ConvertToMetaMod(search));
    }

    // Remove invalids mods
    mods.erase(std::remove_if(std::begin(mods), std::end(mods),
                              [](const auto &mod) { return !mod.modvalid; }),
               std::end(mods));

    if (gameversion != "unspecified") {
        for (auto &mod : mods) {
            mod.fileids.erase(
                std::remove_if(std::begin(mod.fileids), std::end(mod.fileids),
                               [&](const auto &file) {
                                   const auto &[id, version] = file;
                                   return version != this->gameversion;
                               }),
                std::end(mod.fileids));
        }
    }

    return mods;
}

CurseMetaMod ModFinder::GetCurseMod(const SearchData &         data,
                                    std::optional<std::string> gameversion)
{
    CurseMetaMod mod = ConvertToMetaMod(data);

    if (!mod.modvalid)
        return {};

    if (gameversion) {
        mod.fileids.erase(
            std::remove_if(std::begin(mod.fileids), std::end(mod.fileids),
                           [&](const auto &file) {
                               const auto &[id, version] = file;
                               return version != gameversion.value();
                           }),
            std::end(mod.fileids));
    }

    return mod;
}

void ModFinder::DownloadSearchSite()
{
    if (search.length() == 0)
        return;

    auto& networkmanager = GetNetworkManager();
    static const std::string     SEARCH_URL
        = "https://minecraft.curseforge.com/search?search=";

    const std::string final_url = SEARCH_URL + search;
    const auto        request   = GetDefaultRequest(final_url);

    QObject::disconnect(&networkmanager, &QNetworkAccessManager::finished, this,
                        nullptr);
    QObject::connect(&networkmanager, &QNetworkAccessManager::finished, this,
                     [&](QNetworkReply *rep) {
                         if (!(rep->property("DownloadSearchSite").isValid()))
                             return;

                         std::string data(rep->readAll());
                         ParseSearchSite(data);
                         rep->deleteLater();
                     });

    networkmanager.get(request)->setProperty("DownloadSearchSite", QVariant(true));
}

void ModFinder::ParseSearchSite(const std::string &data)
{
    using namespace tinyxml2;

    std::list<SearchData> foundaddons;

    const char *input = data.c_str();

    TidyBuffer output   = { 0 };
    TidyBuffer errorbuf = { 0 };
    TidyDoc    tdoc     = tidyCreate();

    if (!tidyOptSetBool(tdoc, TidyXhtmlOut, yes)) {
        qCritical() << "tidysetbool()" << __FILE__ << __FUNCTION__;
    }

    if (tidySetErrorBuffer(tdoc, &errorbuf) < 0) {
        qCritical() << "tidySetBuffer()" << __FILE__ << __FUNCTION__;
    }

    if (tidyParseString(tdoc, input) < 0) {
        qCritical() << "tidyParseString()" << __FILE__ << __FUNCTION__;
    }

    if (tidyCleanAndRepair(tdoc) < 0) {
        qCritical() << "tidyCleanAndRepair()" << __FILE__ << __FUNCTION__;
    }

    if (tidySaveBuffer(tdoc, &output) < 0) {
        qCritical() << "tidySaveBuffer()" << __FILE__ << __FUNCTION__;
    }

    const std::string xmldata(reinterpret_cast<char *>(output.bp));

    tidyBufFree(&output);
    tidyBufFree(&errorbuf);
    tidyRelease(tdoc);

    XMLDocument doc;
    if (doc.Parse(xmldata.c_str()) != XML_SUCCESS) {
        qWarning() << "Couldnt parse a xml string";
    }

    auto *root = doc.RootElement();

    XMLRecursiveSearcher searcher(root, [](auto &elem) {
        if (!elem)
            return false;

        std::string_view name(elem->Name());

        if (name.find("table") == std::string_view::npos)
            return false;

        const auto classes = elem->Attribute("class");

        if (!classes)
            return false;

        std::string_view classes_sv(classes);

        return classes_sv.find("listing-project") != std::string_view::npos;
    });

    searcher.StartSearch();
    const auto elements = searcher.GetResult();

    if (elements.size() != 1) {
        qCritical() << "Couldnt parse the search results correctly. Found "
                       "results for valid tables are not equal to 1";
        emit SearchParseFinished();
        return;
    }

    const auto table_body = elements.front()->FirstChildElement("tbody");

    for (auto result = table_body->FirstChildElement(); result;
         result      = result->NextSiblingElement()) {

        if (std::string_view(result->Attribute("class")) != "results")
            continue;

        const auto img_icon = result->FirstChildElement("td")
                                  ->FirstChildElement("a")
                                  ->FirstChildElement("img");
        const auto td
            = result->FirstChildElement("td")->NextSiblingElement("td");
        const auto div1 = td->FirstChildElement("div");
        const auto div2 = div1->NextSiblingElement("div");

        if (std::string_view(div1->Attribute("class")) != "results-name") {
            qCritical()
                << "Search parse not in the correct configuratin for div1";
            continue;
        }

        if (std::string_view(div2->Attribute("class")) != "results-summary") {
            qCritical()
                << "Search parse not in the correct configuratin for div1";
            continue;
        }

        // TODO: Maybe more checking here

        const auto  namelink = div1->FirstChildElement("a");
        std::string projectname(
            *namelink->GetText() == '\n'
                ? namelink->GetText() + 1
                : namelink->GetText()); // Some names start with a newline. Just
                                        // remove it.
        std::string projecturl(namelink->Attribute("href"));
        std::string summary(div2->GetText());
        std::string iconurl(
            img_icon
                ? (img_icon->Attribute("src") ? img_icon->Attribute("src") : "")
                : "");

        SearchData data{ std::move(projectname), std::move(projecturl),
                         std::move(iconurl), std::move(summary) };

        if (SimpleCheckIsMod(data)) {
            emit FoundSearchData(data);
            data_cache.push_back(std::move(data));
        }
    }

    emit SearchParseFinished();
}

CurseMetaMod ModFinder::ConvertToMetaMod(const SearchData &object)
{
    CurseMetaMod mod;
    mod.addonname   = object.projectname;
    mod.description = object.summary;
    mod.iconurl     = object.iconurl;

    auto projectid = object.projecturl.find("projectID");

    if (projectid == std::string::npos) {
        qCritical() << "Provided search data didnt have a projectID in its url";
    }
    projectid += 10;
    std::string projectid_v(object.projecturl.c_str() + projectid);

    mod.addonid = std::stoi(projectid_v);
    cursemeta_resolve(mod);

    return mod;
}

bool ModFinder::SimpleCheckIsMod(const SearchData &data)
{
    const auto &url = data.projecturl;
    const auto &pos = url.find("gameCategorySlug=mc-mods");
    return pos != std::string::npos;
}
