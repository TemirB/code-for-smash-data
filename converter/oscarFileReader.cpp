#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <memory>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "McRun.h"
#include "McEvent.h"
#include "McParticle.h"
#include "McArrays.h"

class Particle {
public:
    double t, x, y, z, mass, p0, px, py, pz;
    int pdg, ID, charge;
    bool isInitNucl;

    Particle()
        : t(-1), x(0), y(0), z(0), mass(0), p0(0), px(0), py(0), pz(0), pdg(0), ID(0), charge(0), isInitNucl(false) {}

    Particle(double t, double x, double y, double z, double mass, double p0, double px, double py, double pz, int pdg, int ID, int charge)
        : t(t), x(x), y(y), z(z), mass(mass), p0(p0), px(px), py(py), pz(pz), pdg(pdg), ID(ID), charge(charge), isInitNucl(false) {}

    void setSpectator() {
        t = x = y = z = -999.;
    }
};

enum class Mode {
  InEvent = 0,
  OutEvent = 1,
  EndEvent = 2,
  SkipEvent = 3,
  Interaction = 4,

  Init = -1,
};


int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " input.oscar output.McDst.root\n";
    return 1;
  }

  const char* inFileName = argv[1];
  const char* outFileName = argv[2];

  TFile* outputFile = TFile::Open(outFileName, "RECREATE", "Oscar to McDst");
  outputFile->SetCompressionLevel(9);

  TTree* tree = new TTree("McDst", "Converted Oscar Data");

  TClonesArray* arrays[McArrays::NAllMcArrays];
  for (unsigned int i = 0; i < McArrays::NAllMcArrays; i++) {
    arrays[i] = new TClonesArray(McArrays::mcArrayTypes[i], McArrays::mcArraySizes[i]);
    tree->Branch(McArrays::mcArrayNames[i], &arrays[i], 65536, 99);
  }

  std::ifstream infile(inFileName);
  if (!infile.is_open()) {
    std::cerr << "Cannot open input file: " << inFileName << std::endl;
    return 1;
  }

  bool isElastic = false;
  int ev_num = -1;
  int preevent = -1;
  int n_part = -1;
  Mode mode = Mode::Init;
  int startParticlesNum = 394;

  double timpactParameter = -1.;

  std::unordered_map<int, Particle> buffer;
  std::unordered_map<int, Particle> endBuffer;
  std::unordered_map<int, Particle> eventBuffer;

  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);

    if (line.empty()) continue;

    if (line[0] == '#') {
      std::string dummy, keyWord;
      iss >> dummy >> dummy >> ev_num >> keyWord >> n_part;

      if (dummy == "interaction") {
        mode = Mode::Interaction;
        continue;
      } else if (dummy == "event") {
        if (keyWord == "in") {
          mode = Mode::InEvent;
          isElastic = false;

          buffer.clear();
          endBuffer.clear();
          eventBuffer.clear();
          for (unsigned int i = 0; i < McArrays::NAllMcArrays; i++) arrays[i]->Clear();
          continue;
        } else if (keyWord == "out") {
          mode = (n_part == startParticlesNum) ? Mode::SkipEvent : Mode::OutEvent;
          isElastic = (mode == Mode::SkipEvent);
          continue;
        } else if (keyWord == "end") {
          mode = Mode::EndEvent;
          iss >> dummy >> timpactParameter;
          if (isElastic) {
            timpactParameter = -1.;
            continue;
          } // Be careful when mode == end, important logic is executed, don't skip this mode
        } else {
          mode = Mode::SkipEvent;
          continue;
        }
      }
    }

    // skip (for example service line or init lines)
    if (mode == Mode::SkipEvent) continue;

    double t, x, y, z, mass, p0, px, py, pz;
    int pdg, ID, charge;
    iss >> t >> x >> y >> z >> mass >> p0 >> px >> py >> pz >> pdg >> ID >> charge;
    Particle p(t, x, y, z, mass, p0, px, py, pz, pdg, ID, charge);

    if (mode == Mode::Interaction || mode == Mode::InEvent) {
      if (ID < 394) p.isInitNucl = true;
      buffer[ID] = p;
    } else if (mode == Mode::OutEvent) {
      if (isElastic) continue;
      endBuffer[ID] = p;
    } else if (mode == Mode::EndEvent) {
      for (const auto& entry : buffer) {
        int id = entry.first;
        const Particle& initParticle = entry.second;

        if (endBuffer.find(id) == endBuffer.end()) continue;

        if (initParticle.isInitNucl) {
          Particle specParticle = initParticle;
          specParticle.setSpectator();
          eventBuffer[id] = specParticle;
          } else {
          eventBuffer[id] = initParticle;
        }
      }

      int idx = 0;
      for (const auto& entry : eventBuffer) {
        const Particle& p1 = entry.second;
        new((*(arrays[McArrays::Particle]))[arrays[McArrays::Particle]->GetEntries()])
        McParticle(idx++, p1.pdg, 0, 0, 0, -1, 0, nullptr,
                  p1.px, p1.py, p1.pz, p1.p0,
                  p1.x, p1.y, p1.z, p1.t);
      }

      McEvent* event = new((*(arrays[McArrays::Event]))[arrays[McArrays::Event]->GetEntries()]) McEvent();
      event->setEventNr(ev_num);
      event->setB(timpactParameter);
      event->setPhi(0.);
      event->setNes(1);
      event->setComment("");
      event->setStepNr(1);
      event->setStepT(200.);

      tree->Fill();
      buffer.clear();
      endBuffer.clear();
      eventBuffer.clear();
    }
  }

    McRun run("SMASH", "Converted from Oscar file",
              0, 0, 0., 0, 0, 0.,
              0., 0., -1, 0, 0, 0., tree->GetEntries());
    run.Write();

    outputFile->Write();
    outputFile->Close();

    std::cout << "Conversion completed. Total events: " << tree->GetEntries() << std::endl;

    return 0;
}
