#ifndef CASUPCURL_H
#define CASUPCURL_H

#include <string>
#include <curl/curl.h>
#include <vector>
#include <map>

class CuTaDbCacheCURL_P;
class CuTaDbCacheCURL;

class CuTaDbCacheCURLWriteFuncData {
public:
    CuTaDbCacheCURLWriteFuncData(const std::string& _chan) {
		chan = _chan;
	}

	std::string chan;
	int su_cnt;
};

class CuTaDbCacheCURL
{
public:
    CuTaDbCacheCURL(const std::string &url, bool ssl_verifypeer);
    virtual ~CuTaDbCacheCURL();

    bool send(const std::vector<std::string>& srcs, std::string &response);

	static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

	static int socket_callback(CURL *easy,      /* easy handle */
	                           curl_socket_t s, /* socket */
	                           int what,        /* what to wait for */
	                           void *userp,     /* private callback pointer */
	                           void *socketp);
	static int start_timeout(CURLM *multi, long timeout_ms, void *userp);
    const std::string message() const;

private:
    CuTaDbCacheCURL_P *d;
};

#endif // CASUPCURL_H
