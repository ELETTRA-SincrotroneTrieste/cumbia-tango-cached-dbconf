#include "cutadbcache-curl.h"
#include "cutadbcjsoniz.h"
#include <curl/curl.h>
#include <cumacros.h>

class CuTaDbCacheCURL_P {
public:
    CURL *curl;
    std::string url, message;
    bool ok, ssl_verify_peer;
};

CuTaDbCacheCURL::CuTaDbCacheCURL(const std::string& url, bool ssl_verifypeer) {
    d = new CuTaDbCacheCURL_P;
    d->url = url;
    d->ssl_verify_peer = ssl_verifypeer;
    d->curl = curl_easy_init();
    d->ok = d->curl != nullptr;
}

CuTaDbCacheCURL::~CuTaDbCacheCURL() {
    curl_easy_cleanup(d->curl);
    delete d;
}

bool CuTaDbCacheCURL::send(const std::vector<std::string>& srcs, std::string& response) {
    std::map<std::string, std::string> resultmap;
    bool ok;
    CaSupJsoniz jiz;
    std::string json = jiz.jsonize(srcs);
    struct curl_slist *slist = nullptr;
    std::string content_len = "Content-Length: " + std::to_string(json.length());
    char errbuf[CURL_ERROR_SIZE];
    curl_slist_append(slist, "Accept: application/json");
    curl_slist_append(slist, "Content-Type: application/json");
    curl_slist_append(slist, content_len.c_str());
    curl_easy_setopt(d->curl, CURLOPT_URL, d->url.c_str());
    curl_easy_setopt(d->curl, CURLOPT_HEADER, slist);
    curl_easy_setopt(d->curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(d->curl, CURLOPT_ERRORBUFFER, errbuf);
    // CURLOPT_SSL_VERIFYPEER true by default in CaOptParser
    curl_easy_setopt(d->curl, CURLOPT_SSL_VERIFYPEER, d->ssl_verify_peer);
    // suppress response to stdout
    curl_easy_setopt(d->curl, CURLOPT_WRITEFUNCTION, CuTaDbCacheCURL::write_callback);
    response.clear();
    curl_easy_setopt(d->curl, CURLOPT_WRITEDATA, (void *) &response );

    CURLcode cuco = curl_easy_perform(d->curl);
    ok = (cuco == CURLE_OK);
    // if CURL successful let message hold the server reply
    ok ? d->message =  "" : d->message = jiz.make_err_msg(std::string("CuTaDbCacheCURL: CURL error: ")  + curl_easy_strerror(cuco) + ": "
                                       + errbuf + ": code " + std::to_string(cuco));
    curl_slist_free_all(slist);
    return ok;
}

size_t CuTaDbCacheCURL::write_callback(char *contents, size_t size, size_t nmemb, void *userdata) {
    std::string *response = static_cast<std::string *>(userdata);
    response->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int CuTaDbCacheCURL::socket_callback(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp) {
    return 0;
}

int CuTaDbCacheCURL::start_timeout(CURLM *multi, long timeout_ms, void *userp) {
    return 0;
}

const std::string CuTaDbCacheCURL::message() const {
    return d->message;
}
