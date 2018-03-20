import grequests
import logging
import os
import requests
import subprocess
import tempfile
import time
import unittest

class ManyRequestsTestCase(unittest.TestCase):
  def setUp(self):
    # A temporary file is used to avoid pipe buffering issues.
    self.stderr = tempfile.NamedTemporaryFile("rw+", delete=True)

  def tearDown(self):
    # The file is deleted as soon as it is closed.
    self.stderr.close()
    self.stderr = None

  def __create_config(self):
    for create_config_path in ("./create_config.sh", "./e2e/create_config.sh"):
      if os.path.isfile(create_config_path):
        subprocess.check_call(create_config_path)
        break
    else:
      self.fail('"create_config.sh" was not found')

  @staticmethod
  def __start_nats_streaming_server():
    subprocess.Popen("nats-streaming-server")

  @staticmethod
  def __start_verbose_nats_streaming_server():
    subprocess.Popen(["nats-streaming-server", "-SDV", "-DV"])

  @staticmethod
  def __start_envoy():
    subprocess.Popen(["envoy", "-c", "./envoy.yaml", "--log-level", "debug"])
    time.sleep(5)

  def __sub(self):
    p = subprocess.Popen(
      ["stan-sub", "-id", "17", "subject1"],
      stderr=self.stderr)
    return p

  def __make_request(self, payload):
    response = requests.post('http://localhost:10000/post', payload)
    self.assertEqual(200, response.status_code)

  def __make_many_requests(self, payloads):
    requests = (grequests.post('http://localhost:10000/post', data=p) for p in payloads)
    responses = grequests.map(requests)
    for response in responses:
      self.assertEqual(200, response.status_code)

  def __wait_for_response(self, p, data):
    time.sleep(0.1)
    p.terminate()
    self.stderr.seek(0, 0)
    stderr = self.stderr.read()
    expected = 'subject:"subject1" data:"%s"' % data
    self.assertTrue(expected in stderr)

  def test_make_many_requests(self):
    self.__create_config()
    self.__start_verbose_nats_streaming_server()
    self.__start_envoy()
    p = self.__sub()
    for i in xrange(4):
      payloads = [("solopayload %d %d" % (i, j)) for j in xrange(1024)]
      self.__make_many_requests(payloads)
      time.sleep(0.1)
    self.__wait_for_response(p, "solopayload 3 1023")

if __name__ == "__main__":
  logging.basicConfig(level=logging.DEBUG)
  unittest.main()
