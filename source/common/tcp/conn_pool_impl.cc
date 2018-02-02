#include "common/tcp/conn_pool_impl.h"

namespace Envoy {
namespace Tcp {
namespace ConnPool {

ConfigImpl::ConfigImpl(const std::chrono::milliseconds &op_timeout)
    : op_timeout_(op_timeout) {}

} // namespace ConnPool
} // namespace Tcp
} // namespace Envoy
