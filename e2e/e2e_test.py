import grequests
import httplib
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
  def __start_nats_server():
    return subprocess.Popen("gnatsd")

  @staticmethod
  def __start_verbose_nats_server():
    return subprocess.Popen(["gnatsd", "-DV"])

  @staticmethod
  def __start_nats_streaming_server():
    return subprocess.Popen(["nats-streaming-server", "-ns", "nats://localhost:4222"])

  @staticmethod
  def __start_verbose_nats_streaming_server():
    return subprocess.Popen(
      ["nats-streaming-server", "-ns", "nats://localhost:4222", "-SDV"])

  @staticmethod
  def __start_envoy():
    subprocess.Popen(["envoy", "-c", "./envoy.yaml", "--log-level", "debug"])
    time.sleep(5)

  def __sub(self):
    p = subprocess.Popen(
      ["stan-sub", "-id", "17", "subject1"],
      stderr=self.stderr)
    return p

  def __make_request(self, payload, expected_status):
    response = requests.post('http://localhost:10000/post', payload)
    self.assertEqual(expected_status, response.status_code)

  def __make_many_requests(self, payloads, expected_status):
    requests = (grequests.post('http://localhost:10000/post', data=p) for p in payloads)
    responses = grequests.map(requests)
    for response in responses:
      self.assertEqual(expected_status, response.status_code)

  def __wait_for_response(self, stan_pub, data):
    time.sleep(0.1)
    stan_pub.terminate()
    self.stderr.seek(0, 0)
    stderr = self.stderr.read()
    expected = 'subject:"subject1" data:"%s"' % data
    self.assertTrue(expected in stderr)

  def __make_request_batches(self,
                             format_string,
                             batches,
                             requests_in_batch,
                             sleep_interval,
                             expected_status):
    for i in xrange(batches):
      payloads = [(format_string % (i, j)) for j in xrange(requests_in_batch)]
      self.__make_many_requests(payloads, expected_status)
      time.sleep(sleep_interval)

  def test_make_many_requests(self):
    # Set up environment.
    self.__create_config()
    self.__start_verbose_nats_server()
    nats_streaming_server = self.__start_verbose_nats_streaming_server()
    self.__start_envoy()
    stan_pub = self.__sub()

    # Make many requests and assert that they succeed.
    self.__make_request_batches("solopayload %d %d", 3, 1024, 0.1, httplib.OK)
    self.__wait_for_response(nats_streaming_server, "solopayload 2 1023")

    # Terminate NATS Streaming to make future requests timeout.
    nats_streaming_server.terminate()

    # Make many requests and assert that they timeout.
    self.__make_request_batches("solopayload %d %d", 2, 1024, 0.1, httplib.REQUEST_TIMEOUT)

if __name__ == "__main__":
  logging.basicConfig(level=logging.DEBUG)
  unittest.main()
