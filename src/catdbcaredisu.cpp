#include "catdbcaredisu.h"
#include <cudata.h>
#include <optional>
#include <chrono>

class CaTDBCaRedisU_P {
public:
    CaTDBCaRedisU_P() : redis(nullptr) {}
    sw::redis::Redis *redis;
    std::string error;

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
    std::vector<std::string> attcnf_keys;
};

CaTDBCaRedisU::CaTDBCaRedisU(const CuData &o) {
    d = new CaTDBCaRedisU_P;

    // Connection options: see also https://github.com/sewenew/redis-plus-plus#connection-options
    sw::redis::ConnectionOptions co;
    if(o.containsKey("redis_ho")) co.host = o.s("redis_ho");
    if(o.s("redis_po").length() > 0 && atoi(o.s("redis_po").c_str()) > 0) {
        co.port = atoi(o.s("redis_po").c_str());
    }
    // You don't need to check whether Redis object connects to server successfully.
    // If Redis fails to create a connection to Redis server, or the connection is
    // broken at some time, it throws an exception of type Error when you try to send
    // command with Redis
    // (https://github.com/sewenew/redis-plus-plus#api-reference)
    d->redis = new sw::redis::Redis(co);

}

CaTDBCaRedisU::~CaTDBCaRedisU() {
    delete d->redis;
    delete d;
}

bool CaTDBCaRedisU::update(const std::string &src, const CuData &c) {
    d->error.clear();
    if(d->attcnf_keys.size() == 0)
        d->attcnf_keys = c.keys();
    std::unordered_map<std::string, std::string> m;
    for(const std::string& k : d->attcnf_keys)
        m[k] = c.s(k);
    try {
        d->redis->hmset(src, m.begin(), m.end());
    }
    catch(const sw::redis::Error& e) {
        d->error = std::string(e.what());
    }
    return d->error.length() == 0;
}

bool CaTDBCaRedisU::get(const std::string& src, CuData& out) {
    d->error.clear();
    std::vector<std::optional<std::string> > vals;
    try {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        d->redis->hmget(src, d->attcnf_keys.begin(), d->attcnf_keys.end(), std::back_inserter(vals));
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        printf("CaTDBCaRedisU::get: hmget took %ldus\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());

        int i = 0;
        for(const std::optional<std::string>& val : vals) {
            const std::string& k = d->attcnf_keys[i++];
            if(val) {
                printf("CaTDBCaRedisU.get: %s --> %s\n", k.c_str(), val->c_str());
                out[k] = val.value();
            }
            else {
                out[k] = "";
                printf("CaTDBCaRedisU.get: %s --> null value\n", k.c_str());
            }
        }
    }
    catch(const sw::redis::Error& e) {
        d->error = std::string(e.what());
    }
    return d->error.length() == 0;
}

std::string CaTDBCaRedisU::error() const {
    return d->error;
}
