// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ecurl.h"
#include "modemcontroller.h"
#include "settingsmanager.h"

#include <netdb.h>

ECurl::ECurl()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    ares_library_init(ARES_LIB_INIT_ALL);
}

ECurl::~ECurl()
{
    ares_library_cleanup();
    curl_global_cleanup();
}

QByteArray ECurl::networkRequest(const QString &url, const QByteArray &data) const
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        return QByteArray();
    }
    struct curl_slist *host = NULL;

    // provide a buffer to store errors in
    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    QString ifaceName = ModemController::instance().ifaceName;
    curl_easy_setopt(curl, CURLOPT_INTERFACE, (SL("if!") + ifaceName).toUtf8().constData());

    // only attempt to resolve if not using proxy
    if (SettingsManager::self()->mmsProxy().isEmpty()) {
        QString dnsServers = ModemController::instance().dnsServers;
        CURLcode supported = curl_easy_setopt(curl, CURLOPT_DNS_INTERFACE, ifaceName.toUtf8().constData());

        // use c-ares if curl was built without dns resolver support
        if (supported == CURLE_UNKNOWN_OPTION || supported == CURLE_NOT_BUILT_IN) {
            ares_channel channel;

            int aresReturn = ares_init(&channel);

            if (aresReturn != ARES_SUCCESS) {
                qDebug() << "Ares init failed:" << ares_strerror(aresReturn);
            } else {
                ares_set_local_dev(channel, ifaceName.toUtf8().constData());
                ares_set_servers_csv(channel, dnsServers.toUtf8().constData());

                CURLU *curlUrl = curl_url();
                curl_url_set(curlUrl, CURLUPART_URL, url.toUtf8().constData(), 0);
                char *hostname = url.toUtf8().data();
                curl_url_get(curlUrl, CURLUPART_HOST, &hostname, 0);
                curl_url_cleanup(curlUrl);

                char *hostIp = NULL;
                ares_gethostbyname(channel, hostname, AF_UNSPEC, aresResolveCallback, (void *)&hostIp);
                aresResolveWait(channel);

                if (hostIp == NULL) {
                    qDebug() << "Failed to resolve:" << hostname;
                }

                const char *resolve = QByteArray("+").append(hostname).append(":80:[").append(hostIp).append("]").constData();
                host = curl_slist_append(NULL, resolve);
                curl_easy_setopt(curl, CURLOPT_RESOLVE, host);

                curl_free(hostname);
                ares_destroy(channel);
            }
        } else {
            curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, dnsServers.toUtf8().constData());
        }
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

    // Fake user agent for carrier network compatibility
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Android MmsLib/1.0");

    if (!SettingsManager::self()->mmsProxy().isEmpty()) {
        QString proxy = SettingsManager::self()->mmsProxy() + SL(":") + QString::number(SettingsManager::self()->mmsPort());
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.toUtf8().constData());
    }

    if (!data.isEmpty()) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.constData());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());

        QString len = QString::number(data.length());
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/vnd.wap.mms-message");
        headers = curl_slist_append(headers, (SL("Content-Length: ") + len).toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    QByteArray response;
    const QByteArray *pointer = &response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(host);

    if (res != CURLE_OK) {
        qDebug() << "Network Error:" << errbuf;
        return QByteArray();
    } else {
        return response;
    }
}

size_t ECurl::curlWriteFunction(char *chunk, size_t size, size_t len, QByteArray *response)
{
    response->append(chunk, static_cast<int>(size * len));
    return size * len;
}

void ECurl::aresResolveCallback(void *arg, int status, int timeouts, struct hostent *host)
{
    char **hostIp = (char **)arg;

    // when the ares handle is getting destroyed, the 'arg' pointer may not
    // be valid so only defer it when we know the 'status' says its fine!
    if (!host || status != ARES_SUCCESS) {
        return;
    }

    char ip[INET6_ADDRSTRLEN];

    for (int i = 0; host->h_addr_list[i]; ++i) {
        ares_inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
        if (i > 0) {
            strcat(*hostIp, ",");
            strcat(*hostIp, strdup(ip));
        } else {
            *hostIp = strdup(ip);
        }

        qDebug() << "Found IP for" << host->h_name << *hostIp;
    }

    (void)timeouts; // ignored
}

void ECurl::aresResolveWait(ares_channel channel)
{
    qint64 start = QTime::currentTime().msecsSinceStartOfDay();

    for (;;) {
        struct timeval *tvp, tv;
        fd_set read_fds, write_fds;
        int nfds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds);
        if (nfds == 0) {
            break;
        }
        tvp = ares_timeout(channel, NULL, &tv);
        int count = select(nfds, &read_fds, &write_fds, NULL, tvp);
        if (count == -1) {
            qDebug() << "Error waiting for c-ares read/write descriptors";
            break;
        }
        if (QTime::currentTime().msecsSinceStartOfDay() - start > 15 * 1000) {
            qDebug() << "Resovler timeout";
            break;
        }
        ares_process(channel, &read_fds, &write_fds);
    }
}
