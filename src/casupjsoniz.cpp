#include "casupjsoniz.h"
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
std::map<std::string, std::string> CaSupJsoniz::jsonize(const std::vector<CuData> &dl,
                                                        std::map<std::string, std::list<CuData>> & clidmap) const {
    std::map<std::string, std::string> jsonma;
    // group by cli_id
    for(const CuData& d : dl) {
        clidmap[d["cli_id"].toString()].push_back(d);
    }
    for(std::map<std::string, std::list<CuData>>::const_iterator it = clidmap.begin(); it != clidmap.end(); ++it) {
        nlohmann::json src_array;
        for(const CuData& d : it->second) {
            nlohmann::json data_o;
            nlohmann::json o, options;

            // OPTIONS
            data_o["subscribe-only"] = "true";
            data_o["value-only"] = "true";
            // option names list
            options.push_back("subscribe-only");
            options.push_back("value-only");
            options.push_back("recovered-from-srv-conf");
            options.push_back("recovered-by");

            // more option names may follow:
            //
            // options.push_back("blabla");
            // data_o["blabla"] = "blabbbla";
            // "options" key contains the list of option keys
            data_o["options"] = options;
            // END OPTIONS
            //
            // from source (db table name) to src
            data_o["src"] = d["source"].toString();
            data_o["method"] = "S";
            data_o["recovered-from-srv-conf"] = d["conf_id"].toString();
            data_o["recovered-by"] = "casupervisor";
            data_o["channel"] = d.s("chan").length() > 0 ? d.s("chan") : d.s("channel");
            src_array.push_back(data_o);
        }
        nlohmann::json req;
        req["srcs"] = src_array;
        req["id"] = it->first;
        jsonma[it->first] = req.dump() + "\r\n\r\n";
    }
    return jsonma;
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
