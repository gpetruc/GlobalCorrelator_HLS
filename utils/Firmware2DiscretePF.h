#ifndef FASTPUPPI_NTUPLERPRODUCER_FIRMWARE2DISCRETEPF_H
#define FASTPUPPI_NTUPLERPRODUCER_FIRMWARE2DISCRETEPF_H

/// NOTE: this include is not standalone, since the path to DiscretePFInputs is different in CMSSW & Vivado_HLS

#include "../dataformats/pf.h"
#include <vector>
#include <cassert>

namespace fw2dpf {

    // convert inputs from discrete to firmware
    inline void convert(const PFChargedObj & src, const l1tpf_impl::PropagatedTrack & track, std::vector<l1tpf_impl::PFParticle> &out) {
        l1tpf_impl::PFParticle pf;
        pf.hwPt = src.hwPt;
        pf.hwEta = src.hwEta;
        pf.hwPhi = src.hwPhi;
        pf.hwVtxEta = src.hwEta; // FIXME: get from the track
        pf.hwVtxPhi = src.hwPhi; // before propagation
        pf.track = track; // FIXME: ok only as long as there is a 1-1 mapping
        pf.cluster.hwPt = 0;
        pf.cluster.src = nullptr;
        pf.muonsrc = nullptr;
        if (src.hwId.isMuon()) {
            pf.hwId = 4;
        } else if (src.hwId.isElectron()) {
            pf.hwId = 1;
        } else {
            pf.hwId = 0;
        }
        pf.hwStatus = 0;
        out.push_back(pf);
    }
    // convert inputs from discrete to firmware
    inline void convert(const TkObj & in, l1tpf_impl::PropagatedTrack & out) ;
    inline void convert(const PFChargedObj & src, const TkObj & track, std::vector<l1tpf_impl::PFParticle> &out) {
        l1tpf_impl::PFParticle pf;
        pf.hwPt = src.hwPt;
        pf.hwEta = src.hwEta;
        pf.hwPhi = src.hwPhi;
        pf.hwVtxEta = src.hwEta; // FIXME: get from the track
        pf.hwVtxPhi = src.hwPhi; // before propagation
        convert(track, pf.track); // FIXME: ok only as long as there is a 1-1 mapping
        pf.cluster.hwPt = 0;
        pf.cluster.src = nullptr;
        pf.muonsrc = nullptr;
        if (src.hwId.isMuon()) {
            pf.hwId = 4;
        } else if (src.hwId.isElectron()) {
            pf.hwId = 1;
        } else {
            pf.hwId = 0;
        }
        pf.hwStatus = 0;
        out.push_back(pf);
    }
    inline void convert(const PFNeutralObj & src, std::vector<l1tpf_impl::PFParticle> &out) {
        l1tpf_impl::PFParticle pf;
        pf.hwPt = src.hwPt;
        pf.hwEta = src.hwEta;
        pf.hwPhi = src.hwPhi;
        pf.hwVtxEta = src.hwEta;
        pf.hwVtxPhi = src.hwPhi;
        pf.track.hwPt = 0;
        pf.track.src = nullptr;
        pf.cluster.hwPt = src.hwPt;
        pf.cluster.src = nullptr;
        pf.muonsrc = nullptr;
        pf.hwId = src.hwId.isPhoton() ? 3 : 2;
        pf.hwStatus = 0;
        out.push_back(pf);
    }
    inline void convert_puppi(const PuppiObj & src, const l1tpf_impl::PropagatedTrack * track, std::vector<l1tpf_impl::PFParticle> &out) {
        l1tpf_impl::PFParticle pf;
        pf.hwPt = src.hwPt;
        pf.hwEta = src.hwEta;
        pf.hwPhi = src.hwPhi;
        pf.hwVtxEta = src.hwEta;
        pf.hwVtxPhi = src.hwPhi;
        switch(src.hwId.rawId()) {
            case ParticleID::PHOTON: pf.hwId = 3; break;
            case ParticleID::HADZERO: pf.hwId = 2; break;
            case ParticleID::ELEPLUS: pf.hwId =  1; break;
            case ParticleID::ELEMINUS: pf.hwId =  1; break;
            case ParticleID::MUPLUS: pf.hwId =  4; break;
            case ParticleID::MUMINUS: pf.hwId =  4; break;
            case ParticleID::HADPLUS: pf.hwId = 0; break;
            case ParticleID::HADMINUS: pf.hwId = 0; break;
        }
        if (src.hwId.charged()) {
            if (track != nullptr) {
                pf.track = *track; // FIXME: ok only as long as there is a 1-1 mapping
            } else {
                pf.track.hwPt = src.hwPt;
                pf.track.src = nullptr;
            }
            pf.cluster.hwPt = 0;
            pf.cluster.src = nullptr;
            pf.setPuppiW(1.0);
        } else {
            pf.track.hwPt = 0;
            pf.track.src = nullptr;
            pf.cluster.hwPt = src.hwPt;
            pf.cluster.src = nullptr;
            pf.hwPuppiWeight = src.hwPuppiW();
        }
        pf.muonsrc = nullptr;
        pf.hwStatus = 0;
        out.push_back(pf);
    }


    // convert inputs from discrete to firmware
    inline void convert(const TkObj & in, l1tpf_impl::PropagatedTrack & out) {
        out.hwPt = in.hwPt;
        out.hwCaloPtErr = in.hwPtErr;
        out.hwEta = in.hwEta; // @calo
        out.hwPhi = in.hwPhi; // @calo
        out.hwZ0 = in.hwZ0;
        out.src = nullptr;
    }
    inline void convert(const HadCaloObj & in, l1tpf_impl::CaloCluster & out) {
        out.hwPt = in.hwPt;
        out.hwEmPt = in.hwEmPt;
        out.hwEta = in.hwEta;
        out.hwPhi = in.hwPhi;
        out.isEM = in.hwIsEM;
        out.src = nullptr;
    }
    inline void convert(const EmCaloObj & in, l1tpf_impl::CaloCluster & out) {
        out.hwPt = in.hwPt;
        out.hwPtErr = in.hwPtErr;
        out.hwEta = in.hwEta;
        out.hwPhi = in.hwPhi;
        out.src = nullptr;
    }
    inline void convert(const MuObj & in, l1tpf_impl::Muon & out) {
        out.hwPt = in.hwPt;
        out.hwEta = in.hwEta; // @calo
        out.hwPhi = in.hwPhi; // @calo
        out.src = nullptr;
    }


    template<unsigned int NMAX, typename In>
    void convert(const In in[NMAX], std::vector<l1tpf_impl::PFParticle> &out) {
        for (unsigned int i = 0; i < NMAX; ++i) {
            if (in[i].hwPt > 0) convert(in[i], out);
        }
    } 
    template<typename In>
    void convert(unsigned int NMAX, const In in[], std::vector<l1tpf_impl::PFParticle> &out) {
        for (unsigned int i = 0; i < NMAX; ++i) {
            if (in[i].hwPt > 0) convert(in[i], out);
        }
    } 
    inline void convert_puppi(unsigned int NMAX, const PuppiObj in[], std::vector<l1tpf_impl::PFParticle> &out) {
        for (unsigned int i = 0; i < NMAX; ++i) {
            if (in[i].hwPt > 0) convert_puppi(in[i], nullptr, out);
        }
    } 
    inline void convert_puppi(unsigned int NMAX, const PuppiObj in[], const std::vector<l1tpf_impl::PropagatedTrack> & srctracks, std::vector<l1tpf_impl::PFParticle> &out) {
        for (unsigned int i = 0; i < NMAX; ++i) {
            if (in[i].hwPt > 0) convert_puppi(in[i], &srctracks[i], out);
        }
    } 



    template<unsigned int NMAX>
    void convert(const PFChargedObj in[NMAX], const std::vector<l1tpf_impl::PropagatedTrack> & srctracks, std::vector<l1tpf_impl::PFParticle> &out) {
        for (unsigned int i = 0; i < NMAX; ++i) {
            if (in[i].hwPt > 0) {
                assert(i < srctracks.size());
                convert(in[i], srctracks[i], out);
            }
        }
    }
    inline void convert(unsigned int NMAX, const PFChargedObj in[], const std::vector<l1tpf_impl::PropagatedTrack> &srctracks, std::vector<l1tpf_impl::PFParticle> &out) {
        for (unsigned int i = 0; i < NMAX; ++i) {
            if (in[i].hwPt > 0) {
                assert(i < srctracks.size());
                convert(in[i], srctracks[i], out);
            }
        }
    }
 
} // namespace

#endif
