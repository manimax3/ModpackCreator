#include "iconcache.h"

IconCache &IconCache::Get()
{
    static IconCache instance;
    return instance;
}

const QPixmap &IconCache::GetIcon(const std::string &key) const
{
    std::lock_guard<std::mutex> lck(iconsLock);
    return icons.at(key);
}

void IconCache::StoreIcon(const std::string &key, const QPixmap &icon)
{
    std::lock_guard<std::mutex> lck(iconsLock);
    icons.insert_or_assign(key, icon);
}

bool IconCache::HasIcon(const std::string &key) const
{
    std::lock_guard<std::mutex> lck(iconsLock);
    return icons.find(key) != std::end(icons);
}
