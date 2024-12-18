#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <utility>
#include <memory>

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"

class Particle {
public:
    double t, x, y, z, mass, p0, px, py, pz;
    int pdg, ID, charge;
    bool isInitNucl;

    Particle() 
        : t(-1), x(0), y(0), z(0), mass(0), p0(0), px(0), py(0), pz(0), pdg(0), ID(0), charge(0), isInitNucl(false) {}

    Particle(double t, double x, double y, double z, double mass, double p0, double px, double py, double pz, int pdg, int ID, int charge) 
        : t(t), x(x), y(y), z(z), mass(mass), p0(p0), px(px), py(py), pz(pz), pdg(pdg), ID(ID), charge(charge), isInitNucl(false) {}

    void setSpectator(){
        t = -999.;
        x = -999.;
        y = -999.;
        z = -999.;
    }
};

int main(int argc, char *argv[]) {

    if (argc != 3) {
        std::cout << "usage:   " << argv[0] << " input outputfile\n";
        exit(0);
    }

    // Read number of events to convert from the command line
    const char* oscarDst = argv[2];

    // Read input file name
    const char* inFileName = argv[1];

    
    std::unique_ptr<TFile> outputFile(new TFile(oscarDst, "RECREATE"));
    TTree tree("Particles", "Particles");

    // Контейнеры для веток
    int tnpart;
    double timpactParameter;
    std::vector<double> tt, tx, ty, tz, tmass, tp0, tpx, tpy, tpz;
    std::vector<int> tpdg, tID, tcharge;

    // Ветки дерева
    tree.Branch("npart", &tnpart);
    tree.Branch("impactParameter", &timpactParameter);
    tree.Branch("t", &tt);
    tree.Branch("x", &tx);
    tree.Branch("y", &ty);
    tree.Branch("z", &tz);
    tree.Branch("mass", &tmass);
    tree.Branch("p0", &tp0);
    tree.Branch("px", &tpx);
    tree.Branch("py", &tpy);
    tree.Branch("pz", &tpz);
    tree.Branch("ID", &tID);
    tree.Branch("pdg", &tpdg);
    tree.Branch("charge", &tcharge);

    std::ifstream infile(inFileName);

    bool isElastic = false;
    int ev_num = -1;
    int preevent = -1;
    int n_part = -1;
    int mode = -1; // 0 - in, 1 - out, 2 - end, 3 - skip
    
    int _all_particles_counter = 0;
    int _200_all_particle_counter = 0;
    int _200_nonelastic_particle_counter = 0;
    
    int _not_spectator_particle_counter = 0;
    int _spectator_particle_counter = 0;

    int _elastic_event_counter = 0;

    int startParticlesNum = 394;

    std::string line;
    std::vector<int> spectatorsID;
    std::map<int, Particle> buffer;
    std::map<int, Particle> eventBuffer;
    std::map<int, Particle> endBuffer;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);

        if (line[0] == '#') {
            std::string buff, keyWord;
            iss >> buff >> buff >> ev_num >> keyWord >> n_part;

            if (preevent != ev_num && ev_num != 0) { // чтобы проверить точно ли мы перебираем все события
                std::cout << "event # " << ev_num << " in progress" << std::endl;
                preevent = ev_num;
            }

            if (buff == "interaction") {
                mode = 4;
                continue;
            }else if(buff == "event"){
                if (keyWord == "in") {
                    mode = 0;
                    isElastic = false;
                    buffer.clear();
                    endBuffer.clear();
                    eventBuffer.clear();

                    tt.clear();
                    tx.clear();
                    ty.clear();
                    tz.clear();
                    tmass.clear();
                    tp0.clear();
                    tpx.clear();
                    tpy.clear();
                    tpz.clear();
                    tID.clear();
                    tpdg.clear();
                    tcharge.clear();
                    
                    continue;
                } else if (keyWord == "out") {
                    if(n_part == startParticlesNum) {
                        mode = 3;
                        isElastic = true;
                        _elastic_event_counter++;
                    } else {
                        mode = 1;
                        isElastic = false;
                    }
                    tnpart = n_part;
                    continue;
                } else if (keyWord == "end") {
                    mode = 2;
                    iss >> buff >> timpactParameter;
                    if (isElastic){
                        timpactParameter = -1.;
                        continue;
                    }
                } else {
                    mode = 3;
                    continue;
                }
            }
        }

        _all_particles_counter++;
        if (mode == 3) continue;

        double t, x, y, z, mass, p0, px, py, pz;
        int pdg, ID, charge;
        iss >> t >> x >> y >> z >> mass >> p0 >> px >> py >> pz >> pdg >> ID >> charge;
        Particle p(t, x, y, z, mass, p0, px, py, pz, pdg, ID, charge);

        if(t == 200) _200_all_particle_counter++;

        // mode = 0 - in, 1 - out, 2 - end, 3 - skip, 4 - interaction
        if (mode == 4) {
            if (ID < 394) p.isInitNucl = true;
            buffer[ID] = p;
        } else if (mode == 0){
            buffer[ID] = p;
        } else if (mode == 1) {
           if (isElastic) continue;
           endBuffer[ID] = p;
           _200_nonelastic_particle_counter++;
        } else if (mode == 2) {
            int counter = 0;
            //std::cout << "endBuffer size = " << endBuffer.size() << std::endl;
            for (const auto& currentParticle : buffer){ // проверка на то является ли частица спектатором
                int id = currentParticle.first;
                Particle p1 = currentParticle.second;
                if(endBuffer.find(id) == endBuffer.end()) continue;
                
                if (p1.isInitNucl){
                    p1.setSpectator();
                    eventBuffer[id] = p1;
                    _spectator_particle_counter++;
                }else{
                    eventBuffer[id] = buffer[id];
                    _not_spectator_particle_counter++;
                }
                counter++;
            }
            if ((endBuffer.size() - counter)!= 0) std::cout << "Error " << std::endl;
            
            for (const auto& currentParticle : eventBuffer) {
                const Particle& p1 = currentParticle.second;
                tt.push_back(p1.t); tx.push_back(p1.x); ty.push_back(p1.y); tz.push_back(p1.z);
                tmass.push_back(p1.mass); tp0.push_back(p1.p0); 
                tpx.push_back(p1.px); tpy.push_back(p1.py); tpz.push_back(p1.pz);
                tID.push_back(p1.ID); tpdg.push_back(p1.pdg); tcharge.push_back(p1.charge);
            }

            tree.Fill();
            buffer.clear();
            endBuffer.clear();
            eventBuffer.clear();
        }
    }
    
    outputFile->cd();
    tree.Write();
    outputFile->Close();

    std::cout << "Number of all(line) particles in .oscar = " << _all_particles_counter << std::endl

        << "Number of 200'th particles = " << _200_all_particle_counter << std::endl
        << "Numer of non elastic 200'th particles = " << _200_nonelastic_particle_counter << std::endl

        << "Total number of stored particles = " << _spectator_particle_counter + _not_spectator_particle_counter << std::endl
        << "Numbers of particles who survived to 200 fm/c = " << _not_spectator_particle_counter << std::endl
        << "Number of spectators = " << _spectator_particle_counter << std::endl
        << "Number of elastic event = " << _elastic_event_counter << std::endl;

    return 0;
}
