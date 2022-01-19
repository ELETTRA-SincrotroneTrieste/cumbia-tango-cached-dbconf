#ifndef CATDBCACHEREDISU_H
#define CATDBCACHEREDISU_H

#include <sw/redis++/redis++.h>
#include <cuservicei.h>

class CuTDBRedisService_P;
class CuData;
class Cumbia;

class CuTDBRedisService : public CuServiceI
{
public:
    enum Type { CuTDBRedisServiceType = CuServices::User + 32 };
    CuTDBRedisService(const CuData &o);
    ~CuTDBRedisService();

    std::string error() const;
    bool get(const std::string &src, CuData &out);
    bool src_request(const std::string& src, std::string &response);

    const sw::redis::ConnectionOptions &connection_options() const;

    std::string last_curl_message() const;
    bool curl_error() const;

private:
    CuTDBRedisService_P *d;

    // CuServiceI interface
public:
    std::string getName() const;
    CuServices::Type getType() const;
};

class CuTDBRedisServiceMaker {
public:
    CuTDBRedisService* get_cumbia_redis_service(Cumbia* c, const CuData &o) const;
};

#endif // CATDBCACHEREDISU_H
