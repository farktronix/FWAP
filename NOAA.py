#!/usr/local/bin/python3

import requests
import logging

class NOAA:
    NOAA_BASE_URL="https://api.weather.gov"

    def __init__(self, latLon=None):
        self._latLon = latLon
        self._stationID = None

    def debugLog(self, msg):
        logging.debug(msg)

    def doRequest(self, url):
        self.debugLog("📜  Sending request to %s" % url)
        req = requests.get(url)
        if req.status_code != 200:
            raise  Exception("Error: {} {}".format(req.status_code, req.reason))
        return req.json()

    @property
    def latLon(self):
        if self._latLon is None:
            self.debugLog("🌎  Fetching lat/long from geoapi...")
            req = self.doRequest("https://freegeoip.app/json/")
            self._latLon = [req["latitude"], req["longitude"]]

        return self._latLon

    @property
    def latLonString(self):
        return ",".join(map(str,self.latLon))


    @property
    def nearestStationID(self):
        if self._stationID is None:
            stations = self.getStations()
            if stations is not None and len(stations) > 0:
                station = stations[0]
                stationID = station['properties']['stationIdentifier']
                self.debugLog("📡  Nearest station is %s" % stationID)
                self._stationID = stationID
        return self._stationID



    """ Expects latitude and longitude as a comma separated string
    """
    def getStations(self):
        requestURL = self.NOAA_BASE_URL+"/points/"+self.latLonString+"/stations"
        self.debugLog("🌨  Fetching stations from NOAA...")
        stationsRequest = self.doRequest(requestURL)
        return stationsRequest["features"]

    def getNearestLatestObservation(self):
        requestURL = self.NOAA_BASE_URL+"/stations/{}/observations/latest".format(self.nearestStationID)
        observation = self.doRequest(requestURL)
        return observation

    def getCurrentSeaLevelPressure(self):
        obs = self.getNearestLatestObservation()
        pressure = obs['properties']['seaLevelPressure']['value']
        units = obs['properties']['seaLevelPressure']['unitCode']
        if units == "unit:Pa":
            return pressure
        else:
            raise Exception("Error: Unknown pressure units (%s)" % units)
