#pragma once

#include <string>

#include "topic_retriever.h"
#include "gmock/gmock.h"

namespace Envoy {
namespace Http {

class MockTopicRetriever : public TopicRetriever {
public:
  MockTopicRetriever();
  ~MockTopicRetriever();

  MOCK_METHOD2(getTopic, Optional<Topic>(const RouteEntry &routeEntry,
                                         const ClusterInfo &info));

  Optional<Topic> topic_;
};

} // namespace Http
} // namespace Envoy
