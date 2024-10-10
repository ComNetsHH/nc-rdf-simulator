#ifndef KpiLogger_H
#define KpiLogger_H

#include <fstream>

#include "ns3/core-module.h"

using namespace std;


class KpiLogger {
public:
  KpiLogger ();

  KpiLogger (std::string file);

  ~KpiLogger ();

  void SetFile(std::string file);

  void CreateEntry (double pd, double pe500);
  void CreateEntry (double pd, double pe500, double sumSent, double sumRcvd, double sumFwd, double sumSentCoded = 0);

private:
  std::ofstream outputFile;

  bool createHeader = true;
};

KpiLogger::KpiLogger () : outputFile() {
};

KpiLogger::KpiLogger (std::string file) : outputFile() {
  outputFile.open(file);
};

KpiLogger::~KpiLogger () {
  //outputFile.close();
}

void KpiLogger::SetFile(std::string file) {
  outputFile.open(file);
}

void KpiLogger::CreateEntry (double pd, double pe500){
  if (createHeader) {
    createHeader = false;
    outputFile << "timestamp" << ","
               << "pd" << ","
               << "pe500" << std::endl;
  }
  outputFile << std::to_string(ns3::Simulator::Now().GetSeconds()) << ","
             << pd << ","
             << pe500 << std::endl;
}

void KpiLogger::CreateEntry (double pd, double pe500, double sumSent, double sumRcvd, double sumFwd, double sumSentCoded){
  if (createHeader) {
    createHeader = false;
    outputFile << "timestamp" << ","
               << "pd" << ","
               << "pe500" << ","
               << "sumSent" << ","
               << "sumSentCoded" << ","
               << "sumRcvd" << ","
               << "sumFwd" << std::endl;
  }
  outputFile << std::to_string(ns3::Simulator::Now().GetSeconds()) << ","
             << pd << ","
             << pe500 << ","
             << sumSent << ","
             << sumSentCoded << ","
             << sumRcvd << ","
             << sumFwd << std::endl;
}

#endif
