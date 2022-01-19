#ifndef CATDBCACHEU_H
#define CATDBCACHEU_H

#include <tango.h>
#include <string>

class CuData;

class CaTDBCacheU
{
public:
    CaTDBCacheU();

    Tango::DeviceProxy *m_get_dev(const std::string &nam);
    int m_att_conf_change_subscribe(Tango::DeviceProxy *dev, const string &devna, const std::string& attna, Tango::CallBack *cb);

    // -----------------------------------------------------------------------------+
    // copied from CuTangoWorld (to avoid including cumbia-tango as dependency)
    void m_fill_from_attconf(const Tango::AttributeInfoEx *ai, CuData &dat);
    std::string tg_strerror(const Tango::DevFailed &e) const;
    string format_to_str(Tango::AttrDataFormat f) const;
    // -----------------------------------------------------------------------------+

    std::string error;
    std::string m_dev_get_name(Tango::DeviceProxy *dev);
    std::string m_mkattsrc(const std::string& dev, const std::string& att) const;
    std::string m_mkcmdsrc(const std::string& dev, const std::string& cmdnam) const;
};

#endif // CATDBCACHEU_H
