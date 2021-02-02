#ifndef FASTPUPPI_NTUPLERPRODUCER_DISCRETEPF2FIRMWARE_H
#define FASTPUPPI_NTUPLERPRODUCER_DISCRETEPF2FIRMWARE_H

/// NOTE: this include is not standalone, since the path to DiscretePFInputs is different in CMSSW & Vivado_HLS

#include "../dataformats/layer1_objs.h"
#include "../dataformats/pf.h"
#include "../dataformats/puppi.h"
#include <vector>

namespace dpf2fw {

    // convert inputs from discrete to firmware
    inline void convert(const l1tpf_impl::PropagatedTrack & in, l1ct::TkObj &out) {
        out.clear();
        out.hwPt = l1ct::Scales::makePt(in.hwPt);
        out.hwEta = in.hwEta; // @calo
        out.hwPhi = in.hwPhi; // @calo
        out.hwZ0 = in.hwZ0;
        out.hwQuality = l1ct::TkObj::PFLOOSE + (in.hwStubs >= 6 && in.hwChi2 < 500 ? l1ct::TkObj::PFTIGHT : 0);
    }

    inline l1ct::TkObj transformConvert(const l1tpf_impl::PropagatedTrack & in) {
        l1ct::TkObj out;
        convert(in, out);
        return out;
    }

    inline void convert(const l1tpf_impl::CaloCluster & in, l1ct::HadCaloObj & out) {
        out.clear();
        out.hwPt = l1ct::Scales::makePt(in.hwPt);
        out.hwEmPt = l1ct::Scales::makePt(in.hwEmPt);
        out.hwEta = in.hwEta;
        out.hwPhi = in.hwPhi;
        out.hwIsEM = in.isEM;
    }
    inline void convert(const l1tpf_impl::CaloCluster & in, l1ct::EmCaloObj & out) {
        out.clear();
        out.hwPt = l1ct::Scales::makePt(in.hwPt);
        out.hwPtErr = l1ct::Scales::makePt(in.hwPtErr);
        out.hwEta = in.hwEta;
        out.hwPhi = in.hwPhi;
    }
    inline void convert(const l1tpf_impl::Muon & in, l1ct::MuObj & out) {
        out.clear();
        out.hwPt = l1ct::Scales::makePt(in.hwPt);
        out.hwEta = in.hwEta; // @calo
        out.hwPhi = in.hwPhi; // @calo
    }

    inline void convert(const l1tpf_impl::PFParticle &src, l1ct::PFChargedObj & pf) {
        pf.clear();
        pf.hwPt = l1ct::Scales::makePt(src.hwPt);
        pf.hwEta = src.hwEta;
        pf.hwPhi = src.hwPhi;
        switch(src.hwId) {
            case 0: pf.hwId = l1ct::ParticleID::mkChHad(src.track.hwCharge); break;
            case 1: pf.hwId = l1ct::ParticleID::mkElectron(src.track.hwCharge); break;
            case 2: pf.hwId = l1ct::ParticleID::HADZERO; assert(false); break;
            case 3: pf.hwId = l1ct::ParticleID::PHOTON; break;
            case 4: pf.hwId = l1ct::ParticleID::mkMuon(src.track.hwCharge); break;
        }
        pf.hwZ0 = src.track.hwZ0;
    }
    inline void convert(const l1tpf_impl::PFParticle &src, l1ct::PFNeutralObj & pf) {
        pf.clear();
        pf.hwPt = l1ct::Scales::makePt(src.hwPt);
        pf.hwEta = src.hwEta;
        pf.hwPhi = src.hwPhi;
        switch(src.hwId) {
            case 0: pf.hwId = l1ct::ParticleID::mkChHad(src.track.hwCharge); assert(false); break;
            case 1: pf.hwId = l1ct::ParticleID::mkElectron(src.track.hwCharge); assert(false); break;
            case 2: pf.hwId = l1ct::ParticleID::HADZERO; break;
            case 3: pf.hwId = l1ct::ParticleID::PHOTON; break;
            case 4: pf.hwId = l1ct::ParticleID::mkMuon(src.track.hwCharge); assert(false); break;
        }
    }


    inline void convert(const l1tpf_impl::InputRegion & src, l1ct::PFRegion & region) {
        region.hwEtaCenter = l1ct::Scales::makeGlbEta(src.etaCenter);
    }

    template<unsigned int NMAX, typename In, typename Out>
    void convert(const std::vector<In> & in, Out out[NMAX]) {
        for (unsigned int i = 0, n = std::min<unsigned int>(NMAX, in.size()); i < n; ++i) {
            convert(in[i], out[i]);
        }
        for (unsigned int i = in.size(); i < NMAX; ++i) {
            clear(out[i]);
        }
    }

    template<typename In, typename Out>
    void convert(unsigned int NMAX, const std::vector<In> & in, Out out[]) {
        for (unsigned int i = 0, n = std::min<unsigned int>(NMAX, in.size()); i < n; ++i) {
            convert(in[i], out[i]);
        }
        for (unsigned int i = in.size(); i < NMAX; ++i) {
            clear(out[i]);
        }
    }

} // namespace

#endif
