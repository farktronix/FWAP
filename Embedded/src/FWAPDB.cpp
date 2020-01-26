#include <ESP8266HTTPClient.h>

#include "FWAPDB.h"

FWAPDB::FWAPDB(String host, String dbName) {
  // Hardcoded for now. TODO: Add accessors for all of this
  _db = new Influxdb(host);
  _db->setDb(dbName);
}

void FWAPDB::prepare(InfluxData data) { 
    _db->prepare(data);
}

boolean FWAPDB::write(InfluxData data) {
    return _db->write(data);
}

boolean FWAPDB::write() {
    return _db->write();
}