#include "bayesian-packet-loss-estimator.h"

using namespace std;

namespace ns3
{

    BayesianPacketLossEstimator::BayesianPacketLossEstimator()
    {
        for (double pL = 0; pL <= 1.0; pL += step) {
            probabilities.push_back(1.0);
            packetLossCandidates.push_back(pL);
        }
        probabilities = normalize(probabilities);
    }

    void BayesianPacketLossEstimator::reportObservation(int numSuccess, int numFailures) {
        vector<double> posterior;
        for (int i=0; i< packetLossCandidates.size(); i++) {
            double pL = packetLossCandidates.at(i);
            double likelihood = pow(pL, numFailures) * pow(1-pL, numSuccess);
            posterior.push_back(probabilities.at(i) * likelihood);
        }

        probabilities = normalize(posterior);
    }

    double BayesianPacketLossEstimator::getExpectedLoss() {
        double loss = 0;
        for (int i=0; i< packetLossCandidates.size(); i++) {
            loss += packetLossCandidates.at(i) * probabilities.at(i);
        }

        if (isnan(loss)) {
            return 0.5;
        }

        return loss;
    }

    double BayesianPacketLossEstimator::getLossPctl(double targetPctl) {
        double sum = 0.0;
        for (int i=0; i< packetLossCandidates.size(); i++) {
            sum += probabilities.at(i);
            if (sum >= targetPctl) {
                return packetLossCandidates.at(i);
            }
        }

        return 0.5;
    }

    double BayesianPacketLossEstimator::getMostLikelyLoss() {
        double loss = 0;
        double likelihood = 0;
        for (int i=0; i< packetLossCandidates.size(); i++) {
            if (probabilities.at(i) > likelihood) {
                likelihood = probabilities.at(i);
                loss = packetLossCandidates.at(i);
            }
        }

        return loss;
    }

    double BayesianPacketLossEstimator::sum(vector<double> data) {
        double total = 0;
        for (int i=0; i< data.size(); i++) {
            total += data.at(i);
        }

        return total;
    }

    vector<double> BayesianPacketLossEstimator::normalize(vector<double> data) {
        double total = sum(data);
        vector<double> normalizedData;
        for (int i=0; i< data.size(); i++) {
            normalizedData.push_back(data.at(i) / total);
        }

        return normalizedData;
    }
}
