#ifndef CASUPJSONIZ_H
#define CASUPJSONIZ_H

#include <string>
#include <map>
#include <vector>
#include <list>

class CuData;

// Jsonizer

class CaSupJsoniz
{
public:
	CaSupJsoniz();
    std::map<std::string, std::string> jsonize(const std::vector<CuData>&dl,
                                               std::map<std::string, std::list<CuData> > &clidmap) const;
    void extract(const std::string& json, std::list<CuData>& dl) const;
    const std::string make_msg_ok() const;
    const std::string make_err_msg(const std::string& msg) const;
    bool is_err_msg(const std::string& json, std::string& errmsg) const;
    const std::string make_scheduled_subscribe(const std::string& msg, int scheduled_cnt, int queuepos, int queuetot) const;
};

#endif // CASUPJSONIZ_H
