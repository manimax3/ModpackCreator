#include "modfinder.h"

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
    , gameversion("unsepcified")
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

    for (const auto &data : data_cache) {
        const auto search_data = ParseSearchSite(data);
        for (const auto &search : search_data) {
            mods.push_back(ConvertToMetaMod(search));
        }
    }

    return mods;
}

void ModFinder::DownloadSearchSite()
{
    if (search.length() == 0)
        return;

    static QNetworkAccessManager networkmanager;
    static const std::string     SEARCH_URL
        = "https://minecraft.curseforge.com/search?search=";

    const std::string final_url = SEARCH_URL + search;

    QNetworkRequest request(QUrl(final_url.c_str()));
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    // We wait for this request to be finished
    // TODO: This is so stupid
    QEventLoop pause;

    QObject::connect(&networkmanager, &QNetworkAccessManager::finished, this,
                     [&](QNetworkReply *rep) {
                         std::string data(rep->readAll());
                         data_cache.push_back(std::move(data));
                         pause.quit();
                     });

    networkmanager.get(request);

    pause.exec();
}

std::list<ModFinder::SearchData>
ModFinder::ParseSearchSite(const std::string &data)
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
        return {};
    }

    const auto table_body = elements.front()->FirstChildElement("tbody");

    for (auto result = table_body->FirstChildElement(); result;
         result      = result->NextSiblingElement()) {

        if (std::string_view(result->Attribute("class")) != "results")
            continue;

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
        std::string projectname(namelink->GetText());
        std::string projecturl(namelink->Attribute("href"));
        std::string summary(div2->GetText());

        SearchData data{ std::move(projectname), std::move(projecturl),
                         std::move(summary) };

        foundaddons.push_back(std::move(data));
    }

    return foundaddons;
}

CurseMetaMod ModFinder::ConvertToMetaMod(const SearchData &object)
{
    CurseMetaMod mod;
    mod.addonname   = object.projectname;
    mod.description = object.summary;

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
