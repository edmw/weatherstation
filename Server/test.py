import os

import unittest
import unittest.mock
import webtest

import ota


app = webtest.TestApp(ota.web)


class TestServer(unittest.TestCase):

    def test_root(self):
        resp = app.get('/', expect_errors=True)
        assert resp.status_int == 404


class TestOTAUpdate(unittest.TestCase):

    def setUp(self):
        test_dir = ota.__location__
        test_mac = '12345678ABCD'
        test_firmwares = [
            'firmware-sketch-6.bin',
            'firmware-sketch-4.txt',
            'firmware-sketch-2.bin',
            'firmware-sketch-1.bin',
            'firmware-sketch-a.bin',
        ]
        test_firmware_name = test_firmwares[0]
        test_firmware_content = b'sketch'

        # patch os functions to mock firmware files

        patcher = unittest.mock.patch('os.listdir')
        self.addCleanup(patcher.stop)
        mock_listdir = patcher.start()
        mock_listdir.return_value = test_firmwares
        patcher = unittest.mock.patch('os.path.exists')
        self.addCleanup(patcher.stop)
        self.mock_path_exists = patcher.start()
        self.mock_path_exists.side_effect = (
            lambda x:
                x == test_mac or
                x in test_firmwares or
                x in [os.path.join(test_mac, f) for f in test_firmwares] or
                x in [os.path.join(test_dir, test_mac, f) for f in test_firmwares]
        )
        patcher = unittest.mock.patch('os.path.isdir')
        self.addCleanup(patcher.stop)
        self.mock_isdir = patcher.start()
        self.mock_isdir.side_effect = (
            lambda x:
                x == test_mac or
                x == os.path.join(test_dir, test_mac)
        )
        patcher = unittest.mock.patch('os.path.isfile')
        self.addCleanup(patcher.stop)
        self.mock_isfile = patcher.start()
        self.mock_isfile.side_effect = (
            lambda x:
                x in test_firmwares or
                x in [os.path.join(test_mac, f) for f in test_firmwares] or
                x in [os.path.join(test_dir, test_mac, f) for f in test_firmwares]
        )
        patcher = unittest.mock.patch('os.access')
        self.addCleanup(patcher.stop)
        self.mock_access = patcher.start()
        self.mock_access.return_value = True
        m = unittest.mock.mock_open(read_data=test_firmware_content)
        patcher = unittest.mock.patch('builtins.open', m)
        self.addCleanup(patcher.stop)
        self.mock_open = patcher.start()

        # patch bottle to mock static file

        patcher = unittest.mock.patch('ota.static_file')
        self.addCleanup(patcher.stop)
        mock_static_file = patcher.start()
        headers = dict()
        headers['Content-Type'] = 'application/octet-stream'
        headers['Content-Disposition'] = 'attachment; filename="{}"'.format(test_firmware_name)
        headers['Content-Length'] = len(test_firmware_content)
        mock_static_file.return_value = ota.HTTPResponse(test_firmware_content, **headers)

    def __test_headers(self,
        useragent=ota.ESP_USER_AGENT,
        mac='12:34:56:78:AB:CD',
        sketch_size=123,
        sketch_md5='11111111111111111111111111111111',
        version=1
    ):
        headers = [
            (ota.ESP_STATION_MAC, str(mac)),
            (ota.ESP_CHIP_SIZE, '123'),
            (ota.ESP_FREE_SPACE, '123'),
            (ota.ESP_SKETCH_SIZE, str(sketch_size)),
            (ota.ESP_SKETCH_MD5, str(sketch_md5)),
            (ota.ESP_SDK_VERSION, '123'),
            (ota.ESP_UPDATE_MODE, 'sketch'),
            (ota.ESP_UPDATE_VERSION, str(version)),
        ]
        if useragent is not None:
            headers.append(('User-Agent', str(useragent)))
        return headers

    def __assert_error(self, resp, message):
        assert resp.content_type == 'application/json'
        assert resp.json['message'] == message

    def test_missing_headers(self):
        resp = app.get('/ota/update', expect_errors=True)
        assert resp.status == '403 Forbidden'
        self.__assert_error(resp, 'Access denied (User-Agent)')
        headers = dict()
        headers['User-Agent'] = ota.ESP_USER_AGENT
        resp = app.get('/ota/update', headers=headers, expect_errors=True)
        assert resp.status == '403 Forbidden'
        self.__assert_error(resp, 'Access denied (Header)')

    def test_wrong_useragent(self):
        headers = self.__test_headers(useragent=None)
        resp = app.get('/ota/update', headers=headers, expect_errors=True)
        assert resp.status == '403 Forbidden'
        self.__assert_error(resp, 'Access denied (User-Agent)')
        headers = self.__test_headers(useragent='UA')
        resp = app.get('/ota/update', headers=headers, expect_errors=True)
        assert resp.status == '403 Forbidden'
        self.__assert_error(resp, 'Access denied (User-Agent)')

    def test_unknown_mac(self):
        headers = self.__test_headers(mac='12:12:12:12:12:12')
        resp = app.get('/ota/update', headers=headers, expect_errors=True)
        assert resp.status == '404 Not Found'

    def test_update(self):
        resp = app.get('/ota/update', headers=self.__test_headers(), expect_errors=True)
        assert resp.status == '200 OK'
        assert resp.content_type == 'application/octet-stream'
        assert resp.headers['Content-Disposition'] == 'attachment; filename="firmware-sketch-6.bin"'
        assert resp.headers['Content-Length'] == '6'
        assert resp.headers[ota.UPDATE_SKETCH_MD5] == '834feae744c43369c32b2cdbf2ada1e6'

    def test_update_no_update(self):
        headers = self.__test_headers(version=6)
        resp = app.get('/ota/update', headers=headers)
        assert resp.status == '304 Not Modified'

    def test_update_no_update_wrong_version(self):
        headers = self.__test_headers(version=1, sketch_md5='834feae744c43369c32b2cdbf2ada1e6')
        resp = app.get('/ota/update', headers=headers)
        assert resp.status == '304 Not Modified'


if __name__ == '__main__':
    unittest.main()
