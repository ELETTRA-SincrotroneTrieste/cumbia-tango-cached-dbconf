#include "cutcachedconffactories.h"
#include "cutdbredis-service.h" // for CaTDBRedisServiceType
#include "cutacached-dbconf-executor.h"
#include <cutconfiguration.h>
#include <cumbiatango.h>
#include <cuserviceprovider.h>
#include <cuservices.h>

CuTangoActionI *CuTCachedReaderConfFactory::create(const string &s, CumbiaTango *ct) const {
    CuTDBRedisServiceMaker sm;
    CuTDBRedisService *reds = sm.get_cumbia_redis_service(ct, m_o);
    return new CuTConfiguration(s, ct, CuTangoActionI::ReaderConfig, opts, dtag, reds != nullptr ? new CuTgDbCachedDbConfX(reds) : nullptr);
}

CuTangoActionI::Type CuTCachedReaderConfFactory::getType() const {
    return CuTangoActionI::ReaderConfig;
}

CuTangoActionI *CuTCachedWriterConfFactory::create(const string &s, CumbiaTango *ct) const {
    CuTDBRedisServiceMaker sm;
    CuTDBRedisService *reds = sm.get_cumbia_redis_service(ct, m_o);
    CuTConfiguration *w = new CuTConfiguration(s, ct, CuTangoActionI::ReaderConfig, opts, dtag, reds != nullptr ? new CuTgDbCachedDbConfX(reds) : nullptr);
    if(opts.containsKey("fetch_props"))
        w->setDesiredAttributeProperties(opts["fetch_props"].toStringVector());
    return w;
}

CuTangoActionI::Type CuTCachedWriterConfFactory::getType() const {
    return CuTangoActionI::WriterConfig;
}
