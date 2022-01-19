#ifndef CASUPERVISOR_H
#define CASUPERVISOR_H

#include <vector>
#include <cuthreadlistener.h>
#include <tango.h>
#include <cudata.h>
#include <cutimerlistener.h>

class CuLogImplI;
class CaSupDbActivity;
class Cumbia;
class CuEventLoopService;
class CaTgDbCacheMgrPrivate;

class CaTgDbCacheMgr : public CuThreadListener, public Tango::CallBack, public CuTimerListener
{
public:
    CaTgDbCacheMgr(Cumbia *cu, CuEventLoopService *loos, const CuData& opts, CuLogImplI *log);
    virtual ~CaTgDbCacheMgr();

    // CuThreadListener interface
public:
    void onProgress(int step, int total, const CuData &data);
    void onResult(const CuData &data);
    void onResult(const std::vector<CuData> &srcs);
    CuData getToken() const;

private:

    CaTgDbCacheMgrPrivate *d;
    int m_sock_write(int sofd, const std::string &buf);

    int m_monitor(const string &src);

public:   // CallBack interface
    void push_event(Tango::AttrConfEventData *ed);


    // CuTimerListener interface
public:
    void onTimeout(CuTimer *t);
};

#endif // CASUPERVISOR_H
