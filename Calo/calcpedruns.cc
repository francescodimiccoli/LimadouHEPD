#include "LEvRec0File.hh"
#include "LCaloCalManager.hh"          	
#include <iostream>
#include <fstream>
#include <iomanip> 

int main(int argc,char *argv[]){

// tail correction factors
// these are the sum of the theory correction and some fine tuning
double fac3h=(1./9.865784e-01)+0.01;
double fac3l=(1./9.865784e-01)+0.02;

// out fil
std::ofstream out1("pedcalc.HG.txt");
std::ofstream out2("pedcalc.LG.txt");

// getting pedestals with LCaloManager class

 double meanc0[NPMT]; double rmsc0[NPMT]; double meanc1[NPMT]; double rmsc1[NPMT];

 double corr[2]={0}, corr1[2]={0};
 int outcounts[2];

 // getting the file


 LCaloCalManager::GetInstance().LoadRun(argv[1]);
 
 // HG 
 for (int pmtu=0; pmtu<NPMT; pmtu++){
 
 double hgseed = LCaloCalManager::GetInstance().GetPeak(pmtu);

 LCaloCalManager::GetInstance().PMTsWindowedRmsHG(pmtu,hgseed,5., corr, outcounts);
 corr[1]=corr[1]*fac3h;
 
 LCaloCalManager::GetInstance().PMTsWindowedRmsHG(pmtu,corr[0],corr[1], corr, outcounts);
 corr[1]=corr[1]*fac3h;

 

 meanc0[pmtu]=corr[0];
 rmsc0[pmtu]=corr[1]; 
} 
//--------------------------------------------------------------------

// LG with  

for (int pmtu=0; pmtu<NPMT; pmtu++){
 
 double lgseed = LCaloCalManager::GetInstance().GetPeakLG(pmtu);

 LCaloCalManager::GetInstance().PMTsWindowedRmsLG(pmtu,lgseed,5., corr1, outcounts);
 corr1[1]=corr1[1]*fac3l;
 
 LCaloCalManager::GetInstance().PMTsWindowedRmsLG(pmtu,corr1[0],corr1[1], corr1, outcounts);
 corr1[1]=corr1[1]*fac3l;

 

 meanc1[pmtu]=corr1[0];
 rmsc1[pmtu]=corr1[1]; 
 }

 //output

 for (int loop=0; loop<NPMT; loop++){
  out1 << loop <<" " <<  std::setprecision(4) << meanc0[loop]  << " " << rmsc0[loop]<< std::endl;
  out2 << loop <<" " <<  std::setprecision(4) << meanc1[loop]  << " " << rmsc1[loop]<< std::endl;
}

out1.close();
out2.close();

return 0;}
