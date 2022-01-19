# cumbia-tango-cached-dbconf

Client library to access a Redis database to try to fetch the Tango Attribute configuration instead of the Tango Database.
If the configuration of a given source is not found, then Tango *get attribute config* shall be used and a message to the
service [ca-tango-db-cache-mgr](https://gitlab.elettra.eu/puma/server/ca-tango-db-cache-mgr) shall be sent in order to save
the configuration in the cache later


## Building

> git clone https://github.com/ELETTRA-SincrotroneTrieste/cumbia-tango-cached-dbconf

> cd cumbia-tango-cached-dbconf

Check dependencies and PKG_CONFIG_PATH. In particular, [redis-plus-plus](https://github.com/sewenew/redis-plus-plus) is required

> export PKG_CONFIG_PATH=/usr/local/redis-plus-plus/lib/pkgconfig:$PKG_CONFIG_PATH

> meson builddir

> cd builddir

> meson configure -Dprefix=/usr/local/cumbia-tango-cached-dbconf

> meson configure -Dlibdir=lib

buildtype (debug|release)

> meson configure -Dbuildtype=debug

> ninja && ninja install
