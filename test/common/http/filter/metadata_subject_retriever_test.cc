#include <iostream>

#include "envoy/http/metadata_accessor.h"

#include "common/config/nats_streaming_well_known_names.h"
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

// TODO(talnordan): Move this to `mocks/http/filter`.
class TesterMetadataAccessor : public Http::MetadataAccessor {
public:
  virtual Optional<const std::string *> getFunctionName() const {
    if (!function_name_.empty()) {
      return &function_name_;
    }
    return {};
  }

  virtual Optional<const ProtobufWkt::Struct *> getFunctionSpec() const {
    if (function_spec_ != nullptr) {
      return function_spec_;
    }
    return {};
  }

  virtual Optional<const ProtobufWkt::Struct *> getClusterMetadata() const {
    if (cluster_metadata_ != nullptr) {
      return cluster_metadata_;
    }
    return {};
  }

  virtual Optional<const ProtobufWkt::Struct *> getRouteMetadata() const {
    if (route_metadata_ != nullptr) {
      return route_metadata_;
    }
    return {};
  }

  std::string function_name_;
  const ProtobufWkt::Struct *function_spec_;
  const ProtobufWkt::Struct *cluster_metadata_;
  const ProtobufWkt::Struct *route_metadata_;
};

Protobuf::Struct getMetadata(const std::string &json) {
  Protobuf::Struct metadata;
  MessageUtil::loadFromJson(json, metadata);

  return metadata;
}

Optional<Subject>
getSubjectFromMetadata(const Protobuf::Struct &func_metadata,
                       const Protobuf::Struct &cluster_metadata,
                       const Protobuf::Struct &route_metadata) {
  TesterMetadataAccessor testaccessor;
  testaccessor.function_spec_ = &func_metadata;
  testaccessor.cluster_metadata_ = &cluster_metadata;
  testaccessor.route_metadata_ = &route_metadata;

  MetadataSubjectRetriever subjectRetriever;

  return subjectRetriever.getSubject(testaccessor);
}

Optional<Subject> getSubjectFromJson(const std::string &func_json,
                                     const std::string &cluster_json,
                                     const std::string &route_json) {
  auto func_metadata = getMetadata(func_json);
  auto cluster_metadata = getMetadata(cluster_json);
  auto route_metadata = getMetadata(route_json);

  return getSubjectFromMetadata(func_metadata, cluster_metadata,
                                route_metadata);
}

std::string getClusterJson() {
  // TODO(talnordan)
  return empty_json;
}

std::string getFuncJson() {
  // TODO(talnordan)
  return empty_json;
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

} // namespace

TEST(MetadataSubjectRetrieverTest, EmptyJsons) {
  const std::string &func_json = empty_json;
  const std::string &cluster_json = empty_json;
  const std::string &route_json = empty_json;

  auto Subject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_FALSE(Subject.valid());
}

TEST(MetadataSubjectRetrieverTest, EmptyRouteJson) {
  const std::string func_json = getFuncJson();
  const std::string cluster_json = getClusterJson();
  const std::string &route_json = empty_json;

  auto Subject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_FALSE(Subject.valid());
}

TEST(MetadataSubjectRetrieverTest, ConfiguredSubject) {
  Subject configuredSubject{"Subject1"};

  const std::string func_json = getFuncJson();
  const std::string cluster_json = getClusterJson();
  const std::string route_json = getRouteJson(configuredSubject);

  auto actualSubject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_TRUE(actualSubject.valid());
  EXPECT_EQ(actualSubject.value(), configuredSubject);
}

TEST(MetadataSubjectRetrieverTest, MisconfiguredSubjectOppositeJsons) {
  Subject configuredSubject{"Subject1"};

  // The cluster metadata JSON is used as the route metadata, and vice versa.
  const std::string func_json = getFuncJson();
  const std::string cluster_json = getRouteJson(configuredSubject);
  const std::string route_json = getClusterJson();

  auto actualSubject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_FALSE(actualSubject.valid());
}

TEST(MetadataSubjectRetrieverTest, MisconfiguredSubjectNonStringField) {
  const std::string func_json = getFuncJson();
  const std::string cluster_json = getClusterJson();

  // The subject is an integer.
  const std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : 17,
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().SUBJECT);

  auto actualSubject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_FALSE(actualSubject.valid());
}

TEST(MetadataSubjectRetrieverTest, MisconfigureSubjectEmptyField) {
  Subject configuredSubject{"Subject1"};

  const std::string func_json = getFuncJson();
  const std::string cluster_json = getClusterJson();

  // The subject is empty.
  const std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : "",
    }}
    )EOF",
      Config::MetadataNatsStreamingKeys::get().SUBJECT);

  auto actualSubject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_FALSE(actualSubject.valid());
}

TEST(MetadataSubjectRetrieverTest, MisconfiguredSubjectIncorrectFieldName) {
  Subject configuredSubject{"Subject1"};

  const std::string &func_json = getFuncJson();
  const std::string cluster_json = getClusterJson();

  // The subject key is incorrect.
  const std::string route_json = fmt::format(
      R"EOF(
    {{
      "{}" : "{}",
    }}
    )EOF",
      "$ubgekt", configuredSubject);

  auto actualSubject = getSubjectFromJson(func_json, cluster_json, route_json);

  EXPECT_FALSE(actualSubject.valid());
}

} // namespace Envoy
