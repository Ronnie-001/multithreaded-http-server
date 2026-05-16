#ifndef METRICS_H
#define METRICS_H

#include "prometheus/prometheus.h"

namespace cerberus
{
    class Metrics
    {
        private:
            prometheus::registry_t _registry;
            prometheus::http_server_t _server;

            // The metrics that we want to keep track of.
            
            // Increment on each parse
            prometheus::counter_metric_t _throughput;
            prometheus::histogram_metric_t _latency;

            // Increment on connect, decrement on disconnect
            prometheus::gauge_metric_t _saturation;

        public:
            // Init all the metrics and label them through the constructor.
            Metrics(std::string&& server_address, std::string&& path);

            void countConnection();
    };
}

#endif // !METRICS_H
