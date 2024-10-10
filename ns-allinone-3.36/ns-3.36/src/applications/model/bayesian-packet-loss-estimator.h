#ifndef BAYESIAN_PACKET_LOSS_ESTIMATOR_H
#define BAYESIAN_PACKET_LOSS_ESTIMATOR_H

#include <algorithm>
#include <math.h>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/seq-ts-header.h"
#include "ns3/traced-callback.h"
#include "ns3/contention-based-flooding-header.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"

using namespace std;

namespace ns3
{
    class BayesianPacketLossEstimator
    {
    private:
        double step = 0.001;
        vector<double> probabilities;
        vector<double> packetLossCandidates;

        // Utilitues
        vector<double> normalize(vector<double> data);
        double sum(vector<double> data);

    public:
        BayesianPacketLossEstimator();
        double getExpectedLoss();
        double getMostLikelyLoss();
        double getLossPctl(double targetPctl);
        void reportObservation(int numSuccess, int numFailures);
    };

} // namespace ns3

#endif /* BAYESIAN_PACKET_LOSS_ESTIMATOR_H */
