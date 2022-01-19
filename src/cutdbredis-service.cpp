#include "cutdbredis-service.h"
#include "cutadbcache-curl.h"
#include <cudata.h>
#include <optional>
#include <chrono>
#include <cuservices.h>
#include <cuserviceprovider.h>
#include <cumbia.h>

class CuTDBRedisService_P {
public:
    CuTDBRedisService_P() : redis(nullptr),
        attcnf_keys { "type", "df", "dfs", "dt", "description", "display_unit",
                      "format", "label", "max_alarm", "max_dim_x", "max_dim_y",
                      "max", "min", "min_alarm", "name", "standard_unit", "unit",
                      "writable", "writable_attr_name", "root_attr_name",
                      "delta_t", "delta_val", "max_alarm", "min_alarm", "max_warning",
                      "min_warning", "archive_abs_change", "archive_period",
                      "archive_rel_change", "abs_change", "rel_change", "periodic_period" },
        cacurl(nullptr), curl_ssl_verify_peer(true) { }
    sw::redis::Redis *redis;
    sw::redis::ConnectionOptions conn_o;
    std::string error;

    // http url pointing at ca-tango-db-cache-mgr service
    // in configuration file:
    //
    // caserver-sync:redis-host=pwma-dev.elettra.eu
    // caserver-sync:ca-tango-db-cache-mgr-url=http://taeyang.elettra.eu:9296
    std::string dbcachemgr_url;

    // attribute configuration keys used by cumbia apps
    // see CuTangoWorld::fillFromAttributeConfig and
    // CaTDBCacheU::m_fill_from_attconf
    //
    // NOTE
    //  keeping a fixed list of keys lets us use hmget
    //  avoiding hgetall() and hkeys():
    //    It's always a bad idea to call `hkeys` on a large hash, since it will block Redis.
    //    It's always a bad idea to call `hgetall` on a large hash, since it will block Redis.
    //  https://github.com/sewenew/redis-plus-plus/blob/master/src/sw/redis%2B%2B/redis.h
    const std::vector<std::string> attcnf_keys;

    // CURL to handle requests to the tango attribute config db cache manager
    // https://gitlab.elettra.eu/puma/server/ca-tango-db-cache-mgr
    CuTaDbCacheCURL *cacurl;
    bool curl_ssl_verify_peer;
};

CuTDBRedisService::CuTDBRedisService(const CuData &o) {
    d = new CuTDBRedisService_P;

    printf("CuTDBRedisService::CuTDBRedisService: options %s\n", datos(o));
    // Connection options: see also https://github.com/sewenew/redis-plus-plus#connection-options
    if(o.containsKey("redis_ho")) d->conn_o.host = o.s("redis_ho");
    if(o.s("redis_po").length() > 0 && atoi(o.s("redis_po").c_str()) > 0) {
        d->conn_o.port = atoi(o.s("redis_po").c_str());
    }

    // "ca-tango-db-cache-mgr-host" -> taeyang.elettra.eu
    // "ca-tango-db-cache-mgr-port" -> 9296
    d->dbcachemgr_url = o.s("ca-tango-db-cache-mgr-url");
    // You don't need to check whether Redis object connects to server successfully.
    // If Redis fails to create a connection to Redis server, or the connection is
    // broken at some time, it throws an exception of type Error when you try to send
    // command with Redis
    // (https://github.com/sewenew/redis-plus-plus#api-reference)
    d->redis = new sw::redis::Redis(d->conn_o);
}

CuTDBRedisService::~CuTDBRedisService() {
    delete d->redis;
    delete d;
}

bool CuTDBRedisService::get(const std::string& src, CuData& out) {
    bool complete = true;
    d->error.clear();
    std::vector<std::optional<std::string> > vals;
    try {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        long long count = d->redis->exists(src);
        complete = count > 0;
        if(count > 0) {
            d->redis->hmget(src, d->attcnf_keys.begin(), d->attcnf_keys.end(), std::back_inserter(vals));
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            printf("CuTDBRedisService::get: hmget took %ldus\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
            int i = 0;
            for(const std::optional<std::string>& val : vals) {
                const std::string& k = d->attcnf_keys[i++];
                if(val) {
                    printf("CuTDBRedisService.get: %s --> %s\n", k.c_str(), val->c_str());
                    out[k] = val.value();
                }
                else {
                    out[k] = "";
                    printf("CuTDBRedisService.get: %s --> null value\n", k.c_str());
                    complete = false;
                }
            }
        }
        else
            printf("CuTDBRedisService::get: src %s \e[1;31mdoes not exist\e[0m\n", src.c_str());
    }
    catch(const sw::redis::Error& e) {
        d->error = std::string(e.what());
    }
    return d->error.length() == 0 && complete;
}

bool CuTDBRedisService::src_request(const std::string &src, std::string& response) {
    if(d->cacurl == nullptr) {
        d->cacurl = new CuTaDbCacheCURL(d->dbcachemgr_url, d->curl_ssl_verify_peer);
    }
    return d->cacurl->send(std::vector<std::string> {src}, response);
}

const sw::redis::ConnectionOptions& CuTDBRedisService::connection_options() const {
    return d->conn_o;
}

std::string CuTDBRedisService::last_curl_message() const {
    return d->cacurl != nullptr ? d->cacurl->message() : "";
}

bool CuTDBRedisService::curl_error() const {
    return d->cacurl != nullptr ? d->cacurl->message().length() > 0 : false;
}

std::string CuTDBRedisService::error() const {
    return d->error;
}


std::string CuTDBRedisService::getName() const {
    return "catdb-redis-service";
}

CuServices::Type CuTDBRedisService::getType() const {
    return static_cast<CuServices::Type>(CuTDBRedisServiceType);
}

CuTDBRedisService *CuTDBRedisServiceMaker::get_cumbia_redis_service(Cumbia *c, const CuData& o) const {
    CuTDBRedisService *redis_s = nullptr;
    CuServiceProvider *sp = c->getServiceProvider();
    if(sp) {
        redis_s = static_cast<CuTDBRedisService *>
                (sp->get(static_cast<CuServices::Type>(CuTDBRedisService::CuTDBRedisServiceType)));
        if(!redis_s) {
            redis_s = new CuTDBRedisService(o);
            sp->registerService(redis_s->getType(), redis_s);
        }
        return redis_s;
    }
    return redis_s;
}
