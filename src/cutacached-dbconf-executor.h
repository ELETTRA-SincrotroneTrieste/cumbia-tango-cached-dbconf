#ifndef CASUPERVISOR_H
#define CASUPERVISOR_H

#include <vector>
#include <cuthreadlistener.h>
#include <tango.h>
#include <cudata.h>
#include <cutimerlistener.h>

#include <cutconfigactivity_executor_i.h>

class CuTgDbCachedDbConfX_P;
class CuTDBRedisService;

class CuTgDbCachedDbConfX : public CuTConfigActivityExecutor_I
{
public:
    CuTgDbCachedDbConfX(CuTDBRedisService *s);
    virtual ~CuTgDbCachedDbConfX();

private:
    CuTgDbCachedDbConfX_P *d;

    bool m_cache_srv_notify(const std::string& src, string &response) const;
    std::string m_make_fully_qualified_src(const std::string& devna,
                                           const std::string& attna,
                                           const std::string& proto) const;

    // CuTConfigActivityExecutor_I interface
public:
    bool get_command_info(Tango::DeviceProxy *, const string &, CuData &) const;

    /*!
     * @see CuTConfigActivityExecutor_I::get_att_config in cutconfigactivity_executor_i.h
     */
    bool get_att_config(Tango::DeviceProxy *dev, const string &attribute, CuData &dres, bool skip_read_att, const string &devnam) const;

};

#endif // CASUPERVISOR_H
