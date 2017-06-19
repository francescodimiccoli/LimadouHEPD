#ifndef __LEVREC0FILE__
#define __LEVREC0FILE__ 1

#include "LEvRec0.hh"

#include "TFile.h"
#include "TTree.h"


class LEvRec0File {
public:
   LEvRec0File(const char *inpFile);
   int SetTheEventPointer(LEvRec0 &event);
   int SetTmdPointer(LEvRec0Md &metaData);
   int SetTConfPointer(LEvRec0Conf &dummyPKT);

   // bool GetEntry(int iEntry, LEvRec0 &event); // for future... NO ROOT!
   int GetEntry(int iEntry);
   int GetTmdEntry(int iEntry);
   int GetTConfEntry(int iEntry);
   int GetEntries();
   int GetTmdEntries();
   int GetTConfEntries();
   
   void Close();
   inline bool IsOpen() {return inputCalib->IsOpen();}

   inline bool IsTConf() {return inputCalib->GetListOfKeys()->Contains("TConf");}
   
   inline int GetRunID(int iEntry){return RunId[iEntry];}; 
   int RunIDtoEntry(unsigned short runid);


   
   ~LEvRec0File();
  
private:
   TFile *inputCalib;
    TTree *treeCalib;
   TTree *Tmd;
   TTree *TConf;
   unsigned short *RunId;
};


#endif
