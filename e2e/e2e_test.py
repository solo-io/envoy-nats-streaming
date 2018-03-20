import grequests
import logging
import os
import requests
import subprocess
import time
import unittest

class Test(unittest.TestCase):
  def __create_config(self):
    for create_config_path in ("./create_config.sh", "./e2e/create_config.sh"):
      if os.path.isfile(create_config_path):
        subprocess.check_call(create_config_path)
        break
    else:
      self.fail('"create_config.sh" was not found')

  @staticmethod
  def __start_nats_streaming_server():
    subprocess.Popen(["nats-streaming-server", "-SDV", "-DV"])

  @staticmethod
  def __start_envoy():
    subprocess.Popen(["envoy", "-c", "./envoy.yaml", "--log-level", "debug"])
    time.sleep(5)

  @staticmethod
  def __sub_with_durable_name():
    subprocess.call(
      ["timeout", "1", "stan-sub", "-id", "17", "-unsubscribe=false", "-durable=solo", "subject1"])

  def __make_request(self, payload):
    response = requests.post('http://localhost:10000/post', payload)
    self.assertEqual(200, response.status_code)

  def __make_many_requests(self, payloads):
    requests = (grequests.post('http://localhost:10000/post', data=p) for p in payloads)
    responses = grequests.map(requests)
    for response in responses:
      self.assertEqual(200, response.status_code)

  def __wait_for_response(self, data):
    p = subprocess.Popen(
      ["timeout", "1", "stan-sub", "-id", "17", "-durable=solo", "subject1"],
      stderr=subprocess.PIPE)
    p.wait()
    stderr = p.communicate()[1]
    expected = 'subject:"subject1" data:"%s"' % data
    self.assertTrue(expected in stderr)

  def test_make_request(self):
    self.__create_config()
    self.__start_nats_streaming_server()
    self.__start_envoy()
    self.__sub_with_durable_name()
    self.__make_request("solopayload")
    self.__wait_for_response("solopayload")

  def test_make_many_requests(self):
    self.__create_config()
    self.__start_nats_streaming_server()
    self.__start_envoy()
    self.__sub_with_durable_name()
    payloads = [("solopayload %d" % i) for i in xrange(256)]
    self.__make_many_requests(payloads)
    self.__wait_for_response("solopayload 255")

if __name__ == "__main__":
  logging.basicConfig(level=logging.DEBUG)
  unittest.main()
