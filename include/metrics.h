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
            prometheus::counter_metric_t _throughput;
            prometheus::histogram_metric_t _latency;
            prometheus::gauge_metric_t _saturation;

        public:
            Metrics();

            void countConnection();
    };
}

#endif // !METRICS_H
#define METRICS_H
