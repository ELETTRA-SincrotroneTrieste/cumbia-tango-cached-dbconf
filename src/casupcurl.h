#ifndef CASUPCURL_H
#define CASUPCURL_H

#include <string>
#include <curl/curl.h>
#include <vector>
#include <map>

class CaSupCurlPrivate;
class CaSupCurl;

class CaSupCurlWriteFuncData {
public:
    CaSupCurlWriteFuncData(const std::string& _chan) {
		chan = _chan;
	}

	std::string chan;
	int su_cnt;
};

class CaSupCurl
{
public:
	CaSupCurl(const std::string &url, bool ssl_verifypeer);
	virtual ~CaSupCurl();

    std::map<std::string, std::string> xmit(const std::map<std::string, std::string> &datamap);

	static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

	static int socket_callback(CURL *easy,      /* easy handle */
	                           curl_socket_t s, /* socket */
	                           int what,        /* what to wait for */
	                           void *userp,     /* private callback pointer */
	                           void *socketp);
	static int start_timeout(CURLM *multi, long timeout_ms, void *userp);
    const std::string& response() const;

private:
	CaSupCurlPrivate *d;
};

#endif // CASUPCURL_H
