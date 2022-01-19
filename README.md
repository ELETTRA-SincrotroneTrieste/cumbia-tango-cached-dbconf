# cumbia-tango-cached-dbconf

Client library to access a Redis database to try to fetch the Tango Attribute configuration instead of the Tango Database.
If the configuration of a given source is not found, then Tango *get attribute config* shall be used and a message to the
service [ca-tango-db-cache-mgr](https://gitlab.elettra.eu/puma/server/ca-tango-db-cache-mgr) shall be sent in order to save
the configuration in the cache later



