// Local
#include "ecaladc.hh"
#include "detectorID.h"
#include "Edep_Pos.h"
#include "LEvRec0Writer.hh"
#include "trackeradc.hh"
#include "MapEvents.hh"
#include "Edep2Edep_PosConverter.hh"
#include "LEvRec0.hh"

// C++ std
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>

// MCEventAnalyze
#include "RootEvent.hh"
#include "RootTrack.hh"
#include "RootVertex.hh"
#include "RootCaloHit.hh"
#include "RootTrackerHit.hh"


// ROOT libs
#include "TTree.h"
#include "TVector3.h"
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"








std::string  getMCfilename (int argc, char** argv);
std::string  getLvl0filename (const std::string mcfilename);
void LoopOnEvents (LEvRec0Writer* lvl0writer, TTree* Tmc);
std::vector<float> CaloHitsToEdep (std::vector<RootCaloHit>);
void getPMTs (std::vector<RootCaloHit>, ushort * pmt_high, ushort * pmt_low, EcalADC ecaladc);
void getStrips (std::vector<RootTrackerHit>, short* strips);





int main (int argc, char** argv) {
    const std::string mcfilename = getMCfilename (argc, argv);
    const std::string lvl0filename = getLvl0filename (mcfilename);
    TFile* filemc = TFile::Open (mcfilename.c_str(), "READ");
    TTree* Tmc = (TTree*) filemc->Get ("HEPD/EventTree");
    LEvRec0Writer lvl0writer (lvl0filename);
    LoopOnEvents (&lvl0writer, Tmc);
    lvl0writer.Write();
    lvl0writer.Close(); delete Tmc;
    filemc->Close();    delete filemc;
    return 0;
} // main conclusion




void LoopOnEvents (LEvRec0Writer* lvl0writer, TTree* Tmc)
{
    int ne = Tmc->GetEntries();
    RootEvent* MCevt = new RootEvent;
    TBranch* b_Event = new TBranch;
    Tmc->SetBranchAddress ("Event", &MCevt, &b_Event);
    EcalADC ecaladc;
    for (int ie = 0; ie < ne; ie++) {
        int nb = Tmc->GetEntry (ie);
        int eventid =  MCevt->EventID();
        std::vector<RootCaloHit> caloHits =  MCevt->GetCaloHit();
        std::vector<RootTrackerHit>  trackerHits =  MCevt->GetTrackerHit();
        LEvRec0* ev = lvl0writer->pev();
        ev->Reset();
        getPMTs (caloHits, ev->pmt_high, ev->pmt_low, ecaladc);
        getStrips (trackerHits, ev->strip);
        lvl0writer->Fill();
        std::cout << ie << "\r" << std::flush;
    }
    delete b_Event;
    delete MCevt;
    std::cout << "Done     " << std::endl;
}




std::string  getMCfilename (int argc, char** argv)
{
    std::string filename = "../../../Simulation/run/Simulations_root/hepd5000_qmd_173MeV_proton_3C0.root"; // Supposing you run from Tools/MC2Lvl0/build/ ; I know, it's ugly :(
    if (argc > 1) filename = argv[1];
    std::cout << "MC2Lvl0: MC file name set to " << filename << std::endl;
    return filename;
}

std::string  getLvl0filename (const std::string mcfilename)
{

    vector<string> strings;
    istringstream f(mcfilename.c_str());
    string s;
    while (std::getline(f, s, '/')) {
	    strings.push_back(s);
    }

    std::string lvl0filename = ("MC2Lvl0_"+strings[strings.size()-1]).c_str();
    return lvl0filename;
}




void getPMTs (std::vector<RootCaloHit> CaloHits, ushort* pmt_high, ushort* pmt_low, EcalADC ecaladc )
{
    std::vector<Edep_Pos> pmt_info = Calo2Edep_PosConverter (CaloHits);

    ecaladc.SetPositions(pmt_info);
    ecaladc.NormalizePMThg(pmt_high);
    ecaladc.NormalizePMTlg(pmt_low);
    return;
}



void getStrips (std::vector<RootTrackerHit> TrackerHits, short* strips)
{
    std::vector<std::vector<Edep_Pos>> TrackerEdepPos = Tracker2Edep_PosConverter (TrackerHits);
    TrackerADC trkadc (TrackerEdepPos);
    std::vector<short> trkstrips = trkadc.GetStrips();
    for (uint ic = 0; ic < NCHAN; ic++) {
        strips[ic] = trkstrips[ic];
    }
    return;
}

