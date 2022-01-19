#include "cutadbcjsoniz.h"
#include <nlohmann/json.hpp>
#include <cudata.h>

CaSupJsoniz::CaSupJsoniz()
{

}

/*!
 * \brief CaSupJsoniz::jsonize returns a vector of json requests grouped by cli_id
 * \param dl vector of data as CuData
 * \return a vector of json requests to use with caserver, each element refers to a different cli_id
 */
std::string CaSupJsoniz::jsonize(const std::vector<std::string>& srcs) const {
    nlohmann::json out;
    nlohmann::json src_array;
    for(const std::string& s : srcs) {
        nlohmann::json jsrc;
        jsrc["src"] = s;
        src_array.push_back(jsrc);
    }
    out["srcs"] = src_array;
    return out.dump();
}

void CaSupJsoniz::extract(const std::string &json, std::list<CuData> &dl) const {

}

const std::string CaSupJsoniz::make_msg_ok() const {
    nlohmann::json o;
    o["msg"] = "ok";
    return o.dump();
}


const std::string CaSupJsoniz::make_scheduled_subscribe(const std::string& msg, int scheduled_cnt, int queuepos, int queuetot) const {
    nlohmann::json o;
    if(msg.length() > 0)
        o["msg"] = msg;
    if(queuepos >= 0)
        o["pos"] = queuepos;
    if(queuetot >= 0)
        o["queue-cnt"] = queuetot;
    if(scheduled_cnt >= 0)
        o["scheduled-cnt"] = scheduled_cnt;
    return o.dump();
}

const std::string CaSupJsoniz::make_err_msg(const std::string &msg) const
{
    nlohmann::json o;
    o["msg"] = msg.length() == 0 ? "ok" : msg;
    o["err"] = msg.length() > 0;
    return o.dump();
}

bool CaSupJsoniz::is_err_msg(const std::string &json, std::string &errmsg) const
{
    errmsg.clear();
    if(json.length() > 0)
    {
        try {
            nlohmann::json js = nlohmann::json::parse(json);
            if(js.is_array()) {
                for (nlohmann::json jse : js) {
                    if(jse.contains("err")) {
                        bool err = jse["err"];
                        if(err && jse.contains("msg"))
                            errmsg = jse["msg"];
                    }
                }
            }

        }  catch (const nlohmann::detail::parse_error& pe) {
            perr("CaSupJsoniz::is_err_msg: JSON parse error: %s in \"%s\"", pe.what(), json.c_str());
            errmsg = std::string(pe.what());
        }  catch (const nlohmann::detail::type_error& te) {
            perr("CaSupJsoniz::is_err_msg: nlohmann::json type error: %s", te.what());
            errmsg = std::string(te.what());
        }
    }
    return errmsg.length() > 0;
}
