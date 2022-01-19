#ifndef CATDBCACHEREDISU_H
#define CATDBCACHEREDISU_H

#include <sw/redis++/redis++.h>

class CaTDBCaRedisU_P;
class CuData;

class CaTDBCaRedisU
{
public:
    CaTDBCaRedisU(const CuData &o);
    ~CaTDBCaRedisU();

    bool update(const std::string& src, const CuData& c);
    std::string error() const;
    bool get(const std::string &src, CuData &out);

private:
    CaTDBCaRedisU_P *d;
};

#endif // CATDBCACHEREDISU_H
