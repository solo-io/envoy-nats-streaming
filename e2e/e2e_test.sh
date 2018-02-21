#!/bin/bash
#

set -e

./create_config.sh || ./e2e/create_config.sh

ENVOY=${ENVOY:-envoy}

$ENVOY -c ./envoy.yaml --log-level debug &
sleep 5

curl -v localhost:10000/post --data '"abc"' -H"content-type: application/json" 2>&1 | grep "500 Internal Server Error"

echo PASS
