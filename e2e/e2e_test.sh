#!/bin/bash
set -e

./create_config.sh || ./e2e/create_config.sh

# Binaries we use:
ENVOY=${ENVOY:-envoy}
NATS_SERVER=${NATS_SERVER:-nats-streaming-server}
STAN_SUB=${STAN_SUB:-stan-sub}
STAN_PUB=${STAN_PUB:-stan-pub}
DURABLE=solo
PAYLOAD=solopayload

# Test code:
echo "Starting NATS Streaming Server"
$NATS_SERVER &

$ENVOY -c ./envoy.yaml --log-level debug &
sleep 5

# Register durable name, and don't fail
timeout 1 $STAN_SUB -id 17 -unsubscribe=false -durable=$DURABLE subject1 | :

# TODO(talnordan): Remove this.
$STAN_PUB subject1 $PAYLOAD

curl -v localhost:10000/post --data '"'$PAYLOAD'"' -H"content-type: application/json" 2>&1 | grep "200 OK"

echo "Waiting for response"

# Test that we got the mssage
[[ -n $(timeout 1 $STAN_SUB -id 17 -durable=$DURABLE subject1 2>&1 |grep $PAYLOAD) ]]

echo PASS
