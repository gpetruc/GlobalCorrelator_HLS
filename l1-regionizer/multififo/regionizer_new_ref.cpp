#include "regionizer_new_ref.h"

#include <iostream>

#include "regionizer_elements_ref.icc"


l1ct::MultififoRegionizerEmulator::MultififoRegionizerEmulator(unsigned int nendcaps, unsigned int nclocks, unsigned int ntk, unsigned int ncalo, unsigned int nem, unsigned int nmu, bool streaming, unsigned int outii) :
    RegionizerEmulator(),
    nendcaps_(nendcaps), nclocks_(nclocks), ntk_(ntk), ncalo_(ncalo), nem_(nem), nmu_(nmu), outii_(outii), streaming_(streaming),
    tkRegionizer_(ntk, streaming ? (ntk+outii-1)/outii : ntk, streaming, outii),
    hadCaloRegionizer_(ncalo, streaming ? (ncalo+outii-1)/outii : ncalo, streaming, outii),
    emCaloRegionizer_(nem, streaming ? (nem+outii-1)/outii : nem, streaming, outii),
    muRegionizer_(nmu, streaming ? std::max(1u,(nmu+outii-1)/outii) : nmu, streaming, outii),
    init_(false)
{
    // now we initialize the routes: track finder
    for (unsigned int ie = 0; ie < nendcaps && ntk > 0; ++ie) {
        for (unsigned int is = 0; is < NTK_SECTORS; ++is) { // 9 tf sectors
            for (unsigned int il = 0; il < NTK_LINKS; ++il) { // max tracks per sector per clock
                unsigned int isp = (is+1)%NTK_SECTORS, ism = (is+NTK_SECTORS-1)%NTK_SECTORS;
                tkRoutes_.emplace_back(is  + NTK_SECTORS*ie, il, is + NTK_SECTORS*ie, il);
                tkRoutes_.emplace_back(isp + NTK_SECTORS*ie, il, is + NTK_SECTORS*ie, il+2);
                tkRoutes_.emplace_back(ism + NTK_SECTORS*ie, il, is + NTK_SECTORS*ie, il+4);
            }
        }
    }
    // hgcal
    assert(NCALO_SECTORS == 3 && NTK_SECTORS == 9); // otherwise math below is broken, but it's hard to make it generic
    for (unsigned int ie = 0; ie < nendcaps; ++ie) {
        for (unsigned int is = 0; is < NCALO_SECTORS; ++is) { // NCALO_SECTORS sectors
            for (unsigned int il = 0; il < NCALO_LINKS; ++il) { // max clusters per sector per clock
                for (unsigned int j = 0; j < 3; ++j) {
                    caloRoutes_.emplace_back(is + 3*ie, il, 3*is+j + 9*ie, il);
                    if (j) {
                        caloRoutes_.emplace_back((is+1)%3 + 3*ie, il, 3*is+j + 9*ie, il+4);
                    }
                }
            }
        }
    }
    // mu
    for (unsigned int il = 0; il < NMU_LINKS && nmu > 0; ++il) { // max clusters per sector per clock
        for (unsigned int j = 0; j < NTK_SECTORS*nendcaps; ++j) {
            muRoutes_.emplace_back(0, il, j, il);
        }
    }
}

l1ct::MultififoRegionizerEmulator::~MultififoRegionizerEmulator()
{
}

void l1ct::MultififoRegionizerEmulator::initSectorsAndRegions(const RegionizerDecodedInputs & in, const std::vector<PFInputRegion> & out) {
    assert(out.size() == NTK_SECTORS*nendcaps_);
    if (ntk_) {
        assert(in.track.size() == NTK_SECTORS*nendcaps_);
        tkRegionizer_.initSectors(in.track);
        tkRegionizer_.initRegions(out);
        tkRegionizer_.initRouting(tkRoutes_);
    }
    if (ncalo_) {
        assert(in.hadcalo.size() == NCALO_SECTORS*nendcaps_);
        hadCaloRegionizer_.initSectors(in.hadcalo);
        hadCaloRegionizer_.initRegions(out);
        hadCaloRegionizer_.initRouting(caloRoutes_);
    }
    if (nem_) {
        assert(in.emcalo.size() == NCALO_SECTORS*nendcaps_);
        emCaloRegionizer_.initSectors(in.emcalo);
        emCaloRegionizer_.initRegions(out);
        emCaloRegionizer_.initRouting(caloRoutes_);
    }
    if (nmu_) {
        muRegionizer_.initSectors(in.muon);
        muRegionizer_.initRegions(out);
        muRegionizer_.initRouting(muRoutes_);
    }
}

// clock-cycle emulation
bool l1ct::MultififoRegionizerEmulator::step(bool newEvent, const std::vector<l1ct::TkObjEmu> & links, std::vector<l1ct::TkObjEmu> & out ) {
    return tkRegionizer_.step(newEvent, links, out);
}
bool l1ct::MultififoRegionizerEmulator::step(bool newEvent, const std::vector<l1ct::EmCaloObjEmu> & links, std::vector<l1ct::EmCaloObjEmu> & out ) {
    return emCaloRegionizer_.step(newEvent, links, out);
}
bool l1ct::MultififoRegionizerEmulator::step(bool newEvent, const std::vector<l1ct::HadCaloObjEmu> & links, std::vector<l1ct::HadCaloObjEmu> & out ) {
    return hadCaloRegionizer_.step(newEvent, links, out);
}
bool l1ct::MultififoRegionizerEmulator::step(bool newEvent, const std::vector<l1ct::MuObjEmu> & links, std::vector<l1ct::MuObjEmu> & out ) {
    return muRegionizer_.step(newEvent, links, out);
}


void l1ct::MultififoRegionizerEmulator::fillLinks(unsigned int iclock, const l1ct::RegionizerDecodedInputs & in, std::vector<l1ct::TkObjEmu> & links) {
    assert(ntk_ > 0);
    links.resize(NTK_SECTORS*NTK_LINKS*nendcaps_);
    for (unsigned int is = 0, idx = 0; is < NTK_SECTORS*nendcaps_; ++is) { // tf sectors
        const l1ct::DetectorSector<l1ct::TkObjEmu> & sec = in.track[is];
        for (unsigned int il = 0; il < NTK_LINKS; ++il, ++idx) { 
            int ioffs = iclock * NTK_LINKS + il; 
            if (ioffs < sec.size()) {
                links[idx] = sec[ioffs];
            } else {
                links[idx].clear();
            }
        }
    }
}

template<typename T>
void l1ct::MultififoRegionizerEmulator::fillCaloLinks_(unsigned int iclock, const std::vector<DetectorSector<T>> & in, std::vector<T> & links) {
    links.resize(NCALO_SECTORS*NCALO_LINKS*nendcaps_);
    for (unsigned int is = 0, idx = 0; is < NCALO_SECTORS*nendcaps_; ++is) { 
        for (unsigned int il = 0; il < NCALO_LINKS; ++il, ++idx) { 
            int ioffs = iclock * NCALO_LINKS + il; 
            if (ioffs < in[is].size()) {
                links[idx] = in[is][ioffs];
            } else {
                links[idx].clear();
            }
        }
    }
}

void l1ct::MultififoRegionizerEmulator::fillLinks(unsigned int iclock, const l1ct::RegionizerDecodedInputs & in, std::vector<l1ct::HadCaloObjEmu> & links) {
    assert(ncalo_ > 0);
    fillCaloLinks_(iclock, in.hadcalo, links);
}

void l1ct::MultififoRegionizerEmulator::fillLinks(unsigned int iclock, const l1ct::RegionizerDecodedInputs & in, std::vector<l1ct::EmCaloObjEmu> & links) {
    assert(nem_ > 0);
    fillCaloLinks_(iclock, in.emcalo, links);
}

void l1ct::MultififoRegionizerEmulator::fillLinks(unsigned int iclock, const l1ct::RegionizerDecodedInputs & in, std::vector<l1ct::MuObjEmu> & links) {
    assert(nmu_ > 0);
    links.resize(NMU_LINKS);
    // we have 2 muons on odd clock cycles, and 1 muon on even clock cycles.
    assert(NMU_LINKS == 2);  
    for (unsigned int il = 0, idx = 0; il < NMU_LINKS; ++il, ++idx) { 
        int ioffs = (iclock * 3)/2 + il; 
        if (ioffs < in.muon.size() && (il == 0 || iclock % 2 == 1)) {
            links[idx] = in.muon[ioffs];
        } else {
            links[idx].clear();
        }
    }
}

void l1ct::MultififoRegionizerEmulator::toFirmware(const std::vector<l1ct::TkObjEmu> & emu, TkObj fw[NTK_SECTORS][NTK_LINKS]) {
    if (ntk_ == 0) return;
    assert(emu.size() == NTK_SECTORS*NTK_LINKS*nendcaps_);
    for (unsigned int is = 0, idx = 0; is < NTK_SECTORS*nendcaps_; ++is) { // tf sectors
        for (unsigned int il = 0; il < NTK_LINKS; ++il, ++idx) { 
            fw[is][il] = emu[idx];
        }
    }
}
void l1ct::MultififoRegionizerEmulator::toFirmware(const std::vector<l1ct::HadCaloObjEmu> & emu, HadCaloObj fw[NCALO_SECTORS][NCALO_LINKS]) {
    if (ncalo_ == 0) return;
    assert(emu.size() == NCALO_SECTORS*NCALO_LINKS*nendcaps_);
    for (unsigned int is = 0, idx = 0; is < NCALO_SECTORS*nendcaps_; ++is) { // tf sectors
        for (unsigned int il = 0; il < NCALO_LINKS; ++il, ++idx) { 
            fw[is][il] = emu[idx];
        }
    }
}

void l1ct::MultififoRegionizerEmulator::toFirmware(const std::vector<l1ct::EmCaloObjEmu> & emu, EmCaloObj fw[NCALO_SECTORS][NCALO_LINKS]) {
    if (nem_ == 0) return;
    assert(emu.size() == NCALO_SECTORS*NCALO_LINKS*nendcaps_);
    for (unsigned int is = 0, idx = 0; is < NCALO_SECTORS*nendcaps_; ++is) { // tf sectors
        for (unsigned int il = 0; il < NCALO_LINKS; ++il, ++idx) { 
            fw[is][il] = emu[idx];
        }
    }
}

void l1ct::MultififoRegionizerEmulator::toFirmware(const std::vector<l1ct::MuObjEmu> & emu, MuObj fw[NMU_LINKS]) {
    if (nmu_ == 0) return;
    assert(emu.size() == NMU_LINKS);
    for (unsigned int il = 0, idx = 0; il < NMU_LINKS; ++il, ++idx) { 
        fw[il] = emu[idx];
    }
}

void l1ct::MultififoRegionizerEmulator::run(const RegionizerDecodedInputs & in, std::vector<PFInputRegion> & out) {
    assert(false);
}

