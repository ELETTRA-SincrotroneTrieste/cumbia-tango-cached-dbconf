#ifndef CUTCACHEDCONFFACTORIES_H
#define CUTCACHEDCONFFACTORIES_H

#include <cutangoactionfactories.h>


class CuTCachedReaderConfFactory : public CuTConfFactoryBase
{
public:
    CuTCachedReaderConfFactory(const CuData& op) : m_o(std::move(op)) { }

    CuTangoActionI *create(const std::string &s, CumbiaTango *ct) const;
    CuTangoActionI::Type getType() const;

private:
    CuData m_o;
};

class CuTCachedWriterConfFactory : public CuTConfFactoryBase
{
public:
    CuTCachedWriterConfFactory(const CuData& op) : m_o(op) { }

    CuTangoActionI *create(const std::string &s, CumbiaTango *ct) const;
    CuTangoActionI::Type getType() const;

private:
    CuData m_o;
};

#endif // CUTCACHEDCONFFACTORIES_H
