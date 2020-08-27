#ifndef _FWAP_H
#define _FWAP_H

#define HOSTNAME "FWAPOutside"
#define HOST_LOCATION "Outside"

#define INFLUXDB_HOST "influx.home.rkas.net"

// Reporting Interval (miliseconds)
#define REPORT_INVERVAL 60 * 1000

// WiFi
#define INIT_WIFI 1

// Sensors
#define PMS_ENABLE 1

#define TEMP_SENSOR_ENABLE 1
    #define TEMP_SENSOR_BMP280 0
        #define ONBOARD_BMP_ENABLE 1
        #define DANGLING_BMP_ENABLE 0
    #define TEMP_SENSOR_SI7021 1

#define MOTION_SENSOR_ENABLE 0
#define REED_SWITCH_ENABLE 0

#define LIGHT_SENSOR_ENABLE 1

#endif // _FWAP_H