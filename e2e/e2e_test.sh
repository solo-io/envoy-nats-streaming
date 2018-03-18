#!/bin/bash
set -e

./create_config.sh || ./e2e/create_config.sh

# Binaries we use:
ENVOY=${ENVOY:-envoy}
NATS_SERVER=${NATS_SERVER:-nats-streaming-server}
STAN_SUB=${STAN_SUB:-stan-sub}
STAN_PUB=${STAN_PUB:-stan-pub}
DURABLE=solo

# Test code:
echo "Starting NATS Streaming Server"
$NATS_SERVER -SDV -DV &

$ENVOY -c ./envoy.yaml --log-level debug &
sleep 5

# Register durable name, and don't fail
timeout 1 $STAN_SUB -id 17 -unsubscribe=false -durable=$DURABLE subject1 | :

# TODO(talnordan): Remove this.
# $STAN_PUB subject1 $PAYLOAD

for i in `seq 1 100`;
do
  curl -v localhost:10000/post --data '"'solopayload$i'"' -H"content-type: application/json" 2>&1 | grep "200 OK"
done

echo "Waiting for response"

# Test that we got the mssage
[[ -n $(timeout 1 $STAN_SUB -id 17 -durable=$DURABLE subject1 2>&1 |grep solopayload100) ]]

echo PASS
