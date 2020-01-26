#include <ESP8266HTTPClient.h>

#include "FWAPDB.h"
#include "FWAP.h"

FWAPDB::FWAPDB(String host, String dbName) {
  // Hardcoded for now. TODO: Add accessors for all of this
  _db = new Influxdb(host);
  _db->setDb(dbName);
}

void FWAPDB::prepare(InfluxData data) { 
    data.addTag("host", HOSTNAME);
    data.addTag("location", HOST_LOCATION);
    _db->prepare(data);
}

boolean FWAPDB::write(InfluxData data) {
    data.addTag("host", HOSTNAME);
    data.addTag("location", HOST_LOCATION);
    return _db->write(data);
}

boolean FWAPDB::write() {
    return _db->write();
}