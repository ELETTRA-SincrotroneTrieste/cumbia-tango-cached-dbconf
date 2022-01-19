#include "cutacached-dbconf.h"
#include "casupjsoniz.h"
#include "casupcurl.h"
#include "cutadbcacheu.h"
#include "catdbcaredisu.h"

#include <map>
#include <set>
#include <culog.h>
#include <cajson-src-bundle-ex.h>
#include <canetmsg.h>
#include <queue>
#include <cutimer.h>
#include <cutimerservice.h>
#include <cumbia.h>
#include <cueventloop.h>
#include <queue>

#define _BUFSIZ 512

class CaTgDbCSrcData {
public:
    CaTgDbCSrcData(const std::string& s) : src(s) {}
    std::string src;
};

class CaTgDbCacheMgrPrivate {
public:
    CaTgDbCacheMgrPrivate(CuLogImplI *l, const CuData& op) :
        log(l), cachedim(1e6L), redisu(op), ltag("ca-tango-db-cache-mgr") {
    }

    long cachedim, sub_rate;
    CuLogImplI *log;
    std::map<std::string, Tango::DeviceProxy *>devmap;
    // std::set is an associative container that contains a __sorted__ set of unique objects
    std::map<std::string, int> evidmap;
    CaTDBCacheU tgu; // tango utils
    CaTDBCaRedisU redisu; // redis utils
    const std::string ltag; // log "tag"

    CuTimerService tmr_s;
    CuTimer* tmr;

    std::queue<CaTgDbCSrcData> srcq;
};

CaTgDbCacheMgr::CaTgDbCacheMgr(Cumbia* cu,CuEventLoopService*loos,
                               const CuData &opts, CuLogImplI *log) {
    d = new CaTgDbCacheMgrPrivate(log, opts);
    if(opts.containsKey("cachedim")) opts["cachedim"].to<long>(d->cachedim);
    if(opts.containsKey("subscribe-rate")) opts["subscribe-rate"].to<long>(d->sub_rate);
    else d->sub_rate = 2; // default 2 per second
    d->tmr = d->tmr_s.registerListener(this, 1000/d->sub_rate, loos);
}

CaTgDbCacheMgr::~CaTgDbCacheMgr() {
    delete d;
}

void CaTgDbCacheMgr::onProgress(int step, int total, const CuData &data) { }

void CaTgDbCacheMgr::onResult(const CuData &data) {
    printf("CaTgDbCacheMgr.onResult: received %s\n", datos(data));

    // from CaReceiver_A activity
    //
    // curl -v http://woody.elettra.eu:9296 -d $'{"srcs":[{"src":"test/device/1/double_scalar"}]}'
    //
    if(data.containsKey("data")) {
        int schedcnt = 0, in_queue = d->srcq.size();
        const std::string& s = data.s("data");
        CaNetMsg nm(s);
        printf("ca-tango-db-cache-mgr.onResult: received \e[1;36m%s\e[0m\n", s.c_str());
        CaJsonSrcBundleExtract bux;
        std::list<CuData> dl;
        std::string chan, id, global_m;
        bux.extract(nm.payload, dl, &chan, &id);
        if(bux.error_msg.length() > 0) {
            d->log->write(d->ltag, "Json error: " + bux.error_msg + " | msg payload " + s, CuLog::LevelError);
        }
        else {
            for(const CuData& da : dl) {
                if(da.containsKey("src")) {
                    const std::string& src = da.s("src");
                    if(d->evidmap.find(src) != d->evidmap.end()) {
                        d->log->write(d->ltag, src + " already monitored: event id " + std::to_string(d->evidmap[src]), CuLog::LevelInfo);
                    } else {
                        d->srcq.emplace(CaTgDbCSrcData(src));
                        schedcnt++;
                    }
                }
            }
            if(d->srcq.size() > 0)
                d->tmr_s.restart(d->tmr, 1000/d->sub_rate);
            for(const CuData& da : dl)
                printf("ca-tango-db-cache-mgr.onResult: scheduled %s\n", datos(da));

        }
        int fd = data.I("fd");
        nm.setStatus(200).setPayload(CaSupJsoniz().make_scheduled_subscribe("ok", schedcnt, in_queue, d->srcq.size()));
        int byw = m_sock_write(fd, nm.raw());
        printf("[0x%lx] \e[1;32mCaSupervisor::onResult \e[0;35m %d bytes written in reply \e[0m\n", pthread_self(),  byw);
    }
    // connection / disconnection from ca-proxy
    if(data.containsKey("fd") && data.containsKey("fdopen")) {
        bool o =  data.B("fdopen");
        const int fd = data.I("fd");
        d->log->write(d->ltag, "peer " + std::string(o ? "" : "dis") + "connected: sofd " + std::to_string(fd), CuLog::LevelInfo);
        if(!o)
            if(close(fd) < 0) d->log->write(d->ltag, "error closing peer socket: " + std::string(strerror(errno)));
    }
}

void CaTgDbCacheMgr::onResult(const std::vector<CuData> &srcs) {

}

CuData CaTgDbCacheMgr::getToken() const {
    return CuData("type", "ca-tango-db-cache-mgr");
}

void CaTgDbCacheMgr::push_event(Tango::AttrConfEventData *ed) {
    std::string dnam = d->tgu.m_dev_get_name(ed->device);
    if(dnam.length() == 0)
        d->log->write("ca-tg-db-cache-mgr", "push_event: dev->name failed: " + d->tgu.error);
    CuData co;
    d->tgu.m_fill_from_attconf(ed->attr_conf, co);
    d->log->write(d->ltag, "configuration changed for " + ed->attr_name + ": " + datos(co), CuLog::LevelInfo );
    bool redisok = d->redisu.update(ed->attr_name, co);
    if(!redisok)
        d->log->write(d->ltag, "error updating conf data on redis: " + d->redisu.error());
    else {
        CuData out;
        d->redisu.get(ed->attr_name, out);
        printf("CaTgDbCacheMgr::push_event read just updated data for \e[1;36m%s\e[0m : \e[1;32m%s\e[0m\n",
               ed->attr_name.c_str(), datos(out));
    }
}

int CaTgDbCacheMgr::m_sock_write(int sofd, const std::string &buf) {
    int totclibw = 0, clibw = 0, wlen = buf.length();
    // write rbuf back to client fd
    while(totclibw < wlen && clibw >= 0) {
        clibw = send(sofd, buf.c_str() + totclibw, wlen - totclibw < _BUFSIZ ? wlen - totclibw : _BUFSIZ, MSG_NOSIGNAL);
        if(clibw < 0) {
            d->log->write(d->ltag, "write failed :" + std::string(strerror(errno)));
        }
        totclibw += clibw;
    }
    return clibw >=0 ? totclibw : clibw;
}

int CaTgDbCacheMgr::m_monitor(const std::string &src) {
    int evid = -1;
    // src length, !contains("->") and count('/') already checked by onTimeout
    // src not already monitored (checked within onResult)
    Tango::DeviceProxy *dev = nullptr;
    std::map<std::string, Tango::DeviceProxy *>::const_iterator it = d->devmap.find(src);
    size_t lastsep = src.rfind("/");
    const std::string &devnam = src.substr(0, lastsep);
    const std::string &attnam = src.substr(lastsep + 1);

    if(it != d->devmap.end())
        dev = it->second;
    else {
        printf("CaTgDbCacheMgr: got a new srcs dev \"%s\" attr \"%s\"\n", devnam.c_str(), attnam.c_str());
        dev = d->tgu.m_get_dev(devnam);
        if(dev != nullptr)
            d->devmap[devnam] = dev;
    }

    evid = d->tgu.m_att_conf_change_subscribe(dev, devnam, attnam, this);
    if(evid > -1) {
        d->evidmap[src] = evid;
        d->log->write(d->ltag,
                      src + " successfully subscribed to att_conf_event: id "
                      + std::to_string(evid) +
                      "[" + std::to_string(d->evidmap.size()) + "/" +
                      std::to_string(d->cachedim) + "]", CuLog::LevelInfo);
    } else {
        d->log->write(d->ltag, "error subscribing " + src + ": " + d->tgu.error);
    }

    return evid;
}

void CaTgDbCacheMgr::onTimeout(CuTimer *t) {
    bool ok = false;
    // dequeue one src only, skipping invalid srcs
    while(d->srcq.size() > 0 && !ok) {
        const CaTgDbCSrcData& sd = d->srcq.front();
        const std::string& src = sd.src;
        ok = src.length() > 0 && std::count(src.begin(), src.end(), '/') == 3
                && src.find("->") == std::string::npos;
        if(!ok)
            d->log->write(d->ltag, "invalid source " + src);
        else
            m_monitor(src);
        d->srcq.pop();
    }
    if(d->srcq.size() > 0)
        d->tmr_s.restart(t, 1000/d->sub_rate);
}
