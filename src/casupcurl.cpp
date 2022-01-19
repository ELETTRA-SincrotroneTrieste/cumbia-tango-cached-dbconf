#include "casupcurl.h"
#include "casupjsoniz.h"
#include <curl/curl.h>
#include <cumacros.h>

class CaSupCurlPrivate {
public:
    CURL *curl;
    std::string url, message, response;
    bool ok, ssl_verify_peer;
};

CaSupCurl::CaSupCurl(const std::string& url, bool ssl_verifypeer) {
    d = new CaSupCurlPrivate;
    d->url = url + "/bu/src-bundle";
    d->ssl_verify_peer = ssl_verifypeer;
    d->curl = curl_easy_init();
    d->ok = d->curl != nullptr;
}

CaSupCurl::~CaSupCurl() {
    curl_easy_cleanup(d->curl);
    delete d;
}

std::map<std::string, std::string> CaSupCurl::xmit(const std::map<std::string,std::string> &datamap) {
    std::map<std::string, std::string> resultmap;
    int cnt = 0;
    bool ok;
    std::string message;
    CaSupJsoniz jiz;
    for(const auto& [chan, json] : datamap) {
        ++cnt;
        struct curl_slist *slist = nullptr;
        std::string chanhdr = "X-Channel: " + chan;
        std::string content_len = "Content-Length: " + std::to_string(json.length());
        char errbuf[CURL_ERROR_SIZE];
        curl_slist_append(slist, "Accept: application/json");
        curl_slist_append(slist, "Content-Type: application/json");
        curl_slist_append(slist, chanhdr.c_str());
        curl_slist_append(slist, content_len.c_str());
        curl_easy_setopt(d->curl, CURLOPT_URL, d->url.c_str());
        curl_easy_setopt(d->curl, CURLOPT_HEADER, slist);
        curl_easy_setopt(d->curl, CURLOPT_POSTFIELDS, json.c_str());
        curl_easy_setopt(d->curl, CURLOPT_ERRORBUFFER, errbuf);
        // CURLOPT_SSL_VERIFYPEER true by default in CaOptParser
        curl_easy_setopt(d->curl, CURLOPT_SSL_VERIFYPEER, d->ssl_verify_peer);
        // suppress response to stdout
        curl_easy_setopt(d->curl, CURLOPT_WRITEFUNCTION, CaSupCurl::write_callback);
        CaSupCurlWriteFuncData *wd = new CaSupCurlWriteFuncData(std::to_string(cnt));
        d->response.clear();
        curl_easy_setopt(d->curl, CURLOPT_WRITEDATA, (void *) &d->response );

        CURLcode cuco = curl_easy_perform(d->curl);
        ok = (cuco == CURLE_OK);
        // if CURL successful let message hold the server reply
        ok ?  message = d->response :
                message = jiz.make_err_msg(std::string("CaSupCurl::xmit: ")  + curl_easy_strerror(cuco) + ": "
                                           + errbuf + ": code " + std::to_string(cuco));
        curl_slist_free_all(slist);
        resultmap[chan] = message;
        delete wd;
    }
    return resultmap;
}

size_t CaSupCurl::write_callback(char *contents, size_t size, size_t nmemb, void *userdata) {
    std::string *response = static_cast<std::string *>(userdata);
    response->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int CaSupCurl::socket_callback(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp) {
    return 0;
}

int CaSupCurl::start_timeout(CURLM *multi, long timeout_ms, void *userp) {
    return 0;
}

const std::string &CaSupCurl::response() const {
    return d->response;
}
