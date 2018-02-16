#include <iostream>

#include "common/config/solo_well_known_names.h"
#include "common/http/filter/metadata_subject_retriever.h"
#include "common/protobuf/utility.h"

#include "test/test_common/utility.h"

#include "fmt/format.h"

namespace Envoy {

using Http::MetadataSubjectRetriever;
using Http::Subject;

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

std::string getRouteJson(const std::string &subject) {
  return fmt::format(
      R"EOF(
    {{
      "{}" : "{}",
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().SUBJECT, subject);
}

std::string getClusterJson() {
  // TODO(talnordan)
  return empty_json;
}

Optional<Subject>
getSubjectFromMetadata(const Protobuf::Struct &route_metadata,
                       const Protobuf::Struct &cluster_metadata) {
  MetadataSubjectRetriever subjectRetriever(
      Config::SoloMetadataFilters::get().NATS_STREAMING,
      Config::MetadataNatsStreamingKeys::get().SUBJECT);

  return subjectRetriever.getSubject(route_metadata.fields(),
                                     cluster_metadata.fields());
}

Optional<Subject> getSubjectFromJson(const std::string &route_json,
                                     const std::string &cluster_json) {
  Protobuf::Struct route_metadata = getMetadata(route_json);
  Protobuf::Struct cluster_metadata = getMetadata(cluster_json);
  return getSubjectFromMetadata(route_metadata, cluster_metadata);
}

} // namespace

TEST(MetadataSubjectRetrieverTest, EmptyJsons) {
  const std::string &route_json = empty_json;
  const std::string &cluster_json = empty_json;

  auto Subject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_FALSE(Subject.valid());
}

TEST(MetadataSubjectRetrieverTest, EmptyRouteJson) {
  const std::string &route_json = empty_json;
  const std::string cluster_json = getClusterJson();

  auto Subject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_FALSE(Subject.valid());
}

TEST(MetadataSubjectRetrieverTest, ConfiguredSubject) {
  Subject configuredSubject{"Subject1"};

  std::string route_json = getRouteJson(configuredSubject);
  std::string cluster_json = getClusterJson();

  auto actualSubject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_TRUE(actualSubject.valid());
  EXPECT_EQ(actualSubject.value(), configuredSubject);
}

TEST(MetadataSubjectRetrieverTest, MisconfiguredSubjectOppositeJsons) {
  Subject configuredSubject{"Subject1"};

  // The cluster metadata JSON is used as the route metadata, and vice versa.
  std::string route_json = getClusterJson();
  std::string cluster_json = getRouteJson(configuredSubject);

  auto actualSubject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualSubject.valid());
}

TEST(MetadataSubjectRetrieverTest, MisconfiguredSubjectNonStringField) {
  // The subject is an integer.
  std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : 17,
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().SUBJECT);

  std::string cluster_json = getClusterJson();

  auto actualSubject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualSubject.valid());
}

TEST(MetadataSubjectRetrieverTest, MisconfigureSubjectEmptyField) {
  Subject configuredSubject{"Subject1"};

  // The subject is empty.
  std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : "",
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().SUBJECT);

  std::string cluster_json = getClusterJson();

  auto actualSubject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualSubject.valid());
}

TEST(MetadataSubjectRetrieverTest, MisconfiguredSubjectIncorrectFieldName) {
  Subject configuredSubject{"Subject1"};

  // The subject key is incorrect.
  std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : "{}",
    }}
    )EOF",
      "$ubgekt", configuredSubject);

  std::string cluster_json = getClusterJson();

  auto actualSubject = getSubjectFromJson(route_json, cluster_json);

  EXPECT_FALSE(actualSubject.valid());
}

} // namespace Envoy
