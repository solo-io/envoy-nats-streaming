#include <iostream>

#include "common/config/solo_well_known_names.h"
#include "common/protobuf/utility.h"

#include "test/test_common/utility.h"

#include "fmt/format.h"
#include "metadata_topic_retriever.h"

namespace Envoy {

using Http::MetadataTopicRetriever;
using Http::Topic;

namespace {

const std::string empty_json = R"EOF(
    {
    }
    )EOF";

Protobuf::Struct getMetadata(const std::string &json) {
  Protobuf::Struct metadata;
  MessageUtil::loadFromJson(json, metadata);

  return metadata;
}

std::string getRouteJson(const std::string &topic) {
  return fmt::format(
      R"EOF(
    {{
      "{}" : "{}",
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().TOPIC, topic);
}

std::string getClusterJson() {
  // TODO(talnordan)
  return empty_json;
}

Optional<Topic> getTopicFromMetadata(const Protobuf::Struct &route_metadata,
                                     const Protobuf::Struct &cluster_metadata) {
  MetadataTopicRetriever topicRetriever(
      Config::SoloMetadataFilters::get().NATS_STREAMING,
      Config::MetadataNatsStreamingKeys::get().TOPIC);

  return topicRetriever.getTopic(route_metadata.fields(),
                                 cluster_metadata.fields());
}

Optional<Topic> getTopicFromJson(const std::string &route_json,
                                 const std::string &cluster_json) {
  Protobuf::Struct route_metadata = getMetadata(route_json);
  Protobuf::Struct cluster_metadata = getMetadata(cluster_json);
  return getTopicFromMetadata(route_metadata, cluster_metadata);
}

} // namespace

TEST(MetadataTopicRetrieverTest, EmptyJsons) {
  const std::string &route_json = empty_json;
  const std::string &cluster_json = empty_json;

  auto Topic = getTopicFromJson(route_json, cluster_json);

  EXPECT_FALSE(Topic.valid());
}

TEST(MetadataTopicRetrieverTest, EmptyRouteJson) {
  const std::string &route_json = empty_json;
  const std::string cluster_json = getClusterJson();

  auto Topic = getTopicFromJson(route_json, cluster_json);

  EXPECT_FALSE(Topic.valid());
}

TEST(MetadataTopicRetrieverTest, ConfiguredTopic) {
  Topic configuredTopic{"Topic1"};

  std::string route_json = getRouteJson(configuredTopic);
  std::string cluster_json = getClusterJson();

  auto actualTopic = getTopicFromJson(route_json, cluster_json);

  EXPECT_TRUE(actualTopic.valid());
  EXPECT_EQ(actualTopic.value(), configuredTopic);
}

TEST(MetadataTopicRetrieverTest, MisconfiguredTopicOppositeJsons) {
  Topic configuredTopic{"Topic1"};

  // The cluster metadata JSON is used as the route metadata, and vice versa.
  std::string route_json = getClusterJson();
  std::string cluster_json = getRouteJson(configuredTopic);

  auto actualTopic = getTopicFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualTopic.valid());
}

TEST(MetadataTopicRetrieverTest, MisconfiguredTopicNonStringField) {
  // The topic is an integer.
  std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : 17,
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().TOPIC);

  std::string cluster_json = getClusterJson();

  auto actualTopic = getTopicFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualTopic.valid());
}

TEST(MetadataTopicRetrieverTest, MisconfiguredTopicEmptyField) {
  Topic configuredTopic{"Topic1"};

  // The topic is empty.
  std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : "",
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().TOPIC);

  std::string cluster_json = getClusterJson();

  auto actualTopic = getTopicFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualTopic.valid());
}

TEST(MetadataTopicRetrieverTest, MisconfiguredTopicIncorrectFieldName) {
  Topic configuredTopic{"Topic1"};

  // The topic key is incorrect.
  std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : "{}",
    }}
    )EOF",
      "Tupik", configuredTopic);

  std::string cluster_json = getClusterJson();

  auto actualTopic = getTopicFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualTopic.valid());
}

} // namespace Envoy
