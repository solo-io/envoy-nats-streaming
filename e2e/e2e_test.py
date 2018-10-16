import ctypes
import ctypes.util
import grequests
import httplib
import logging
import os
import requests
import signal
import subprocess
import tempfile
import time
import unittest

def envoy_preexec_fn():
  PR_SET_PDEATHSIG = 1  # See prtcl(2).
  os.setpgrp()
  libc = ctypes.CDLL(ctypes.util.find_library('c'), use_errno=True)
  libc.prctl(PR_SET_PDEATHSIG, signal.SIGTERM)

DEBUG=True

class ManyRequestsTestCase(unittest.TestCase):
  def setUp(self):
    # A temporary file is used to avoid pipe buffering issues.
    self.cleanup()
    self.stderr = tempfile.NamedTemporaryFile("rw+", delete=True)

  def tearDown(self):
    for p in (self.sub_process, self.nats_server, self.nats_streaming_server):
      if p is not None:
        p.terminate()
    if self.envoy is not None:
      self.envoy.send_signal(signal.SIGINT)
      self.envoy.wait()

    # The file is deleted as soon as it is closed.
    if self.stderr is not None:
      self.stderr.close()
    self.cleanup()

  def cleanup(self):
    self.sub_process = None
    self.nats_server = None
    self.nats_streaming_server = None
    self.envoy = None
    self.stderr = None

  def __create_config(self):
    for create_config_path in ("./create_config.sh", "./e2e/create_config.sh"):
      if os.path.isfile(create_config_path):
        subprocess.check_call(create_config_path)
        break
    else:
      self.fail('"create_config.sh" was not found')

  def __start_nats_server(self):
    if DEBUG:   
      self.nats_server = subprocess.Popen(["gnatsd", "-DV"])
    else:
      self.nats_server = subprocess.Popen("gnatsd")

  def __start_nats_streaming_server(self):
    if DEBUG:
      self.nats_streaming_server = subprocess.Popen(
      ["nats-streaming-server", "-ns", "nats://localhost:4222", "-SDV"])
    else:
      self.nats_streaming_server = subprocess.Popen(["nats-streaming-server", "-ns", "nats://localhost:4222"])

  def __start_envoy(self, prefix = None, suffix = None):
    if prefix is None:
      prefix = []
    if suffix is None:
      suffix = suffix = ["--log-level", "debug"] if DEBUG else []

    envoy = os.environ.get("TEST_ENVOY_BIN","envoy")

    self.envoy = subprocess.Popen(prefix + [envoy, "-c", "./envoy.yaml"]+suffix, preexec_fn=envoy_preexec_fn)
    time.sleep(5)

  def __sub(self):
    self.sub_process = subprocess.Popen(
      ["stan-sub", "-id", "17", "subject1"],
      stderr=self.stderr)
    time.sleep(.1)

  def __make_request(self, payload, expected_status):
    response = requests.post('http://localhost:10000/post', payload)
    self.assertEqual(expected_status, response.status_code)

  def __make_many_requests(self, payloads, expected_status):
    requests = (grequests.post('http://localhost:10000/post', data=p) for p in payloads)
    responses = grequests.map(requests)
    if expected_status:
      for response in responses:
        self.assertEqual(expected_status, response.status_code)

  def __wait_for_response(self, data):
    time.sleep(0.1)
    self.sub_process.terminate()
    self.sub_process = None
    self.stderr.seek(0, 0)
    stderr = self.stderr.read()

    # TODO(talnordan): Validate the entire Protobuf message, including headers.
    self.assertIn('subject:"subject1"', stderr)
    self.assertIn(data, stderr)

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
    self.__start_nats_server()
    self.__start_nats_streaming_server()
    self.__start_envoy()
    self.__sub()

    # Make many requests and assert that they succeed.
    self.__make_request_batches("solopayload %d %d", 3, 1024, 0.1, httplib.OK)
    self.__wait_for_response("solopayload 2 1023")

    # Terminate NATS Streaming to make future requests timeout.
    self.nats_streaming_server.terminate()
    self.nats_streaming_server = None

    # Make many requests and assert that they timeout.
    self.__make_request_batches("solopayload %d %d", 2, 1024, 0.1, httplib.REQUEST_TIMEOUT)

  def test_profile(self):
    report_loc = os.environ.get("TEST_PROF_REPORT","")
    if not report_loc:
      self.skipTest("to enable, set TEST_PROF_REPORT to where you want the report to be saved. " + \
                    "i.e. TEST_PROF_REPORT=report.data")
    print("Starting perf tests; if you have issues you might need to enable perf for normal users:")
    print("'echo -1 | sudo tee  /proc/sys/kernel/perf_event_paranoid'")
    print("'echo  0 | sudo tee  /proc/sys/kernel/kptr_restrict'")
    # Set up environment.
    # See https://github.com/envoyproxy/envoy/blob/e51c8ad0e0526f78c47a7f90807c184a039207d5/tools/envoy_collect/envoy_collect.py#L192
    self.__create_config()
    self.__start_nats_server()
    self.__start_nats_streaming_server()
    self.__start_envoy(["perf", "record", "-g","--"], ["-l","error"])
    self.__sub()
    
    # Make many requests and assert that they succeed.
    self.__make_request_batches("solopayload %d %d", 20, 1024, 0.1, None)
    # The performance tests are slower so we have lower expectations of whats received
    self.__wait_for_response("solopayload 0 500")

    # tear down everything so we can copy the report
    self.tearDown()

    # print the report
    subprocess.check_call(["cp", "perf.data", report_loc])

if __name__ == "__main__":
  global DEBUG
  DEBUG =  True if os.environ.get("DEBUG","") != "0" else False
  if DEBUG:
    logging.basicConfig(level=logging.DEBUG)
  unittest.main()
  