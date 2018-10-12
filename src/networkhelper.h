#pragma once
#include <QNetworkAccessManager>

inline auto &GetNetworkManager()
{
    static QNetworkAccessManager networkmanager;
    return networkmanager;
}

inline auto GetDefaultRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    return request;
}

inline auto GetDefaultRequest(const std::string &url)
{
    return GetDefaultRequest(QUrl(url.c_str()));
}

inline auto GetDefaultRequest(const QString &url)
{
    return GetDefaultRequest(QUrl(url));
}
