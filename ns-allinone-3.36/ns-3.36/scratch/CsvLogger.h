#ifndef CSVLOGGER_H
#define CSVLOGGER_H

#include <fstream>

#include "ns3/core-module.h"

using namespace std;


class CsvLogger {
public:
  CsvLogger ();

  CsvLogger (std::string file);

  //CsvLogger (const CsvLogger&) {};

  ~CsvLogger ();

  void SetFile(std::string file);

  void CreateEntry (uint32_t node_id, string seqNo, string eventType, string src, string lastHop, string delay, string numHops, string pL = "-1.0");

  void CreateCourse(uint32_t node_id, ns3::Ptr<const ns3::MobilityModel> mobility);

private:
  std::ofstream outputFile;

  bool createHeader = true;
};

CsvLogger::CsvLogger () : outputFile() {
};

CsvLogger::CsvLogger (std::string file) : outputFile() {
  outputFile.open(file);
};

CsvLogger::~CsvLogger () {
  //outputFile.close();
}

void CsvLogger::SetFile(std::string file) {
  outputFile.open(file);
}

void CsvLogger::CreateEntry (uint32_t node_id, string seqNo, string eventType, string src, string lastHop, string delay, string numHops, string pL){
  if (createHeader) {
    createHeader = false;
    outputFile << "timestamp" << ","
               << "nodeId" << ","
               << "seqNo" << ","
               << "eventType" << ","
               << "src" << ","
               << "lastHop" << ","
               << "delay" <<  ","
               << "numHops" << ","
               << "pL" <<std::endl;
  }
  outputFile << std::to_string(ns3::Simulator::Now().GetSeconds()) << ","
             << node_id << ","
             << seqNo << ","
             << eventType << ","
             << src << ","
             << lastHop << ","
             << delay << ","
             << numHops << ","
             << pL << std::endl;
}

void CsvLogger::CreateCourse(uint32_t node_id, ns3::Ptr<const ns3::MobilityModel> mobility) {
  if (createHeader) {
    createHeader = false;
    outputFile << "timestamp" << ","
               << "nodeId" << ","
               << "pos_x" << ","
               << "pos_y" << ","
               << "pos_z" << std::endl;
  }

  ns3::Vector pos = mobility->GetPosition ();
  outputFile << ns3::Simulator::Now().GetSeconds() << ","
             << node_id << ","
             << pos.x << ","
             << pos.y << ","
             << pos.z << std::endl;
}


#endif
