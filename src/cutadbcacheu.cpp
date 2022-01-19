#include "cutadbcacheu.h"
#include <cudata.h>

CaTDBCacheU::CaTDBCacheU()
{

}

Tango::DeviceProxy *CaTDBCacheU::m_get_dev(const std::string &nam) {
    error.clear();
    Tango::DeviceProxy *dev = nullptr;
    try {
        dev = new Tango::DeviceProxy(nam.c_str());
    }
    catch(const Tango::DevFailed& e) {
        dev = nullptr;
        error = std::string("device ") + nam + " connection error: " + tg_strerror(e);
    }
    return dev;
}

int CaTDBCacheU::m_att_conf_change_subscribe(Tango::DeviceProxy *dev, const std::string&devna, const string &attna, Tango::CallBack* cb) {
    int evid = -1;
    error.clear();
    try {
        evid = dev->subscribe_event(attna, Tango::ATTR_CONF_EVENT, cb, true);
    }
    catch(const Tango::DevFailed& e) {
        error = "device " + devna + " subscribe error: " + tg_strerror(e);
    }
    return evid;
}

std::string CaTDBCacheU::m_dev_get_name(Tango::DeviceProxy *dev) {
    std::string n;
    error.clear();
    try {
        n = dev->name();
    }
    catch(const Tango::DevFailed& e) {
        error = "dev->name failed: " + tg_strerror(e);
    }
    return n;
}

std::string CaTDBCacheU::m_mkattsrc(const string &dev, const string &att) const {
    return dev + "/" + att;
}

void CaTDBCacheU::m_fill_from_attconf(const Tango::AttributeInfoEx *ai, CuData &dat)
{
    dat["type"] = "property";
    dat["df"] = ai->data_format;
    dat["dfs"] = format_to_str(ai->data_format); /* as string */
    dat["dt"] = ai->data_type;
    dat["description"] = ai->description;
    ai->display_unit != std::string("No display unit") ? dat["display_unit"] = ai->display_unit : dat["display_unit"] = "";
    dat["format"] = ai->format;
    dat["label"] = ai->label;
    dat["max_alarm"] = ai->max_alarm;
    dat["max_dim_x"] = ai->max_dim_x;
    dat["max_dim_y"] = ai->max_dim_y;
    dat["max"] = ai->max_value;
    dat["min"] = ai->min_value;
    dat["min_alarm"] = ai->min_alarm;
    dat["name"] = ai->name;
    dat["standard_unit"] = ai->standard_unit;
    dat["unit"] = ai->unit;
    dat["writable"] = ai->writable;
    dat["writable_attr_name"] = ai->writable_attr_name;
    dat["disp_level"] = ai->disp_level;
    dat["root_attr_name"] = ai->root_attr_name; // Root attribute name (in case of forwarded attribute)

    Tango::AttributeAlarmInfo aai = ai->alarms;
    dat["delta_t"] = aai.delta_t;
    dat["delta_val"] = aai.delta_val;
    dat["max_alarm"] = aai.max_alarm;
    dat["min_alarm"] = aai.min_alarm;
    dat["max_warning"] = aai.max_warning;
    dat["min_warning"] = aai.min_warning;

    Tango::AttributeEventInfo ei = ai->events;
    dat["archive_abs_change"] = ei.arch_event.archive_abs_change;
    dat["archive_period"] = ei.arch_event.archive_period;
    dat["archive_rel_change"] = ei.arch_event.archive_rel_change;

    dat["abs_change"] = ei.ch_event.abs_change;
    dat["rel_change"] = ei.ch_event.rel_change;

    dat["periodic_period"] = ei.per_event.period;

    // dim_x property contains the actual number of x elements
    long int dimx = dat["value"].getSize(); // if !contains value, empty variant, 0 dimx
    if(dimx > 0)
        dat["dim_x"] = dimx;
}

string CaTDBCacheU::format_to_str(Tango::AttrDataFormat f) const {
    switch(f) {
    case Tango::SCALAR:
        return "scalar";
    case Tango::SPECTRUM:
        return "vector";
    case Tango::IMAGE:
        return "matrix";
    default:
        return "data format unknown";
    }
}

std::string CaTDBCacheU::tg_strerror(const Tango::DevFailed &e) const
{
    std::string msg;
    if(e.errors.length() > 0)
        msg = tg_strerror(e.errors);
    return msg;
}
