#include "cutacached-dbconf-executor.h"
#include "cutadbcjsoniz.h"
#include "cutadbcache-curl.h"
#include "cutadbcacheu.h"
#include "cutdbredis-service.h"
#include "cutcachedconffactories.h"
#include "cutadbcache-curl.h"

#include <map>
#include <cumbia.h>
#include <cudata.h>
#include <cutango-world.h>
#include <chrono>

#define _BUFSIZ 512

class CaTgDbCSrcData {
public:
    CaTgDbCSrcData(const std::string& s) : src(s) {}
    std::string src;
};

class CuTgDbCachedDbConfX_P {
public:
    CuTgDbCachedDbConfX_P(CuTDBRedisService* s) :
          redis_s(s){
    }
    // redis shared Cumbia service (single instance)
    CuTDBRedisService *redis_s;
    std::string error;
};

CuTgDbCachedDbConfX::CuTgDbCachedDbConfX(CuTDBRedisService* s) {
    d = new CuTgDbCachedDbConfX_P(s);
}

CuTgDbCachedDbConfX::~CuTgDbCachedDbConfX() {
    delete d;
}

bool CuTgDbCachedDbConfX::m_cache_srv_notify(const string &src, std::string& response) const {
    return d->redis_s->src_request(src, response);
}

std::string CuTgDbCachedDbConfX::m_make_fully_qualified_src(const string &devna,
                                                            const string &attna,
                                                            const string &proto) const {
    std::string src = devna + "/" + attna;
    // tango:// ?
    if(src.find(proto, 0) == std::string::npos)
        src = proto + src;
    return src;
}


bool CuTgDbCachedDbConfX::get_command_info(Tango::DeviceProxy *, const string &, CuData &) const {
    return false;
}

bool CuTgDbCachedDbConfX::get_att_config(Tango::DeviceProxy *dev,
                                         const string &attribute,
                                         CuData &dres,
                                         bool skip_read_att,
                                         const std::string& devnam) const {
    d->error.clear();
    std::vector<std::optional<std::string> > vals;
    bool cache_hit = false;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::string src = m_make_fully_qualified_src(devnam, attribute, "tango://");

    printf("CuTgDbCachedDbConfX::get_att_config searching in cache src \e[1;32m%s\e[0m\n", src.c_str());
    cache_hit = d->redis_s->get(src, dres);
    if(cache_hit && !skip_read_att) {
        CuTangoWorld w;
        w.read_att(dev, attribute, dres);
        // dim_x property contains the actual number of x elements
        long int dimx = dres["value"].getSize(); // if !contains value, empty variant, 0 dimx
        if(dimx > 0)
            dres["dim_x"] = dimx;
    }

    if(!cache_hit) {
        std::string curlre; // curl response
        CuTangoWorld w;
        w.get_att_config(dev, attribute, dres, skip_read_att);
        bool curlok = m_cache_srv_notify(devnam + "/" + attribute, curlre);
        if(!curlok) d->error = d->redis_s->last_curl_message();
        printf("CuTgDbCachedDbConfX::get_att_config: after notifying need new src %s: response %s error %s\n",
               std::string(devnam + "/" + attribute).c_str(), curlre.c_str(), d->error.c_str());
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    printf("CuTgDbCachedDbConfX::get_att_config returns\n%s\nFROM %s\e[0m in \e[1;36m%ldus\e[0m\n",
           datos(dres), cache_hit ? "\e[1;32mCACHE" : "\e[1;35mTANGO DATABASE\e[0m",
           std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
    return d->error.length() == 0;
}
