#ifndef __HOUSEKEEPING__
#define __HOUSEKEEPING__ 1

#include "LEvRec0File.hh"
#include "LEvRec0.hh"
#include <TString.h>

#define EASIROC_CH 64
#define EASIROC_HALF_CH 32
#define SILSTRIP 4608
#define TRIG_PAD 6
#define CALO_PL 16
#define LYSO_CU 9
#define VETO_PL 5

using namespace std;

void HVPSMonitorToXML(TString rootname, TString xslPath);
void DUMPConfigToXML(TString rootname, TString xslPath);
void BroadcastToXML(TString rootname, TString xslPath, TString xslPath2);
void CPUTimeTempToXML(TString rootname, TString xslPath);
void HVPSConfigToXML(TString rootname, TString xslPath);
void RunInfoToXML(TString rootname, TString xslPath);
void ScintConfigToXML(TString rootname, TString xslPath);
void SilConfigToXML(TString rootname, TString xslPath);
void TelemetryToXML(TString rootname, TString xslPath);
   
#endif
