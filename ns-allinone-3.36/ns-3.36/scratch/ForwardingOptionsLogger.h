#ifndef FORWARDINGLOGGER_H
#define FORWARDINGLOGGER_H

#include <fstream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"

using namespace std;
using namespace ns3;


class ForwardingOptionsLogger {
public:
  ForwardingOptionsLogger ();

  ForwardingOptionsLogger (std::string file);

  //CsvLogger (const CsvLogger&) {};

  ~ForwardingOptionsLogger ();

  void SetFile(std::string file);

  void CreateEntry (string seqNo, uint32_t forwarder, Vector forwarder_pos, uint32_t forwarded_from, Vector forwarded_from_pos, uint32_t option, Vector option_pos);

private:
  std::ofstream outputFile;

  bool createHeader = true;
  bool skip = true;
};

ForwardingOptionsLogger::ForwardingOptionsLogger () : outputFile() {
};

ForwardingOptionsLogger::ForwardingOptionsLogger (std::string file) : outputFile() {
  outputFile.open(file);
};

ForwardingOptionsLogger::~ForwardingOptionsLogger () {
  //outputFile.close();
}

void ForwardingOptionsLogger::SetFile(std::string file) {
  outputFile.open(file);
  skip = false;
}

void ForwardingOptionsLogger::CreateEntry (string seqNo, uint32_t forwarder, Vector forwarder_pos, uint32_t forwarded_from, Vector forwarded_from_pos, uint32_t option, Vector option_pos){

  if(skip) {
    return;
  }
  if (createHeader) {
    createHeader = false;
    outputFile << "timestamp" << ","
               << "seqNo" << ","
               << "forwarder" << ","
               << "forwarder_x" << ","
               << "forwarder_y" << ","
               << "forwarded_from" << ","
               << "forwarded_from_x" << ","
               << "forwarded_from_y" << ","
               << "option" << ","
               << "option_x" << ","
               << "option_y" <<std::endl;
  }
  outputFile << std::to_string(ns3::Simulator::Now().GetSeconds()) << ","
             << seqNo << ","
             << forwarder << ","
             << forwarder_pos.x << ","
             << forwarder_pos.y << ","
             << forwarded_from << ","
             << forwarded_from_pos.x << ","
             << forwarded_from_pos.y << ","
             << option << ","
             << option_pos.x << ","
             << option_pos.y << std::endl;
}

#endif
