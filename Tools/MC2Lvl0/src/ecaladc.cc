/*
 * ecaladc.cc
 *
 *
 *
 */


#include "ecaladc.hh"
#include "TVector3.h"
#include "TVector2.h"
#include <iostream>




float Vector3Dist (TVector3 v1, TVector3 v2)
{
    TVector3 diff = v1 - v2;
    return static_cast<float> (diff.Mag() ); // get magnitude (=rho=Sqrt(x*x+y*y+z*z)))
}


float VectorXYDist (TVector3 v1, TVector3 v2)
{
    v2.SetZ (v1.Z() );
    return Vector3Dist (v1, v2);
}



EcalADC::EcalADC() {
    initHGaggregate();
    initLGaggregate();
    initScint();
    initMCpos();
}


void EcalADC::initScint()
{
    const std::array<float, NSCINTPLANES> MeVPeakLayer = {
        5.19256, 5.31434, 5.43623, 5.56329, 5.71436, 5.89574, 6.07625, 6.29347,
        6.53174, 6.80163, 7.13162, 7.51907, 8.00189, 8.59306, 9.37135, 10.4922
    };
    const std::map<PMTenum, int> PMT2Layer;
    std::vector <PMTenum > scintPMT = {P1se, P2sw, P3se, P4sw, P5se, P6sw, P7se, P8sw,
                                    P9se, P10sw, P11se, P12sw, P13se, P14sw, P15se, P16sw,
                                    P1nw, P2ne, P3nw, P4ne, P5nw, P6ne, P7nw, P8ne,
                                    P9nw, P10ne, P11nw, P12ne, P13nw, P14ne, P15nw, P16ne
                                   };
    for (int iPMT=0; iPMT<scintPMT.size(); iPMT++) {
        uint layer= iPMT % scintPMT.size()/2;
        float peak=MeVPeakLayer[layer];
        PMTenum idx=scintPMT[iPMT];
        hgPMT[idx].layerScint=layer;
        hgPMT[idx].scintMeVPeak=peak;
        lgPMT[idx].layerScint=layer;
        lgPMT[idx].scintMeVPeak=peak;

    }
    return;
}


void EcalADC::initMCpos() {
    std::vector<float> PMTzMC={312.2,297.42,282.64,267.86,253.08,238.3,223.52,208.74,193.96,179.18,
        164.814,150.51,136.241,121.944,107.659,93.3685};
   for (int i=0; i<NPMT; i++) {
      float x=(i%2 == 0)?82.5:-82.5; //mm
      float y=(i < NPMT/2)?82.5:-82.5; //mm
      MCtoPhysicalDetectorFrame mcVector(TVector3(x, y, PMTzMC[i]));
      hgPMT[i].physPosition=mcVector.GetPhysicalVector();
      lgPMT[i].physPosition=mcVector.GetPhysicalVector();
   }
   return;
}


void EcalADC::SetPositions(std::vector<Edep_Pos> pmt_info) {
    for (int ip = 0; ip < NPMT; ip++) {
        correctedPMTs[ip] = pmt_info[ip].totEdep;
        TVector3 PMTpos = hgPMT[ip].physPosition;
        TVector3 ParticlePos = pmt_info[ip].position;
        float distance = VectorXYDist (PMTpos, ParticlePos);
        float attcor = PMTAttCorr (distance);
        correctedPMTs[ip] *= attcor;
    }
}

void EcalADC::NormalizePMThg ( ushort* pmt_high)
{
    NormalizePMT(pmt_high, hgPMT);
    return;
}

void EcalADC::NormalizePMTlg ( ushort* pmt_low)
{
    NormalizePMT(pmt_low, lgPMT);
    return;
}


void EcalADC::NormalizePMT ( ushort* pmt_out, PMTarray pmtDB) {
    for (uint ip = 0; ip < NPMT; ip++) {
        float MeVToADC = EcalMev2ADCfactor (PMTiterator[ip], pmtDB);
        int untrimmedPMT = static_cast<int> (correctedPMTs[ip] * MeVToADC) + pmtDB[ip].pedMean;
        if (untrimmedPMT > NADC) untrimmedPMT = NADC - 1;
        pmt_out[ip] = static_cast<short> (untrimmedPMT);
    }
    return;
}



std::vector<int> EcalADC::GetPMTHGPeds()
{
    std::vector<int>  PMTHGPeds (NPMT);
    for (int ip = 0; ip < NPMT; ip++) {
        PMTHGPeds[ip] = static_cast<int> (hgPMT[ip].pedMean);
    }
    return PMTHGPeds;
}

std::vector<int> EcalADC::GetPMTLGPeds()
{
    std::vector<int>  PMTLGPeds (NPMT);
    for (int ip = 0; ip < NPMT; ip++) {
        PMTLGPeds[ip] = static_cast<int> (lgPMT[ip].pedMean);
    }
    return PMTLGPeds;
}



float EcalADC::EcalMev2ADCfactor (PMTenum PMT, PMTarray pmtDB)
{
    float MaxMeV = pmtDB[PMT].isScint? pmtDB[PMT].scintMeVPeak:15;
    float absMaxADC = pmtDB[PMT].maxPeak;
    float ped = pmtDB[PMT].pedMean;
    float relMaxADC = absMaxADC - ped;
    float MeV2ADC = relMaxADC / MaxMeV;
    return MeV2ADC;
}



float EcalADC::PMTAttCorr (float dist)
{
    float lambda = 2764.; //mm
    return exp (- dist / lambda);
}






void EcalADC::initHGaggregate () {
    TVector3 test= {0, 0, 0};
   hgPMT = {{
        {T1e  , "T1e"  ,  412.881081 ,  5.093883 ,   472.881081 , 0    , 0, 0, {0, 0, 0}},
        {T2e  , "T2e"  ,  414.176163 ,  4.326234 ,  1020.807331 , 0    , 0, 0, {0, 0, 0}},
        {T3e  , "T3e"  ,  415.425746 ,  4.121837 ,   541.034929 , 0    , 0, 0, {0, 0, 0}},
        {T4e  , "T4e"  ,  407.145891 ,  4.621238 ,   575.252082 , 0    , 0, 0, {0, 0, 0}},
        {T5e  , "T5e"  ,  425.449938 ,  6.050231 ,   550.449938 , 0    , 0, 0, {0, 0, 0}},
        {T6e  , "T6e"  ,  428.115108 ,  4.650233 ,   511.366271 , 0    , 0, 0, {0, 0, 0}},
        {P1se , "P1se" ,  430.355283 , 16.186345 ,   720.355283 , true , 0, 0, {0, 0, 0}},
        {P2sw , "P2sw" ,  417.464629 ,  6.610456 ,   707.464629 , true , 0, 0, {0, 0, 0}},
        {P3se , "P3se" ,  412.608072 ,  9.512528 ,   832.608072 , true , 0, 0, {0, 0, 0}},
        {P4sw , "P4sw" ,  430.881204 ,  6.871393 ,   915.881204 , true , 0, 0, {0, 0, 0}},
        {P5se , "P5se" ,  428.139805 ,  5.186284 ,   458.139805 , true , 0, 0, {0, 0, 0}},
        {P6sw , "P6sw" ,  416.522330 ,  5.849863 ,   646.522330 , true , 0, 0, {0, 0, 0}},
        {P7se , "P7se" ,  409.647628 ,  5.147244 ,   954.647628 , true , 0, 0, {0, 0, 0}},
        {P8sw , "P8sw" ,  410.832036 ,  6.252032 ,   870.832036 , true , 0, 0, {0, 0, 0}},
        {P9se , "P9se" ,  418.328410 ,  6.430319 ,   708.328410 , true , 0, 0, {0, 0, 0}},
        {P10sw, "P10sw",  423.420672 ,  6.879046 ,   833.420672 , true , 0, 0, {0, 0, 0}},
        {P11se, "P11se",  430.422299 ,  7.082736 ,   865.422299 , true , 0, 0, {0, 0, 0}},
        {P12sw, "P12sw",  438.338769 ,  6.347400 ,   893.338769 , true , 0, 0, {0, 0, 0}},
        {P13se, "P13se",  421.560884 ,  6.770046 ,  1056.560884 , true , 0, 0, {0, 0, 0}},
        {P14sw, "P14sw",  423.465648 ,  9.530436 ,  1108.465648 , true , 0, 0, {0, 0, 0}},
        {P15se, "P15se",  417.392465 ,  5.913402 ,  1222.392465 , true , 0, 0, {0, 0, 0}},
        {P16sw, "P16sw",  420.224073 ,  6.112400 ,  1360.224073 , true , 0, 0, {0, 0, 0}},
        {VNu  , "VNu"  ,  424.774791 ,  6.582503 ,   514.774791 , 0    , 0, 0, {0, 0, 0}},
        {VEu  , "VEu"  ,  429.370026 ,  6.455723 ,   529.370026 , 0    , 0, 0, {0, 0, 0}},
        {VSu  , "VSu"  ,  428.145046 , 13.088752 ,   478.145046 , 0    , 0, 0, {0, 0, 0}},
        {VWu  , "VWu"  ,  429.907221 , 18.225902 ,   494.907221 , 0    , 0, 0, {0, 0, 0}},
        {VBne , "VBne" ,  423.299925 ,  6.750211 ,   603.299925 , 0    , 0, 0, {0, 0, 0}},
        {L9sw , "L9sw" ,  438.722418 , 19.493861 ,   478.722418 , 0    , 0, 0, {0, 0, 0}},
        {L7nw , "L7nw" ,  422.969108 , 15.874327 ,   512.969108 , 0    , 0, 0, {0, 0, 0}},
        {L1ne , "L1ne" ,  415.129171 , 25.826501 ,   720.129171 , 0    , 0, 0, {0, 0, 0}},
        {L8w  , "L8w"  ,  413.458845 , 11.736555 ,   498.458845 , 0    , 0, 0, {0, 0, 0}},
        {L5c  , "L5c"  ,  430.865169 ,  7.090569 ,  1045.865169 , 0    , 0, 0, {0, 0, 0}},
        {T1w  , "T1w"  ,  312.678994 ,  5.263946 ,   372.678994 , 0    , 0, 0, {0, 0, 0}},
        {T2w  , "T2w"  ,  318.003454 ,  4.578126 ,   420.894085 , 0    , 0, 0, {0, 0, 0}},
        {T3w  , "T3w"  ,  310.250425 ,  4.322035 ,   426.860599 , 0    , 0, 0, {0, 0, 0}},
        {T4w  , "T4w"  ,  307.053455 ,  4.358171 ,   408.844309 , 0    , 0, 0, {0, 0, 0}},
        {T5w  , "T5w"  ,  315.675663 ,  4.549825 ,   438.424787 , 0    , 0, 0, {0, 0, 0}},
        {T6w  , "T6w"  ,  309.535722 ,  4.305248 ,   436.061963 , 0    , 0, 0, {0, 0, 0}},
        {P1nw , "P1nw" ,  402.542064 , 65.379367 ,   637.542064 , true , 0, 0, {0, 0, 0}},
        {P2ne , "P2ne" ,  328.774567 ,  9.286540 ,   593.774567 , true , 0, 0, {0, 0, 0}},
        {P3nw , "P3nw" ,  321.547513 ,  7.840695 ,   656.547513 , true , 0, 0, {0, 0, 0}},
        {P4ne , "P4ne" ,  332.553483 ,  7.773640 ,   927.553483 , true , 0, 0, {0, 0, 0}},
        {P5nw , "P5nw" ,  328.824463 ,  7.317108 ,   598.824463 , true , 0, 0, {0, 0, 0}},
        {P6ne , "P6ne" ,  326.966478 ,  7.821378 ,   606.966478 , true , 0, 0, {0, 0, 0}},
        {P7nw , "P7nw" ,  327.450339 ,  6.574737 ,   897.450339 , true , 0, 0, {0, 0, 0}},
        {P8ne , "P8ne" ,  318.161227 ,  8.743401 ,   948.161227 , true , 0, 0, {0, 0, 0}},
        {P9nw , "P9nw" ,  324.454721 ,  8.411402 ,   874.454721 , true , 0, 0, {0, 0, 0}},
        {P10ne, "P10ne",  315.770293 ,  7.612869 ,   740.770293 , true , 0, 0, {0, 0, 0}},
        {P11nw, "P11nw",  321.138785 ,  8.858730 ,   991.138785 , true , 0, 0, {0, 0, 0}},
        {P12ne, "P12ne",  328.406985 ,  8.270837 ,   818.406985 , true , 0, 0, {0, 0, 0}},
        {P13nw, "P13nw",  334.606293 ,  8.780044 ,  1064.606293 , true , 0, 0, {0, 0, 0}},
        {P14ne, "P14ne",  325.424411 ,  6.694034 ,   425.424411 , true , 0, 0, {0, 0, 0}},
        {P15nw, "P15nw",  312.662737 ,  6.885861 ,   377.662737 , true , 0, 0, {0, 0, 0}},
        {P16ne, "P16ne",  320.643009 ,  5.980882 ,   405.643009 , true , 0, 0, {0, 0, 0}},
        {VNd  , "VNd"  ,  329.124665 ,  7.170878 ,   389.124665 , 0    , 0, 0, {0, 0, 0}},
        {VEd  , "VEd"  ,  337.051129 ,  6.779592 ,   387.051129 , 0    , 0, 0, {0, 0, 0}},
        {VSd  , "VSd"  ,  331.479400 ,  6.541011 ,   381.479400 , 0    , 0, 0, {0, 0, 0}},
        {VWd  , "VWd"  ,  347.221667 ,  6.750870 ,   392.221667 , 0    , 0, 0, {0, 0, 0}},
        {VBsw , "VBsw" ,  341.079814 ,  7.080330 ,   531.079814 , 0    , 0, 0, {0, 0, 0}},
        {L3se , "L3se" ,  331.311224 ,  6.669596 ,   386.311224 , 0    , 0, 0, {0, 0, 0}},
        {L2e  , "L2e"  ,  334.087855 ,  7.406568 ,   649.087855 , 0    , 0, 0, {0, 0, 0}},
        {L6s  , "L6s"  ,  336.917577 ,  6.948810 ,   386.917577 , 0    , 0, 0, {0, 0, 0}},
        {L4n  , "L4n"  ,  337.570442 ,  5.134804 ,   837.570442 , 0    , 0, 0, {0, 0, 0}},
        {NC   , "NC"   ,  331.487573 ,  6.142241 ,          0   , 0    , 0, 0, {0, 0, 0}}
    }};
    return;
}


void EcalADC::initLGaggregate () {
    lgPMT ={{
        {T1e  , "T1e"  ,  399.008924 ,  3.268237 ,   403.239352 , 0    , 0, 0,  {0, 0, 0}},
        {T2e  , "T2e"  ,  399.584691 ,  3.384526 ,   447.308226 , 0    , 0, 0,  {0, 0, 0}},
        {T3e  , "T3e"  ,  522.220569 , 53.151085 ,   411.178542 , 0    , 0, 0,  {0, 0, 0}},
        {T4e  , "T4e"  ,  392.300114 ,  3.097322 ,   401.664659 , 0    , 0, 0,  {0, 0, 0}},
        {T5e  , "T5e"  ,  418.923692 ,  2.974240 ,   425.830162 , 0    , 0, 0,  {0, 0, 0}},
        {T6e  , "T6e"  ,  400.779195 ,  3.118238 ,   407.435170 , 0    , 0, 0,  {0, 0, 0}},
        {P1se , "P1se" ,  432.192769 ,  6.757755 ,   431.843263 , true , 0, 0,  {0, 0, 0}},
        {P2sw , "P2sw" ,  430.285032 , 11.007667 ,   429.923867 , true , 0, 0,  {0, 0, 0}},
        {P3se , "P3se" ,  431.893790 ,  8.433011 ,   431.652047 , true , 0, 0,  {0, 0, 0}},
        {P4sw , "P4sw" ,  442.374399 , 17.982996 ,   440.950400 , true , 0, 0,  {0, 0, 0}},
        {P5se , "P5se" ,  403.787713 ,  2.949230 ,   406.112563 , true , 0, 0,  {0, 0, 0}},
        {P6sw , "P6sw" ,  426.182138 , 13.230503 ,   428.073099 , true , 0, 0,  {0, 0, 0}},
        {P7se , "P7se" ,  447.339483 , 24.364488 ,   449.076626 , true , 0, 0,  {0, 0, 0}},
        {P8sw , "P8sw" ,  434.475100 , 21.708419 ,   446.011332 , true , 0, 0,  {0, 0, 0}},
        {P9se , "P9se" ,  420.429008 , 13.965482 ,   422.585494 , true , 0, 0,  {0, 0, 0}},
        {P10sw, "P10sw",  435.153799 , 20.107288 ,   435.245457 , true , 0, 0,  {0, 0, 0}},
        {P11se, "P11se",  439.867632 , 18.227464 ,   441.495067 , true , 0, 0,  {0, 0, 0}},
        {P12sw, "P12sw",  443.912112 , 14.300461 ,   446.128243 , true , 0, 0,  {0, 0, 0}},
        {P13se, "P13se",  446.087428 , 22.311186 ,   450.293558 , true , 0, 0,  {0, 0, 0}},
        {P14sw, "P14sw",  463.972819 , 24.098870 ,   466.436905 , true , 0, 0,  {0, 0, 0}},
        {P15se, "P15se",  467.835340 , 29.908638 ,   467.549325 , true , 0, 0,  {0, 0, 0}},
        {P16sw, "P16sw",  459.237594 , 33.453373 ,   461.394218 , true , 0, 0,  {0, 0, 0}},
        {VNu  , "VNu"  ,  395.663711 ,  4.076380 ,   402.178176 , 0    , 0, 0,  {0, 0, 0}},
        {VEu  , "VEu"  ,  396.964784 ,  3.217283 ,   404.444171 , 0    , 0, 0,  {0, 0, 0}},
        {VSu  , "VSu"  ,  416.334958 ,  3.104289 ,   419.981376 , 0    , 0, 0,  {0, 0, 0}},
        {VWu  , "VWu"  ,  396.684153 ,  2.932975 ,   402.386454 , 0    , 0, 0,  {0, 0, 0}},
        {VBne , "VBne" ,  402.246254 ,  3.154824 ,   414.047232 , 0    , 0, 0,  {0, 0, 0}},
        {L9sw , "L9sw" ,  401.651129 ,  3.065235 ,   405.447595 , 0    , 0, 0,  {0, 0, 0}},
        {L7nw , "L7nw" ,  397.538343 ,  3.182374 ,   408.138116 , 0    , 0, 0,  {0, 0, 0}},
        {L1ne , "L1ne" ,  407.914169 ,  3.105616 ,   437.641120 , 0    , 0, 0,  {0, 0, 0}},
        {L8w  , "L8w"  ,  392.045656 ,  3.218268 ,   400.759519 , 0    , 0, 0,  {0, 0, 0}},
        {L5c  , "L5c"  ,  411.536058 ,  3.010167 ,   470.688332 , 0    , 0, 0,  {0, 0, 0}},
        {T1w  , "T1w"  ,  304.345074 ,  3.609370 ,   307.430866 , 0    , 0, 0,  {0, 0, 0}},
        {T2w  , "T2w"  ,  300.802398 , 19.557175 ,   299.718386 , 0    , 0, 0,  {0, 0, 0}},
        {T3w  , "T3w"  ,  355.291697 ,  8.634542 ,   303.702851 , 0    , 0, 0,  {0, 0, 0}},
        {T4w  , "T4w"  ,  304.661661 ,  3.563051 ,   311.278210 , 0    , 0, 0,  {0, 0, 0}},
        {T5w  , "T5w"  ,  298.446013 ,  3.482449 ,   307.567141 , 0    , 0, 0,  {0, 0, 0}},
        {T6w  , "T6w"  ,  309.383422 ,  3.482624 ,   322.993607 , 0    , 0, 0,  {0, 0, 0}},
        {P1nw , "P1nw" ,  335.841188 , 13.207743 ,   333.398119 , true , 0, 0,  {0, 0, 0}},
        {P2ne , "P2ne" ,  328.413679 , 15.605322 ,   329.712036 , true , 0, 0,  {0, 0, 0}},
        {P3nw , "P3nw" ,  370.772189 , 24.145603 ,   335.538383 , true , 0, 0,  {0, 0, 0}},
        {P4ne , "P4ne" ,  359.434118 , 20.855813 ,   355.684706 , true , 0, 0,  {0, 0, 0}},
        {P5nw , "P5nw" ,  335.937379 , 10.226122 ,   335.818005 , true , 0, 0,  {0, 0, 0}},
        {P6ne , "P6ne" ,  327.667156 ,  9.556422 ,   324.490332 , true , 0, 0,  {0, 0, 0}},
        {P7nw , "P7nw" ,  347.850633 , 20.023048 ,   347.557803 , true , 0, 0,  {0, 0, 0}},
        {P8ne , "P8ne" ,  359.138266 , 22.355495 ,   354.606778 , true , 0, 0,  {0, 0, 0}},
        {P9nw , "P9nw" ,  341.254559 , 19.109021 ,   343.348960 , true , 0, 0,  {0, 0, 0}},
        {P10ne, "P10ne",  342.170585 , 18.816923 ,   343.481687 , true , 0, 0,  {0, 0, 0}},
        {P11nw, "P11nw",  380.749801 , 22.141895 ,   349.727029 , true , 0, 0,  {0, 0, 0}},
        {P12ne, "P12ne",  343.777956 , 18.753798 ,   344.095124 , true , 0, 0,  {0, 0, 0}},
        {P13nw, "P13nw",  358.998911 , 24.947332 ,   363.255529 , true , 0, 0,  {0, 0, 0}},
        {P14ne, "P14ne",  384.513106 , 32.911301 ,   311.611647 , true , 0, 0,  {0, 0, 0}},
        {P15nw, "P15nw",  364.697157 ,  8.446660 ,   302.963165 , true , 0, 0,  {0, 0, 0}},
        {P16ne, "P16ne",  395.070524 , 38.498187 ,   326.520920 , true , 0, 0,  {0, 0, 0}},
        {VNd  , "VNd"  ,  311.771857 ,  3.560220 ,   316.243719 , 0    , 0, 0,  {0, 0, 0}},
        {VEd  , "VEd"  ,  314.996470 , 18.185264 ,   320.851029 , 0    , 0, 0,  {0, 0, 0}},
        {VSd  , "VSd"  ,  308.885617 ,  3.508393 ,   312.461533 , 0    , 0, 0,  {0, 0, 0}},
        {VWd  , "VWd"  ,  314.055274 ,  3.325341 ,   317.915942 , 0    , 0, 0,  {0, 0, 0}},
        {VBsw , "VBsw" ,  308.673573 ,  3.521295 ,   323.238075 , 0    , 0, 0,  {0, 0, 0}},
        {L3se , "L3se" ,  313.814653 ,  3.519594 ,   319.099616 , 0    , 0, 0,  {0, 0, 0}},
        {L2e  , "L2e"  ,  318.283583 ,  3.499119 ,   348.363413 , 0    , 0, 0,  {0, 0, 0}},
        {L6s  , "L6s"  ,  298.484384 ,  9.275479 ,   302.829829 , 0    , 0, 0,  {0, 0, 0}},
        {L4n  , "L4n"  ,  308.245027 ,  3.764058 ,   362.481758 , 0    , 0, 0,  {0, 0, 0}},
        {NC   , "NC"   ,  309.820007 ,  4.063671 ,          0   , 0    , 0, 0,  {0, 0, 0}}
    }};
    return;
}



