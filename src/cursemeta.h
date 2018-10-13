#pragma once
#include <QObject>
#include <list>
#include <string>
#include <tuple>

#include "nlohmann/json_fwd.hpp"

struct CurseMetaMod {
    // Fileid, version
    using FileId = std::tuple<int, std::string>;

    std::string       addonname;
    int               addonid = 0;
    std::list<FileId> fileids;
    std::string       description;
    std::string       iconurl;

    bool modvalid = false;
};

struct CurseMetaMcVersion {
    std::string version;
    bool        approved;
    std::string dateModified;
};

struct CurseMetaForge {
    std::string name;
    std::string mcversion;
    std::string dateModified;
    bool        latest      = false;
    bool        recommended = false;
};

class McVersionFinder : public QObject {
    Q_OBJECT

public:
    explicit McVersionFinder(QObject *parent = nullptr);
    ~McVersionFinder();

    void StartSearching();

signals:
    void McVersionFound(CurseMetaMcVersion version);
    void ForgeVersionFound(CurseMetaForge forgeversion);

private slots:
    void RequestFinished(class QNetworkReply *reply);

private:
    QMetaObject::Connection mcconn;
    QMetaObject::Connection forgeconn;
};

void           cursemeta_resolve(CurseMetaMod &mod);
std::list<int> cursemeta_file_dependencies(int addonid, int fileid);

void to_json(nlohmann::json &j, const CurseMetaMod &mod);
void from_json(const nlohmann::json &j, CurseMetaMod &mod);

void from_json(const nlohmann::json &j, CurseMetaMcVersion &mcversion);
void from_json(const nlohmann::json &j, CurseMetaForge &forgeversion);

