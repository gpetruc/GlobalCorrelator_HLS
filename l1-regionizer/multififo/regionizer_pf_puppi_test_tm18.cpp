#include "firmware/regionizer.h"
#include "multififo_regionizer_ref.h"
#include "../../utils/pattern_serializer.h"
#include "../../utils/test_utils.h"
#include "../../utils/DumpFileReader.h"
#include "firmware/dummy_obj_unpackers.h"
#include "utils/dummy_obj_packers.h"
#include "utils/tmux18_utils.h"
#include "tdemux/tdemux_ref.h"
#include "../../pf/ref/pfalgo2hgc_ref.h"
#include "../../pf/firmware/pfalgo2hgc.h"
#include "../../puppi/linpuppi_ref.h"
#include "../../puppi/firmware/linpuppi.h"
#include "../../common/bitonic_hybrid_sort_ref.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <memory>

#define TLEN REGIONIZERNCLOCKS 
#ifndef NTEST
#define NTEST 50
#endif

template<unsigned int NCHANN, unsigned int NBITS>
class Channels {
    public:
        Channels() { clear(); }
        Channels(const char *name) :
            serializer(new PatternSerializer(name, NCHANN*((NBITS+63)/64)))
        {
            clear(); 
        }
        void clear(bool isvalid=false) {
            for (unsigned int iclock = 0; iclock < NCHANN; ++iclock) {
                data[iclock] = 0; valid[iclock] = isvalid;
            }
        }
        unsigned int size() const { return NCHANN; }
        void dump() { serializer->packAndWrite(NCHANN, data, valid); }

        ap_uint<NBITS> data[NCHANN];
        bool valid[NCHANN];
    private:
        std::unique_ptr<PatternSerializer> serializer;
};

int main(int argc, char **argv) {

    DumpFileReader inputs("TTbar_PU200_HGCal.dump");

    const unsigned int nchann_in = NTKSECTORS*3 + 3*NCALOSECTORS*NCALOFIBERS + 3 + 1, nchann_vcu118 = 120;
    const unsigned int nchann_decoded = NTKSECTORS*NTKFIBERS + NCALOSECTORS*NCALOFIBERS + NMUFIBERS + 1;
    const unsigned int nchann_regionized = NTKOUT + NCALOOUT + NMUOUT + 1;
    const unsigned int nchann_pf = NTRACK + NCALO + NMU, nchann_puppi = NTRACK + NCALO, nchann_sort = NPUPPIFINALSORTED;
    const unsigned int tk_offs = 0, calo_offs = NTKSECTORS*3, mu_offs = calo_offs + NCALOSECTORS * NCALOFIBERS * 3, vtx_offs = mu_offs + 3;

    Channels<nchann_in,64> channelsTM("input-emp.txt"), channelsTDemux("input-emp-tdemux.txt");
    Channels<nchann_vcu118,64> channelsVCU118("input-emp-vcu118.txt");
    Channels<nchann_decoded,72> channelsDecode("input-emp-decoded.txt"), channelsIn("input-emp-decoded-ref.txt");
    Channels<nchann_regionized,72> channelsReg("output-emp-regionized-ref.txt");
    Channels<nchann_pf,72> channelsPf("output-emp-pf-ref.txt");
    Channels<nchann_puppi,64> channelsPuppi("output-emp-puppi-ref.txt");
    Channels<nchann_sort,64> channelsPuppiSort("output-emp-puppisort-ref.txt");

    ap_uint<72> decoded_validation_data[2*TLEN+1][nchann_decoded];
    bool        decoded_validation_valid[2*TLEN+1][nchann_decoded];
    unsigned int decoded_validation_index = 0;
    
    // TMux encoders
    TM18LinkMultiplet<ap_uint<64>,TLEN> tk_tmuxer(NTKSECTORS), calo_tmuxer(NCALOSECTORS*NCALOFIBERS), mu_tmuxer(1);
    // TMux decoders, for testing
    TDemuxRef tk_tdemuxer[NTKSECTORS], calo_tdemuxer[NCALOSECTORS][NCALOFIBERS], mu_tdemuxer;
    // make a delay queue for the PV to realign it to the first frame
    DelayQueue pv_delayer(TLEN*2+1); // latency of the TDemuxRef (measured from first valid frame in to first valid frame out)

    // make a version for the VCU118 kit with the proper link mapping
    // -- in the tmux vector, we have the first NTKSECTORS*3 mapped to TF, then 3*NCALOSECTORS*NCALOFIBERS, then 3 muons and 1 PV
    // -- in the VCU118:
    //          region 0 fibers 1-3 mapped to muons 
    //          region 0 fiber 4 mapped to PV
    //          regions 1-3 + 24-27 = 28 fibers mapped to tracker (3x9 fibers)
    //          regions 9-17 = 36 fibers mapped to HGCal
    std::vector<int> vcu118_links(nchann_vcu118, 0); // index is tmux link, value is VCU118 link
    vcu118_links[vtx_offs] = 3;
    for (int iclock =  0; iclock < 3; ++iclock) vcu118_links[mu_offs+iclock] = iclock;
    for (int iclock =  0; iclock < 3*NTKSECTORS; ++iclock) vcu118_links[tk_offs+iclock] = (iclock <= 11) ? (iclock + 4) : ((iclock-12) + 4*24);
    for (int iclock =  0; iclock < 3*NCALOSECTORS*NCALOFIBERS; ++iclock) vcu118_links[calo_offs+iclock] = iclock + 4*9;
    //for (int iclock =  0; iclock <= vtx_offs; ++iclock) {
    //    printf("Input link %3d mapped to VCU channel %3d\n", iclock, vcu118_links[iclock]);
    //}

    unsigned int frame = 0, ilink; 
    bool ok = true, first = true;
    const bool mux = ROUTER_ISMUX, stream = ROUTER_ISSTREAM;
    const unsigned int regii = (stream ? 4 : 6), pfii = 6;
    l1ct::MultififoRegionizerEmulator regEmulator(/*nendcaps=*/1, REGIONIZERNCLOCKS, NTRACK, NCALO, /*NEM=*/0, NMU, stream, regii);
    l1ct::PFAlgo2HGCEmulator pfEmulator(NTRACK, NCALO, NMU, NCALO,
                        PFALGO_DR2MAX_TK_MU, PFALGO_DR2MAX_TK_CALO,
                        l1ct::Scales::makePt(PFALGO_TK_MAXINVPT_LOOSE), l1ct::Scales::makePt(PFALGO_TK_MAXINVPT_TIGHT));
    const float ptErr_edges[PTERR_BINS]  = PTERR_EDGES;
    const float ptErr_offss[PTERR_BINS]  = PTERR_OFFS;
    const float ptErr_scales[PTERR_BINS] = PTERR_SCALE;
    pfEmulator.loadPtErrBins(PTERR_BINS, ptErr_edges, ptErr_scales, ptErr_offss);   
    l1ct::LinPuppiEmulator puEmulator(NTRACK, NALLNEUTRALS, NALLNEUTRALS,
                          LINPUPPI_DR2MIN, LINPUPPI_DR2MAX, LINPUPPI_ptMax, LINPUPPI_dzCut,
                          l1ct::Scales::makeGlbEta(LINPUPPI_etaCut), 
                          LINPUPPI_ptSlopeNe, LINPUPPI_ptSlopeNe_1, LINPUPPI_ptSlopePh, LINPUPPI_ptSlopePh_1, 
                          LINPUPPI_ptZeroNe, LINPUPPI_ptZeroNe_1, LINPUPPI_ptZeroPh, LINPUPPI_ptZeroPh_1, 
                          LINPUPPI_alphaSlope, LINPUPPI_alphaSlope_1, LINPUPPI_alphaZero, LINPUPPI_alphaZero_1, LINPUPPI_alphaCrop, LINPUPPI_alphaCrop_1, 
                          LINPUPPI_priorNe, LINPUPPI_priorNe_1, LINPUPPI_priorPh, LINPUPPI_priorPh_1,
                          l1ct::Scales::makePt(LINPUPPI_ptCut), l1ct::Scales::makePt(LINPUPPI_ptCut_1));

    l1ct::PVObjEmu pv_prev; // we have 1 event of delay in the reference regionizer, so we need to use the PV from 54 clocks before
    for (int itest = 0; itest < NTEST; ++itest) {
        if (!inputs.nextEvent()) break;
        const auto & decodedObjs = inputs.event().decoded;

        // now we make a single endcap setup
        l1ct::RegionizerDecodedInputs in; std::vector<l1ct::PFInputRegion> allpfin;
        for (auto & sec : inputs.event().decoded.track) if (sec.region.floatEtaCenter() >= 0) in.track.push_back(sec);
        for (auto & sec : inputs.event().decoded.hadcalo) if (sec.region.floatEtaCenter() >= 0) in.hadcalo.push_back(sec);
        for (auto & sec : inputs.event().decoded.emcalo) if (sec.region.floatEtaCenter() >= 0) in.emcalo.push_back(sec);
        in.muon = inputs.event().decoded.muon;
        for (auto & reg : inputs.event().pfinputs) if (reg.region.floatEtaCenter() >= 0) allpfin.push_back(reg);
        const l1ct::glbeta_t etaCenter = l1ct::Scales::makeGlbEta(2.0);
        l1ct::PFInputRegion pfin;
        l1ct::OutputRegion pfout;

        if (first) { regEmulator.initSectorsAndRegions(in, allpfin); first = false; }

        // enqueue frames (outside of the frame loop, since it takes 3*TLEN and not TLEN)
        tk_tmuxer.push_links(itest, in.track, pack_tracks);
        calo_tmuxer.push_links(itest, in.hadcalo, pack_hgcal);
        mu_tmuxer.push_link(itest, in.muon, pack_muons);

        //if (itest == 0) printf("Vertexis at z0 = %d\n", inputs.event().pv().hwZ0.to_int());

        for (int iclock = 0; iclock < TLEN; ++iclock, ++frame) {
            // pop out frames from the tmuxer for printing. for each sector, we put the 3 links for 3 set of events next to each other
            const unsigned int tk_offs = 0, calo_offs = NTKSECTORS*3, mu_offs = calo_offs + NCALOSECTORS * NCALOFIBERS * 3, vtx_offs = mu_offs + 3;
            tk_tmuxer.pop_frame(channelsTM, tk_offs); 
            calo_tmuxer.pop_frame(channelsTM, calo_offs);
            mu_tmuxer.pop_frame(channelsTM, mu_offs);
            // the vertex is not TMUXed so we just add it at the end
            channelsTM.data[vtx_offs]  = inputs.event().pv(iclock).pack();
            channelsTM.valid[vtx_offs] = (iclock < TLEN-1);
            channelsTM.dump();
            
            // make also version with vcu118 link mapping
            for (ilink = 0; ilink <= vtx_offs; ++ilink) {
                channelsVCU118.data[vcu118_links[ilink]] = channelsTM.data[ilink];
                channelsVCU118.valid[vcu118_links[ilink]] = channelsTM.valid[ilink];
            }
            channelsVCU118.dump();

            // now let's run the time demultiplexer
            bool newEvt = (iclock == 0 && itest == 0);
            for (int s = 0; s < NTKSECTORS; ++s) {
                tk_tdemuxer[s]( newEvt, &channelsTM.data  [3*s], &channelsTM.valid  [3*s],
                                        &channelsTDemux.data[3*s], &channelsTDemux.valid[3*s]);
            }
            for (int s = 0; s < NCALOSECTORS; ++s) {
                for (int f = 0; f < NCALOFIBERS; ++f) {
                    ilink = calo_offs + 3*(s*NCALOFIBERS + f);
                    calo_tdemuxer[s][f]( newEvt, &channelsTM.data  [ilink], &channelsTM.valid  [ilink],
                                                 &channelsTDemux.data[ilink], &channelsTDemux.valid[ilink]);
                }
            }
            mu_tdemuxer(newEvt, &channelsTM.data  [mu_offs], &channelsTM.valid  [mu_offs],
                                &channelsTDemux.data[mu_offs], &channelsTDemux.valid[mu_offs]);
            // note: the PV is delayed to realign it to the other demuxed channels
            pv_delayer(channelsTM.data    [vtx_offs], channelsTM.valid    [vtx_offs],
                       channelsTDemux.data[vtx_offs], channelsTDemux.valid[vtx_offs]);
            if (frame > 2*TLEN) channelsTDemux.dump();

            // and now we unpack to 64 bit format
            ilink = 0; unsigned int iout = 0;
            for (int s = 0; s < NTKSECTORS; ++s) {
                unpack_track_3to2(channelsTDemux.data[ilink+0], channelsTDemux.valid[ilink+0],
                                  channelsTDemux.data[ilink+1], channelsTDemux.valid[ilink+1],
                                  channelsTDemux.data[ilink+2], channelsTDemux.valid[ilink+2],
                                  channelsDecode.data[iout+0], channelsDecode.valid[iout+0],
                                  channelsDecode.data[iout+1], channelsDecode.valid[iout+1]);
                ilink += 3; iout += 2;
            }
            for (int s = 0; s < NCALOSECTORS; ++s) {
                for (int f = 0; f < NCALOFIBERS; ++f) {
                    unpack_hgcal_3to1(channelsTDemux.data[ilink+0], channelsTDemux.valid[ilink+0],
                                      channelsTDemux.data[ilink+1], channelsTDemux.valid[ilink+1],
                                      channelsTDemux.data[ilink+2], channelsTDemux.valid[ilink+2],
                                      channelsDecode.data[iout+0], channelsDecode.valid[iout+0]);
                    ilink += 3; iout += 1;
                }
            }
            unpack_mu_3to12(channelsTDemux.data[ilink+0], channelsTDemux.valid[ilink+0],
                            channelsTDemux.data[ilink+1], channelsTDemux.valid[ilink+1],
                            channelsTDemux.data[ilink+2], channelsTDemux.valid[ilink+2],
                            channelsDecode.data[iout+0], channelsDecode.valid[iout+0],
                            channelsDecode.data[iout+1], channelsDecode.valid[iout+1]); 
            if (channelsDecode.valid[iout+0]) channelsDecode.valid[iout+1] = 1; // for our purposes, mark both valid if the first is valid
            ilink += 3; iout += 2;
            // the vertex is trivial
            channelsDecode.data[iout]  = channelsTDemux.data [ilink];
            channelsDecode.valid[iout] = channelsTDemux.valid[ilink];
            if (frame > 2*TLEN) {
                channelsDecode.dump();
                // validation
                unsigned int ref_index = (decoded_validation_index + 0) % (2*TLEN+1);
                for (unsigned int i = 0; i < nchann_decoded; ++i) {
                    if (decoded_validation_data[ref_index][i] != channelsDecode.data[i] ||
                        decoded_validation_valid[ref_index][i] != channelsDecode.valid[i]) {
                        if (ok) printf("Mismatch in decoded validation, frame %d, ref_index %u:\n", frame, ref_index); 
                        printf("channel %3u: ref %dv %20s vs emu %dv %20s\n", i, 
                                        int(decoded_validation_valid[ref_index][i]), decoded_validation_data[ref_index][i].to_string(16).c_str(),
                                        int(channelsDecode.valid[i]), channelsDecode.data[i].to_string(16).c_str());
                        ok = false;
                    }
                }
                if (!ok) break;
            }
            // done unpacking

            // emulate regionizer
            std::vector<l1ct::TkObjEmu> tk_links_in, tk_out;
            std::vector<l1ct::EmCaloObjEmu> em_links_in, em_out; // not used but needed by interface
            std::vector<l1ct::HadCaloObjEmu> calo_links_in, calo_out;
            std::vector<l1ct::MuObjEmu> mu_links_in, mu_out;

            regEmulator.fillLinks(iclock, in, tk_links_in);
            regEmulator.fillLinks(iclock, in, calo_links_in);
            regEmulator.fillLinks(iclock, in, mu_links_in);

            ilink = 0; channelsIn.clear(/*valid=*/(iclock < TLEN-1));
            for (int itk = 0; itk < NTKSECTORS*NTKFIBERS; ++itk) 
                channelsIn.data[ilink++] = tk_links_in[itk].pack(); 
            for (int icalo = 0; icalo < NCALOSECTORS*NCALOFIBERS; ++icalo) 
                channelsIn.data[ilink++] = calo_links_in[icalo].pack(); 
            for (int imu = 0; imu < NMUFIBERS; ++imu) 
                channelsIn.data[ilink++] = mu_links_in[imu].pack();
            channelsIn.data[ilink++] = inputs.event().pv(iclock).pack();
            channelsIn.dump();

            // put good decoded data for validation in the ring buffer
            for (unsigned int i = 0; i < nchann_decoded; ++i) {
                 decoded_validation_data[decoded_validation_index][i] = channelsIn.data[i];
                 decoded_validation_valid[decoded_validation_index][i] = channelsIn.valid[i];
            }
            decoded_validation_index = (decoded_validation_index + 1) % (2*TLEN+1);

            bool newevt_ref = (iclock == 0);
            regEmulator.step(newevt_ref, tk_links_in, tk_out, mux);
            regEmulator.step(newevt_ref, calo_links_in, calo_out, mux);
            regEmulator.step(newevt_ref, mu_links_in, mu_out, mux);

            ilink = 0; channelsReg.clear(true);
            for (int i = 0; i < NTKOUT; ++i) 
                channelsReg.data[ilink++] = tk_out[i].pack(); 
            for (int i = 0; i < NCALOOUT; ++i) 
                channelsReg.data[ilink++] = calo_out[i].pack(); 
            for (int i = 0; i < NMUOUT; ++i) 
                channelsReg.data[ilink++] = mu_out[i].pack();
            unsigned int ireg = iclock/regii; bool region_valid = ireg < allpfin.size();
            channelsReg.data[ilink++] = region_valid ? allpfin[ireg].region.pack() : ap_uint<l1ct::PFRegion::BITWIDTH>(0);

            if (itest > 0 && region_valid) {
                regEmulator.destream(iclock, tk_out, em_out, calo_out, mu_out, allpfin[ireg]);
            }
            if (itest > 0 && (iclock % pfii == pfii - 1) && (iclock / pfii < NPFREGIONS)) {
                ireg = (iclock/pfii);
                pfin = allpfin[ireg];
                if (itest <= 5) printf("Will run PF event %d, region %d\n", itest-1, ireg);
                pfEmulator.setDebug(itest <= 5);
                pfEmulator.run(pfin, pfout);
                pfEmulator.mergeNeutrals(pfout);

                channelsPf.clear(true);
                for (int i = 0, n = pfout.pfcharged.size(), ilink = 0; i < n; ++i, ++ilink)
                    channelsPf.data[ilink] = pfout.pfcharged[i].pack();
                for (int i = 0, n = pfout.pfneutral.size(), ilink = NTRACK; i < n; ++i, ++ilink)
                    channelsPf.data[ilink] = pfout.pfneutral[i].pack();
                for (int i = 0, n = pfout.pfmuon.size(), ilink = NTRACK+NCALO; i < n; ++i, ++ilink)
                    channelsPf.data[ilink] = pfout.pfmuon[i].pack();

                // Puppi objects
                if (itest <= 5) printf("Will run Puppi with z0 = %d in event %d, region %d\n", pv_prev.hwZ0.to_int(), itest-1, ireg);
                puEmulator.setDebug(itest <= 5);

                std::vector<l1ct::PuppiObjEmu> outallch, outallne_nocut, outallne, outselne;
                puEmulator.linpuppi_chs_ref(pfin.region, pv_prev, pfout.pfcharged, outallch);
                puEmulator.linpuppi_ref(pfin.region, pfin.track, pv_prev, pfout.pfneutral, outallne_nocut, outallne, outselne);

                outallch.resize(NTRACK);
                outallne.resize(NCALO);
                outallch.insert(outallch.end(), outallne.begin(), outallne.end());
                ilink = 0; channelsPuppi.clear(true);
                for (auto & pup : outallch) channelsPuppi.data[ilink++] = pup.pack();

                pfout.puppi.resize(NPUPPIFINALSORTED);
                hybrid_bitonic_sort_and_crop_ref(NTRACK+NCALO, NPUPPIFINALSORTED, &outallch[0], &pfout.puppi[0]);

                ilink = 0; channelsPuppiSort.clear(true);
                for (auto & pup : pfout.puppi) channelsPuppiSort.data[ilink++] = pup.pack();
            }

                
            if (itest > 0) channelsReg.dump(); // avoid dumping frames of zeros
            if (frame >= TLEN + pfii - 1) {  // avoid dumping frames of zeros
                channelsPf.dump();
                channelsPuppi.dump();
                channelsPuppiSort.dump();
            }

            if (iclock == TLEN-1) pv_prev = inputs.event().pv();
        }
        if (!ok) break;
    } 

    return ok ? 0 : 1;
}
