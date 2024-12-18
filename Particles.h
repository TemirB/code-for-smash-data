//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Oct  4 01:05:36 2024 by ROOT version 6.28/06
// from TTree Particles/Particles
// found on file: oscarDst.root
//////////////////////////////////////////////////////////

#ifndef Particles_h
#define Particles_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class Particles {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           npart;
   Double_t        impactParameter;
   std::vector<double>  *t;
   std::vector<double>  *x;
   std::vector<double>  *y;
   std::vector<double>  *z;
   std::vector<double>  *mass;
   std::vector<double>  *p0;
   std::vector<double>  *px;
   std::vector<double>  *py;
   std::vector<double>  *pz;
   std::vector<int>     *ID;
   std::vector<int>     *pdg;
   std::vector<int>     *charge;

   // List of branches
   TBranch        *b_npart;   //!
   TBranch        *b_impactParameter;   //!
   TBranch        *b_t;   //!
   TBranch        *b_x;   //!
   TBranch        *b_y;   //!
   TBranch        *b_z;   //!
   TBranch        *b_mass;   //!
   TBranch        *b_p0;   //!
   TBranch        *b_px;   //!
   TBranch        *b_py;   //!
   TBranch        *b_pz;   //!
   TBranch        *b_ID;   //!
   TBranch        *b_pdg;   //!
   TBranch        *b_charge;   //!

   // Debug flag
   Bool_t mDebug;

   Particles(TTree *tree=0);
   virtual ~Particles();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(const char* filename);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef Particles_cxx
Particles::Particles(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("oscarDst.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("oscarDst.root");
      }
      f->GetObject("Particles",tree);

   }
   Init(tree);
}

Particles::~Particles()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t Particles::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t Particles::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void Particles::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   t = 0;
   x = 0;
   y = 0;
   z = 0;
   mass = 0;
   p0 = 0;
   px = 0;
   py = 0;
   pz = 0;
   ID = 0;
   pdg = 0;
   charge = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("npart", &npart, &b_npart);
   fChain->SetBranchAddress("impactParameter", &impactParameter, &b_impactParameter);
   fChain->SetBranchAddress("t", &t, &b_t);
   fChain->SetBranchAddress("x", &x, &b_x);
   fChain->SetBranchAddress("y", &y, &b_y);
   fChain->SetBranchAddress("z", &z, &b_z);
   fChain->SetBranchAddress("mass", &mass, &b_mass);
   fChain->SetBranchAddress("p0", &p0, &b_p0);
   fChain->SetBranchAddress("px", &px, &b_px);
   fChain->SetBranchAddress("py", &py, &b_py);
   fChain->SetBranchAddress("pz", &pz, &b_pz);
   fChain->SetBranchAddress("ID", &ID, &b_ID);
   fChain->SetBranchAddress("pdg", &pdg, &b_pdg);
   fChain->SetBranchAddress("charge", &charge, &b_charge);
   Notify();
}

Bool_t Particles::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void Particles::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t Particles::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef Particles_cxx