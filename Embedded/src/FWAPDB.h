#ifndef _FWAPDB_H
#define _FWAPDB_H
#include <InfluxDb.h>

class FWAPDB {
    public:
        FWAPDB(String host, String dbName);

        void prepare(InfluxData data);

        boolean write(InfluxData data);
        boolean write();
    private:
        Influxdb *_db;
};
#endif