# coding: utf-8

import os
import re
import string
import hashlib
import json
import logging

from bottle import (
    Bottle,
    run,
    request,
    response,
    error,
    HTTPResponse,
    HTTPError,
    static_file,
)

from collections import namedtuple
from functools import wraps, partial


__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))


ESP_USER_AGENT = "ESP8266-HTTP-UPDATE"

ESP_STATION_MAC = "X-ESP8266-STA-MAC"
ESP_CHIP_SIZE = "X-ESP8266-CHIP-SIZE"
ESP_FREE_SPACE = "X-ESP8266-FREE-SPACE"
ESP_SKETCH_SIZE = "X-ESP8266-SKETCH-SIZE"
ESP_SKETCH_MD5 = "X-ESP8266-SKETCH-MD5"
ESP_SDK_VERSION = "X-ESP8266-SDK-VERSION"
ESP_UPDATE_MODE = "X-ESP8266-MODE"
ESP_UPDATE_VERSION = "X-ESP8266-VERSION"

UPDATE_SKETCH_MD5 = "x-MD5" # 32 character lower case hex string


web = Bottle()


@web.error(403)
def error403(error):
    response.content_type = 'application/json'
    return json.dumps({
        'message': error.body
    })


''' Checks the request for the specified user agent.
    Returns 403 if no match.
'''
def check_user_agent(user_agent):
    def fdecorator(func):
        @wraps(func)
        def fwrapper():
            request_user_agent = (
                request.headers.get('User-Agent', '').split(' ', 1).pop(0)
            )
            if (request_user_agent.upper() == user_agent):
                return func()
            else:
                raise HTTPError(403, "Access denied (User-Agent)")
        return fwrapper
    return fdecorator


''' Checks the request for the existance of the specified request header.
    Returns 403 if not existing.
'''
def check_header(header):
    def fdecorator(func):
        @wraps(func)
        def fwrapper():
            request_header = request.headers.get(header, None)
            if request_header is not None:
                return func()
            else:
                raise HTTPError(403, "Access denied (Header)")
        return fwrapper
    return fdecorator


@web.get('/ota/update')
@check_user_agent(ESP_USER_AGENT)
@check_header(ESP_STATION_MAC)
@check_header(ESP_CHIP_SIZE)
@check_header(ESP_FREE_SPACE)
@check_header(ESP_SKETCH_SIZE)
@check_header(ESP_SKETCH_MD5)
@check_header(ESP_SDK_VERSION)
@check_header(ESP_UPDATE_MODE)
@check_header(ESP_UPDATE_VERSION)
def update():

    Firmware = namedtuple('Firmware', ['file', 'sketch_version'])

    ''' Returns station mac after validation. Raises 404 if invalid.
        Validation:
            Six  groups of two hexadecimal digits separated by colons.
            (e.g. 01:23:45:67:89:AB)
    '''
    def get_esp_station_mac():
        mac = request.headers.get(ESP_STATION_MAC, '').replace(':', '').upper()
        if all(c in string.hexdigits for c in mac) and len(mac) == 12:
            return str(mac)
        else:
            raise HTTPError(400, "Header: station mac")

    ''' Returns sketch version after validation. Raises 404 if invalid.
        Validation:
            Integer value
    '''
    def get_esp_sketch_version():
        mode = request.headers.get(ESP_UPDATE_MODE, '').lower()
        if mode == 'sketch':
            version = request.headers.get(ESP_UPDATE_VERSION, '').lower()
            if all(c in string.digits for c in version) and len(version) > 0:
                return int(version)
            else:
                raise HTTPError(400, "Header: sketch version")


    ''' Returns sketch hash after validation. Raises 404 if invalid.
        Validation:
            Hex value
    '''
    def get_esp_sketch_hash():
        hash = request.headers.get(ESP_SKETCH_MD5, '').lower()
        if all(c in string.hexdigits for c in hash) and len(hash) == 32:
            return str(hash)
        else:
            raise HTTPError(400, "Header: sketch md5")


    ''' Fetches the firmwares available for the specified mac.
    '''
    def fetch_firmwares(mac):
        path = mac

        # guard: path
        if not os.path.exists(path) or not os.path.isdir(path):
            return None

        # fetch files in directory
        files = [f for f in os.listdir(path) if f.endswith('.bin')]
        # guard: files
        if len(files) == 0:
            return None

        # filter firmwares from files
        r = re.compile(r'firmware-sketch-([0-9]+).bin')
        firmwares = [
            Firmware(os.path.join(path, m.string), int(m.group(1)))
                for m in (r.match(f) for f in files) if m
        ]
        # sort firmwares by version
        firmwares = sorted(firmwares, key=lambda x: x.sketch_version)

        return firmwares


    ''' Fetches the firmware with the highest sketch version available for
        the specified mac.
    '''
    def fetch_latest_firmware(mac):
        firmwares = fetch_firmwares(mac)

        # guard: firmwares
        if firmwares is None or len(firmwares) == 0:
            return None

        # return firmware with highest version number
        return firmwares.pop()


    ''' Calculates the sketch hash for the specified firmware.
    '''
    def calculate_sketch_hash(firmware):
        # guard: firmware
        if firmware is None:
            return None
        # guard: firmware file
        if not os.path.exists(firmware.file) or not os.path.isfile(firmware.file):
            return None

        hash = hashlib.new('md5')
        with open(firmware.file, "rb") as f:
            for c in iter(partial(f.read, 512), b''):
                hash.update(c)
            return hash.hexdigest().lower()

        return None


    esp_station_mac = get_esp_station_mac()
    esp_sketch_version = get_esp_sketch_version()

    firmware = fetch_latest_firmware(esp_station_mac)
    # guard: firmware
    if firmware is None:
        logging.warning(
            "Update request from esp station with mac {} not executed (404; no firmware)".format(
                esp_station_mac)
        )
        return HTTPResponse(status=404)

    if firmware.sketch_version > esp_sketch_version:
        # newer firmware available

        esp_sketch_hash = get_esp_sketch_hash()

        sketch_hash = calculate_sketch_hash(firmware)
        # guard: sketch_hash
        if sketch_hash is None:
            return HTTPResponse(status=500)

        if sketch_hash != esp_sketch_hash:
            # different firmware available

            resp = static_file(
                firmware.file,
                __location__,
                mimetype='application/octet-stream',
                download=True
            )
            resp.add_header(UPDATE_SKETCH_MD5, sketch_hash)
            return resp

        else:
            # firmware installed is identical to the selected firmware
            # this can happen if the sketch version supplied by the esp
            # is not correct, return 304 to avoid an endless update cycle
            logging.warning(
                "Update for esp station with mac {} not possible (304; checksum matches; installed <{}>; available <{}>)".format(
                    esp_station_mac, esp_sketch_version, firmware.sketch_version)
            )
            return HTTPResponse(status=304)
    else:
        # firmware installed has the newest sketch version
        logging.info(
            "Update for esp station with mac {} not required (304; installed <{}>; available <{}>)".format(
                esp_station_mac, esp_sketch_version, firmware.sketch_version)
        )
        return HTTPResponse(status=304)

PRODUCTION = False
if __name__ == "__main__":
    import coloredlogs
    if PRODUCTION == False:
        loglevel = logging.DEBUG
        import bottle
        bottle.debug(True)
    else:
        loglevel = logging.WARN
    coloredlogs.install(
        level=loglevel,
        format='%(asctime)s - %(filename)s:%(funcName)s - %(levelname)s - %(message)s',
        isatty=True
    )
    run(web, host='192.168.178.57', port=8080, reloader=(PRODUCTION == False))
