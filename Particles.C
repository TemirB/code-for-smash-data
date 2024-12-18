#define Particles_cxx

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <cmath>
#include <fstream>

#include "Particles.h"

#include <TH1.h>
#include <TH3.h>
#include <TFile.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include "TChain.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TKey.h"
#include "TString.h"

void SaveHistograms(const char* inputFileName) {
   TFile *file = TFile::Open(inputFileName);

   if (!file || file->IsZombie()) {
      std::cerr << "Ошибка: Не удалось открыть файл " << inputFileName << std::endl;
      return;
   }

   const char* outputDir = "output_histograms";
   gSystem->Exec(Form("mkdir %s", outputDir));

   TIter nextkey(file->GetListOfKeys());
   TKey *key;
   
   while ((key = (TKey*)nextkey())) {
      TObject *obj = key->ReadObj();
      if (obj->InheritsFrom("TH1")) {
         TH1 *hist = (TH1*)obj;
         TCanvas *canvas = new TCanvas();
         hist->Draw();
         canvas->SaveAs(Form("%s/%s.png", outputDir, hist->GetName()));
         delete canvas;
      }
   }
   file->Close();
   delete file;
}

int classByCentr(Double_t impactparameter){
   if (impactparameter > 13.1) return 999;//  80-100 musor
   else if(impactparameter > 10.4) return 3;// 50-80
   else if(impactparameter > 8) return 2;// 30-50
   else if(impactparameter > 4.6) return 1;//10-30
   else return 0;//0-10
}

int classByKt(Double_t k_t){
   if (k_t < 0.15 || k_t > 0.6) return 999;//мусор?
   else if(k_t < 0.25) return 0;
   else if(k_t < 0.35) return 1;
   else if(k_t < 0.45) return 2;
   else return 3;
}

void Particles::Loop(const char* outFileName){
   mDebug = false;
   if (fChain == 0) return;

   // Long64_t nentries = fChain->GetEntriesFast();
   Long64_t nentries = fChain->GetEntries();
   Long64_t nbytes = 0, nb = 0;
   //const double mass = 0.13957;

   std::vector<std::pair<TLorentzVector, TLorentzVector>> particles;
   std::map<int, std::vector<std::vector<std::pair<TLorentzVector, TLorentzVector>>>> Buffer;

   TFile* outputFile = new TFile(outFileName, "RECREATE");

   TVector3 beam_axis(0, 0, 0);
   TVector3 pion_axis(0, 0, 0);

   std::array<std::string, 4> centralityNames = {"0-10", "10-30", "30-50", "50-80"};
   std::array<std::string, 4> ktNames = {"0.15-0.25", "0.25-0.35", "0.35-0.45", "0.45-0.60"};

   std::map<int, std::map<int, TH1D>> hists_A;
   std::map<int, std::map<int, TH1D>> hists_A_wei;
   std::map<int, std::map<int, TH1D>> hists_B;
   std::map<int, std::map<int, TH3D>> hists_A_3d;
   std::map<int, std::map<int, TH3D>> hists_A_wei_3d;
   std::map<int, std::map<int, TH3D>> hists_B_3d;

   for (unsigned long i = 0; i < centralityNames.size(); ++i) {
      for (unsigned long j = 0; j < ktNames.size(); ++j) {
         std::string suffix = centralityNames[i] + "_" + ktNames[j] + "_GeVc";// добавить % и GeV/c
         std::string name = "h_A_q_inv_" + suffix;
         hists_A[i][j] = TH1D(name.c_str(), name.c_str(), 100, 0., 1.);

         name = "h_A_wei_q_inv_" + suffix;
         hists_A_wei[i][j] = TH1D(name.c_str(), name.c_str(), 100, 0., 1.);

         name = "h_B_q_inv_" + suffix;
         hists_B[i][j] = TH1D(name.c_str(), name.c_str(), 100, 0., 1.);

         name = "h3d_A_q_" + suffix;
         hists_A_3d[i][j] = TH3D(name.c_str(), name.c_str(), 80, -0.4, 0.4,
                                                             80, -0.4, 0.4,
                                                             80, -0.4, 0.4);

         name = "h3d_A_wei_q_" + suffix;
         hists_A_wei_3d[i][j] = TH3D(name.c_str(), name.c_str(), 80, -0.4, 0.4,
                                                                 80, -0.4, 0.4,
                                                                 80, -0.4, 0.4);

         name = "h3d_B_q_" + suffix;
         hists_B_3d[i][j] = TH3D(name.c_str(), name.c_str(), 80, -0.4, 0.4,
                                                             80, -0.4, 0.4,
                                                             80, -0.4, 0.4);

      }
   }


   TH1F* multiplicity = new TH1F("multiplicity", "Multiplicity", 200, 394, 700);//perepisat
   multiplicity->SetTitle("Multiplicity; multiplicity; npart");
   TH1F* h_pseudorapidity = new TH1F("pseudorapidity", "Pseudorapidity", 100, -5, 5);
   h_pseudorapidity->SetTitle("Pseudorapidity; #eta; ");
   TH1F* h_rapidity = new TH1F("rapidity", "Rapidity_z", 100, -5, 5);
   h_rapidity->SetTitle("Rapidity_z; y [c]; ");
   TH1F* h_impact_parameter = new TH1F("impact_parameter", "Impact Parameter", 100, 0, 20);
   h_impact_parameter->SetTitle("Impact Parameter; b [fm]; ");
   TH1F* h_pt = new TH1F("pt", "Transverse Momentum", 100, 0, 2);
   h_pt->SetTitle("P_{T}; P_{T} [GeV]; ");
   TH1F* h_p_total = new TH1F("p_total", "Total Momentum", 100, 0, 2);
   h_p_total->SetTitle("Total Momentum; p_{total} [GeV]; ");
   TH1F* h_energy = new TH1F("energy", "Energy", 100, 0, 2);
   h_energy->SetTitle("Energy; E [GeV]; ");
   TH1F* h_time = new TH1F("t", "Time", 300, 0, 210);
   h_time->SetTitle("Time; t [fm/c]; ");
   TH1F* h_x = new TH1F("x", "X Position", 100, -10, 10);
   h_x->SetTitle("X Position; x [fm]; ");
   TH1F* h_y = new TH1F("y", "Y Position", 100, -10, 10);
   h_y->SetTitle("Y Position; y [fm]; ");
   TH1F* h_z = new TH1F("z", "Z Position", 100, -10, 10);
   h_z->SetTitle("Z Position; z [fm]; ");
   TH1F* h_px = new TH1F("px", "px", 100, -1.5, 1.5);
   h_px->SetTitle("X moentum; p_{x} [GeV]; ");
   TH1F* h_py = new TH1F("py", "py", 100, -1.5, 1.5);
   h_py->SetTitle("Y moentum; p_{y} [GeV]; ");
   TH1F* h_pz = new TH1F("pz", "pz", 100, -1.5, 1.5);
   h_pz->SetTitle("Z moentum; p_{z} [GeV]; ");

   if (mDebug) std::cout << "[DEBUG]: output histograms were created" << std::endl;
   

   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      if (mDebug) std::cout << "Event # " << jentry << " of " << nentries << " in progress" << std::endl;
      else        if (jentry%1000==0) std::cout << "Event # " << jentry << " of " << nentries << " in progress" << std::endl;
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

      b_impactParameter->GetEntry(ientry);
      h_impact_parameter->Fill(impactParameter);

      int impactPclass = classByCentr(impactParameter);
      if (impactPclass == 4) continue;
      
      b_t->GetEntry(ientry);
      b_x->GetEntry(ientry);
      b_y->GetEntry(ientry);
      b_z->GetEntry(ientry);
      b_p0->GetEntry(ientry);
      b_px->GetEntry(ientry);
      b_py->GetEntry(ientry);
      b_pz->GetEntry(ientry);
      b_pdg->GetEntry(ientry);

      multiplicity->Fill(npart);

      if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: total particles in event: " << npart << std::endl;

         for (Long64_t track = 0; track < npart; track++) {
            if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: Particle #: " << track << std::endl;
            TVector3 P(px->at(track), py->at(track), pz->at(track));
            beam_axis += P;
            if (std::abs(pdg->at(track)) != 211) continue;
            pion_axis += P;
            TLorentzVector Ptot(px->at(track), py->at(track), pz->at(track), p0->at(track));
            TLorentzVector r(x->at(track), y->at(track), z->at(track), t->at(track));
            TVector3 Pt(px->at(track), py->at(track), 0);
            Double_t pseudorapidity = Ptot.PseudoRapidity();
            if (TMath::Abs(pseudorapidity) > 1.5 || Ptot.Vect().Mag() < 0.15 || Ptot.Vect().Mag() > 1.5 || Pt.Mag() < 0.15 || Pt.Mag() > 1.5) continue;
            if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: Pion #: " << track << std::endl;
            Double_t energy = sqrt(Ptot.Vect().Mag2() + Ptot.M2());
            Double_t rapidity = Ptot.Rapidity();
            
            particles.push_back(std::make_pair(Ptot, r));
            //ipClassparticles[impactPclass].push_back(std::make_pair(Ptot, r));

            h_pt->Fill(Pt.Mag());
            h_p_total->Fill(Ptot.Vect().Mag());
            h_energy->Fill(energy);
            h_time->Fill(t->at(track));
            h_x->Fill(x->at(track));
            h_y->Fill(y->at(track));
            h_z->Fill(z->at(track));
            h_px->Fill(px->at(track));
            h_py->Fill(py->at(track));
            h_pz->Fill(pz->at(track));

            h_rapidity->Fill(rapidity);
            h_pseudorapidity->Fill(pseudorapidity);
         }
      if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: End of track loop." << std::endl;

      for(unsigned long i = 0; i < particles.size(); i++){
         const std::pair<TLorentzVector, TLorentzVector> particle1 = particles[i]; // particle1 = std::pair<TLorentzVector, TLorentzVector>
         const TLorentzVector particle1_mom = particle1.first; // 4-momentum
         const Double_t energy1 = particle1.first.E(); // energy for q_inv
         for(unsigned long j = i + 1; j < particles.size(); j++){
            const std::pair<TLorentzVector, TLorentzVector> particle2 = particles[j];
            const TLorentzVector particle2_mom = particle2.first;
            const Double_t energy2 = particle2.first.E();
            const Double_t femtoWeight = 1 + TMath::Cos((particle1_mom - particle2_mom) * (particle1.second - particle2.second)/0.197);

            const Int_t k_t_class = classByKt((particle1_mom.Vect() + particle2_mom.Vect()).Perp()/2);
            //if (k_t_class == 4) continue;



            const Double_t qinv = TMath::Sqrt((particle1_mom.Vect() - particle2_mom.Vect()).Mag2() - (energy1 - energy2) * (energy1 - energy2));

            const TLorentzVector q = particle1_mom - particle2_mom;
            const TLorentzVector P = particle1_mom + particle2_mom;
            const Double_t qlong = (P.T() * q.Z() - P.Z() * q.T())/TMath::Sqrt(P.T() * P.T() - P.Z() * P.Z());
            const Double_t qout = (P.X() * q.X() + P.Y() * q.Y())/P.Perp();
            const Double_t qside = (P.X() * q.Y() - P.Y() * q.X())/P.Perp();

            hists_A[impactPclass][k_t_class].Fill(qinv);
            hists_A_wei[impactPclass][k_t_class].Fill(qinv, femtoWeight);
            hists_A_3d[impactPclass][k_t_class].Fill(qout, qside, qlong);
            hists_A_wei_3d[impactPclass][k_t_class].Fill(qout, qside, qlong, femtoWeight);
         }
      }
      if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: End of pair loop of single event." << std::endl;

      if (Buffer[impactPclass].size() > 10) Buffer[impactPclass].erase(Buffer[impactPclass].begin());


      //std::vector<std::vector<std::pair<TLorentzVector, TLorentzVector>>> Buffer[impactPclass];
      for(const std::pair<TLorentzVector, TLorentzVector> &particle1 : particles){
         const TLorentzVector particle1_mom = particle1.first;
         const Double_t energy1 = particle1.first.E(); // energy for q_inv
         for(const std::vector<std::pair<TLorentzVector, TLorentzVector>> &preEvent : Buffer[impactPclass]){ //preEvent = pair<int, vector>
            for(const std::pair<TLorentzVector, TLorentzVector> &particle2 : preEvent){ //particle2 = pair<TLorentzVector, TLorentzVector>
               const TLorentzVector particle2_mom = particle2.first; //TLorentzVector
               const Double_t energy2 = particle2.first.E(); // energy for q_inv
               int k_t_class = classByKt((particle1_mom.Vect() + particle2_mom.Vect()).Perp()/2);

               const Double_t qinv = TMath::Sqrt((particle1_mom.Vect() - particle2_mom.Vect()).Mag2() - (energy1 - energy2) * (energy1 - energy2));
               const TLorentzVector q = particle1_mom - particle2_mom;
               const TLorentzVector P = particle1_mom + particle2_mom;
               const Double_t qlong = (P.T() * q.Z() - P.Z() * q.T())/TMath::Sqrt(P.T() * P.T() - P.Z() * P.Z());
               const Double_t qout = (P.X() * q.X() + P.Y() * q.Y())/P.Perp();
               const Double_t qside = (P.X() * q.Y() - P.Y() * q.X())/P.Perp();

               hists_B[impactPclass][k_t_class].Fill(qinv);
               hists_B_3d[impactPclass][k_t_class].Fill(qout, qside, qlong);
            }
         }
      }
      if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: End of pair loop of mixed event." << std::endl;

      Buffer[impactPclass].push_back(particles);
      particles.clear();
      if (mDebug) if (ientry==2397) std::cout << "[DEBUG]: Buffer was filled." << std::endl;

   } // end of event loop

   outputFile->cd();

   h_impact_parameter->Write();
   h_pt->Write();
   h_p_total->Write();
   h_energy->Write();
   h_time->Write();
   h_x->Write();
   h_y->Write();
   h_z->Write();
   h_px->Write();
   h_py->Write();
   h_pz->Write();
   multiplicity->Write();
   h_pseudorapidity->Write();
   h_rapidity->Write();


   for (unsigned long i = 0; i < centralityNames.size(); ++i) {
      for (unsigned long j = 0; j < ktNames.size(); ++j) {
         hists_A[i][j].Write();
         hists_A_wei[i][j].Write();
         hists_B[i][j].Write();
         
         hists_A_3d[i][j].Write();
         hists_A_wei_3d[i][j].Write();
         hists_B_3d[i][j].Write();
      }
   }

   std::cout << "beam axis: \t pion axis" << std::endl 
   << beam_axis.X() << "\t" << pion_axis.X() << std::endl 
   << beam_axis.Y() << "\t" << pion_axis.Y() << std::endl 
   << beam_axis.Z() << "\t" << pion_axis.Z() << std::endl;
   outputFile->Close();
}

int main(int argc, char *argv[]) {
   if (argc != 3) {
      std::cout << "usage:   " << argv[0] << " input outputfile\n";
      exit(0);
   }
   TString inFileName = argv[1];
   // /home/ubuntu/Temir/smash/build/data/script/runlist/split/runlist_3gev.list
   const char* outFileName = argv[2];

   TChain chain("Particles");
   if (inFileName.Contains(".root")) { // one root file 
      chain.Add(inFileName);
   } else if (inFileName.Contains(".list")) { // list of ROOT files
      std::ifstream file(inFileName.Data());
      std::string line;
      while(std::getline(file, line)){
         chain.Add(line.c_str());
      }
   }
   Particles p(&chain);
   p.Loop(outFileName);

   //SaveHistograms("output.root"); //если нужно сохранить png для отчета :)
   return 0;
}