#pragma once
#include <QPixmap>
#include <mutex>
#include <string>
#include <unordered_map>

class IconCache {

public:
    static IconCache &Get();

    const QPixmap &GetIcon(const std::string &key) const;
    void           StoreIcon(const std::string &key, const QPixmap &icon);
    bool           HasIcon(const std::string &key) const;

    // TODO: Maybe some operator overlaods
private:
    IconCache() = default;

    mutable std::mutex                       iconsLock;
    std::unordered_map<std::string, QPixmap> icons;
};
