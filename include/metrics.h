#ifndef METRICS_H
#define METRICS_H

#include "prometheus/core.h"
#include "prometheus/counter.h"
#include "prometheus/gauge.h"
#include "prometheus/histogram.h"
#include "prometheus/http_puller.h"

namespace cerberus
{
    class Metrics
    {
        private:
            prometheus::registry_t _registry;
            prometheus::http_server_t _server;

            // The metrics that we want to keep track of.
            
            // Icrement on each parse
            prometheus::counter_metric_t _throughput;
            prometheus::histogram_metric_t _latency;

            // Increment on connect, decrement on disconnect
            prometheus::gauge_metric_t _saturation;

        public:
            // Init all the metrics and label them through the constructor.
            Metrics(std::string&& server_address, std::string&& path) : _server(_registry, std::move(server_address), std::move(path)),
                        _throughput(_registry, "http_requests", "total_requests"),
                        _latency(_registry, "request_duration", "request_latency", {}, {0.01, 0.05, 0.1, 0.5, 1.0}),
                        _saturation(_registry, "active_connections", "open_connections")
            {}

        void countConnection();
    };
}

#endif // !METRICS_H
