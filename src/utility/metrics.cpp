#include "metrics.h"

cerberus::Metrics::Metrics(std::string&& server_address, std::string&& path) : _server(_registry, std::move(server_address), std::move(path)),
            _throughput(_registry, "http_requests", "total_requests"),
            _latency(_registry, "request_duration", "request_latency", {}, {0.01, 0.05, 0.1, 0.5, 1.0}),
            _saturation(_registry, "active_connections", "open_connections")
{}

void cerberus::Metrics::countRequest() 
{
    _throughput += 1;
}
