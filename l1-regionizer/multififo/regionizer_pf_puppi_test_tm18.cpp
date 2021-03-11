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

#define PAUSES

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

        void dumpNulls(unsigned int n, bool valid=false) {
            clear(valid);
            for (unsigned int i = 0; i < n; ++i) dump();
        }

        ap_uint<NBITS> data[NCHANN];
        bool valid[NCHANN];
    private:
        std::unique_ptr<PatternSerializer> serializer;
};

struct Tester {

    Tester(const std::string & inputFile) :
        inputs("TTbar_PU200_HGCal.dump"),
        channelsTM("input-emp.txt"), 
        channelsTDemux("input-emp-tdemux.txt"),
        channelsVCU118("input-emp-vcu118.txt"),
        channelsDecode("input-emp-decoded.txt"), 
        channelsIn("input-emp-decoded-ref.txt"),
        channelsReg("output-emp-regionized-ref.txt"),
        channelsPf("output-emp-pf-ref.txt"),
        channelsPuppi("output-emp-puppi-ref.txt"),
        channelsPuppiSort("output-emp-puppisort-ref.txt"),
        vcu118_links(nchann_vcu118, 0), // index is tmux link, value is VCU118 link
        decoded_validation_index(0),
        tk_tmuxer(NTKSECTORS), calo_tmuxer(NCALOSECTORS*NCALOFIBERS), mu_tmuxer(1),
        pv_delayer(TLEN*2+1),
        regEmulator (/*nendcaps=*/1, REGIONIZERNCLOCKS, NTRACK, NCALO, /*NEM=*/0, NMU, ROUTER_ISSTREAM, ROUTER_ISSTREAM ? 4 : 6),
        regEmulator2(/*nendcaps=*/1, REGIONIZERNCLOCKS, NTRACK, NCALO, /*NEM=*/0, NMU, ROUTER_ISSTREAM, ROUTER_ISSTREAM ? 4 : 6),
        pfEmulator(NTRACK, NCALO, NMU, NCALO,
                        PFALGO_DR2MAX_TK_MU, PFALGO_DR2MAX_TK_CALO,
                        PFALGO_TK_MAXINVPT_LOOSE, PFALGO_TK_MAXINVPT_TIGHT),
        puEmulator(NTRACK, NALLNEUTRALS, NALLNEUTRALS,
                          LINPUPPI_DR2MIN, LINPUPPI_DR2MAX, LINPUPPI_iptMax, LINPUPPI_dzCut,
                          l1ct::Scales::makeGlbEta(LINPUPPI_etaCut), 
                          LINPUPPI_ptSlopeNe, LINPUPPI_ptSlopeNe_1, LINPUPPI_ptSlopePh, LINPUPPI_ptSlopePh_1, 
                          LINPUPPI_ptZeroNe, LINPUPPI_ptZeroNe_1, LINPUPPI_ptZeroPh, LINPUPPI_ptZeroPh_1, 
                          LINPUPPI_alphaSlope, LINPUPPI_alphaSlope_1, LINPUPPI_alphaZero, LINPUPPI_alphaZero_1, LINPUPPI_alphaCrop, LINPUPPI_alphaCrop_1, 
                          LINPUPPI_priorNe, LINPUPPI_priorNe_1, LINPUPPI_priorPh, LINPUPPI_priorPh_1,
                          LINPUPPI_ptCut, LINPUPPI_ptCut_1),
        regInit(false), 
        frame(0)
    { 
        initVCU118Links();
        const float ptErr_edges[PTERR_BINS]  = PTERR_EDGES;
        const float ptErr_offss[PTERR_BINS]  = PTERR_OFFS;
        const float ptErr_scales[PTERR_BINS] = PTERR_SCALE;
        pfEmulator.loadPtErrBins(PTERR_BINS, ptErr_edges, ptErr_scales, ptErr_offss);   

    }
        

    static const unsigned int nchann_in = NTKSECTORS*3 + 3*NCALOSECTORS*NCALOFIBERS + 3 + 1, nchann_vcu118 = 120;
    static const unsigned int nchann_decoded = NTKSECTORS*NTKFIBERS + NCALOSECTORS*NCALOFIBERS + NMUFIBERS + 1;
    static const unsigned int nchann_regionized = NTKOUT + NCALOOUT + NMUOUT + 1;
    static const unsigned int nchann_pf = NTRACK + NCALO + NMU, nchann_puppi = NTRACK + NCALO, nchann_sort = NPUPPIFINALSORTED;
    static const unsigned int tk_offs = 0, calo_offs = NTKSECTORS*3, mu_offs = calo_offs + NCALOSECTORS * NCALOFIBERS * 3, vtx_offs = mu_offs + 3;

    DumpFileReader inputs;
    Channels<nchann_in,64> channelsTM, channelsTDemux;
    Channels<nchann_vcu118,64> channelsVCU118;
    Channels<nchann_decoded,72> channelsDecode, channelsIn;
    Channels<nchann_regionized,72> channelsReg;
    Channels<nchann_pf,72> channelsPf;
    Channels<nchann_puppi,64> channelsPuppi;
    Channels<nchann_sort,64> channelsPuppiSort;

    std::vector<int> vcu118_links; // index is tmux link, value is VCU118 link

    ap_uint<72> decoded_validation_data[3*TLEN][nchann_decoded];
    bool        decoded_validation_valid[3*TLEN][nchann_decoded];
    unsigned int decoded_validation_index;
    
    // TMux encoders
    TM18LinkMultiplet<ap_uint<64>,TLEN> tk_tmuxer, calo_tmuxer, mu_tmuxer;
    // TMux decoders, for testing
    TDemuxRef tk_tdemuxer[NTKSECTORS], calo_tdemuxer[NCALOSECTORS][NCALOFIBERS], mu_tdemuxer;
    // make a delay queue for the PV to realign it to the first frame
    DelayQueue pv_delayer; // latency of the TDemuxRef (measured from first valid frame in to first valid frame out)

    l1ct::MultififoRegionizerEmulator regEmulator; // for clock-by-clock outputs
    l1ct::MultififoRegionizerEmulator regEmulator2; // run all in once
    l1ct::PFAlgo2HGCEmulator pfEmulator;
    l1ct::LinPuppiEmulator puEmulator;

    bool regInit;
    unsigned int frame;

    void initVCU118Links();
    void readOneEndcap(int itest, l1ct::RegionizerDecodedInputs & in, std::vector<l1ct::PFInputRegion> & allpfin) ;
    bool runTMuxAndDemux(int itest, int indexWithinTrain, int nclocks, bool tailOfTrain)  ;
    void runRegionizer(const l1ct::RegionizerDecodedInputs & in, const std::vector<l1ct::PFInputRegion> & allpfin, unsigned int nclocks, bool firstEventOfTrain, bool tailOfTrain) ;
    void runPFPuppi(int itest, const std::vector<l1ct::PFInputRegion> & allpfin, bool debug) ;
    bool run() ;
};

void Tester::initVCU118Links() 
{
    // make a version for the VCU118 kit with the proper link mapping
    // -- in the tmux vector, we have the first NTKSECTORS*3 mapped to TF, then 3*NCALOSECTORS*NCALOFIBERS, then 3 muons and 1 PV
    // -- in the VCU118:
    //          region 0 fibers 1-3 mapped to muons 
    //          region 0 fiber 4 mapped to PV
    //          regions 1-3 + 24-27 = 28 fibers mapped to tracker (3x9 fibers)
    //          regions 9-17 = 36 fibers mapped to HGCal
    vcu118_links[vtx_offs] = 3;
    for (int iclock =  0; iclock < 3; ++iclock) vcu118_links[mu_offs+iclock] = iclock;
    for (int iclock =  0; iclock < 3*NTKSECTORS; ++iclock) vcu118_links[tk_offs+iclock] = (iclock <= 11) ? (iclock + 4) : ((iclock-12) + 4*24);
    for (int iclock =  0; iclock < 3*NCALOSECTORS*NCALOFIBERS; ++iclock) vcu118_links[calo_offs+iclock] = iclock + 4*9;
    //for (int iclock =  0; iclock <= vtx_offs; ++iclock) {
    //    printf("Input link %3d mapped to VCU channel %3d\n", iclock, vcu118_links[iclock]);
    //}
}

void Tester::readOneEndcap(int itest, l1ct::RegionizerDecodedInputs & in, std::vector<l1ct::PFInputRegion> & allpfin)
{
    in.clear(); allpfin.clear();
    for (auto & sec : inputs.event().decoded.track) if (sec.region.floatEtaCenter() >= 0) in.track.push_back(sec);
    for (auto & sec : inputs.event().decoded.hadcalo) if (sec.region.floatEtaCenter() >= 0) in.hadcalo.push_back(sec);
    for (auto & sec : inputs.event().decoded.emcalo) if (sec.region.floatEtaCenter() >= 0) in.emcalo.push_back(sec);
    in.muon = inputs.event().decoded.muon;
    for (auto & reg : inputs.event().pfinputs) if (reg.region.floatEtaCenter() >= 0) allpfin.push_back(reg);

    if (!regInit) { regEmulator.initSectorsAndRegions(in, allpfin); regInit = true; } // this will do clock-cycle emulation
    regEmulator2.run(in, allpfin); // this also but just internally, from the outside it will seem it took no time

    tk_tmuxer.push_links(itest, in.track, pack_tracks);
    calo_tmuxer.push_links(itest, in.hadcalo, pack_hgcal);
    mu_tmuxer.push_link(itest, in.muon, pack_muons);
}

bool Tester::runTMuxAndDemux(int itest, int indexWithinTrain, int nclocks, bool tailOfTrain) 
{
    bool ok = true;
    int ilink;
    for (int iclock = 0; iclock < nclocks; ++iclock, ++frame) {
        // pop out frames from the tmuxer for printing. for each sector, we put the 3 links for 3 set of events next to each other
        tk_tmuxer.pop_frame(channelsTM, tk_offs, true, tailOfTrain); 
        calo_tmuxer.pop_frame(channelsTM, calo_offs, true, tailOfTrain);
        mu_tmuxer.pop_frame(channelsTM, mu_offs, true, tailOfTrain);
        // the vertex is not TMUXed so we just add it at the end
        if (!tailOfTrain) {
            channelsTM.data[vtx_offs]  = inputs.event().pv(iclock).pack();
            channelsTM.valid[vtx_offs] = (iclock < TLEN-1);
        } else {
            channelsTM.data[vtx_offs]  = 0;
            channelsTM.valid[vtx_offs] = false;
        }
        channelsTM.dump();

        // make also version with vcu118 link mapping
        for (ilink = 0; ilink <= vtx_offs; ++ilink) {
            channelsVCU118.data[vcu118_links[ilink]] = channelsTM.data[ilink];
            channelsVCU118.valid[vcu118_links[ilink]] = channelsTM.valid[ilink];
        }
        channelsVCU118.dump();

        // now let's run the time demultiplexer
        bool newEvt = (iclock == 0 && indexWithinTrain == 0 && !tailOfTrain);
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

        if (indexWithinTrain < 2 || (indexWithinTrain == 2 && iclock == 0)) continue; // skip null frames
        
        channelsTDemux.dump();

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
        channelsDecode.dump();

        if (tailOfTrain && iclock > 2*TLEN) continue; // nothing to validate againts

        // validation
        unsigned int ref_index = (decoded_validation_index + iclock + 3*TLEN - 1) % (3*TLEN);
        for (unsigned int i = 0; i < nchann_decoded; ++i) {
            if (decoded_validation_data[ref_index][i] != channelsDecode.data[i] ||
                    decoded_validation_valid[ref_index][i] != channelsDecode.valid[i]) {
                if (ok) printf("Mismatch in decoded validation, itest %d indexWithinTrain %d iclock %d, frame %d, dvi %u, ref_index %u, tail %d:\n", itest, indexWithinTrain, iclock, frame, decoded_validation_index, ref_index, int(tailOfTrain)); 
                printf("channel %3u: ref %dv %20s vs emu %dv %20s\n", i, 
                        int(decoded_validation_valid[ref_index][i]), decoded_validation_data[ref_index][i].to_string(16).c_str(),
                        int(channelsDecode.valid[i]), channelsDecode.data[i].to_string(16).c_str());
                ok = false;
            }
        }
        //if (!ok) break;
    }
    return ok;

}

void Tester::runRegionizer(const l1ct::RegionizerDecodedInputs & in, const std::vector<l1ct::PFInputRegion> & allpfin, unsigned int nclocks, bool firstEventOfTrain, bool tailOfTrain) 
{
    unsigned int regii = ROUTER_ISSTREAM ? 4 : 6;
    int ilink;
    for (int iclock = 0; iclock < nclocks; ++iclock) {
        // emulate regionizer
        std::vector<l1ct::TkObjEmu> tk_links_in, tk_out;
        std::vector<l1ct::EmCaloObjEmu> em_links_in, em_out; // not used but needed by interface
        std::vector<l1ct::HadCaloObjEmu> calo_links_in, calo_out;
        std::vector<l1ct::MuObjEmu> mu_links_in, mu_out;

        if (!tailOfTrain) {
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
            decoded_validation_index = (decoded_validation_index + 1) % (3*TLEN);
        } else {
            tk_links_in.resize(NTRACK);
            calo_links_in.resize(NCALO);
            mu_links_in.resize(NMU);
            for (auto & t : tk_links_in) t.clear();
            for (auto & c : calo_links_in) c.clear();
            for (auto & m : mu_links_in) m.clear();
        }

        bool newevt_ref = (iclock == 0);
        regEmulator.step(newevt_ref, tk_links_in, tk_out, ROUTER_ISMUX);
        regEmulator.step(newevt_ref, calo_links_in, calo_out, ROUTER_ISMUX);
        regEmulator.step(newevt_ref, mu_links_in, mu_out, ROUTER_ISMUX);

        ilink = 0; channelsReg.clear(true);
        for (int i = 0; i < NTKOUT; ++i) 
            channelsReg.data[ilink++] = tk_out[i].pack(); 
        for (int i = 0; i < NCALOOUT; ++i) 
            channelsReg.data[ilink++] = calo_out[i].pack(); 
        for (int i = 0; i < NMUOUT; ++i) 
            channelsReg.data[ilink++] = mu_out[i].pack();
        unsigned int ireg = iclock/regii; 
        bool region_valid = ireg < allpfin.size();
        channelsReg.data[ilink++] = region_valid ? allpfin[ireg].region.pack() : ap_uint<l1ct::PFRegion::BITWIDTH>(0);

        if (!firstEventOfTrain) {
            channelsReg.dump();
        }
    }
}

void Tester::runPFPuppi(int itest, const std::vector<l1ct::PFInputRegion> & allpfin, bool debug) 
{

    l1ct::OutputRegion pfout;
    int ilink;

    for (int ireg = 0; ireg < NPFREGIONS; ++ireg) {
        if (debug) printf("Will run PF event %d, region %d\n", itest, ireg);
        pfEmulator.setDebug(debug);
        pfEmulator.run(allpfin[ireg], pfout);
        pfEmulator.mergeNeutrals(pfout);

        channelsPf.clear(true);
        for (int i = 0, n = pfout.pfcharged.size(), ilink = 0; i < n; ++i, ++ilink)
            channelsPf.data[ilink] = pfout.pfcharged[i].pack();
        for (int i = 0, n = pfout.pfneutral.size(), ilink = NTRACK; i < n; ++i, ++ilink)
            channelsPf.data[ilink] = pfout.pfneutral[i].pack();
        for (int i = 0, n = pfout.pfmuon.size(), ilink = NTRACK+NCALO; i < n; ++i, ++ilink)
            channelsPf.data[ilink] = pfout.pfmuon[i].pack();

        // Puppi objects
        if (debug) printf("Will run Puppi with z0 = %d in event %d, region %d\n", inputs.event().pv().hwZ0.to_int(), itest, ireg);
        puEmulator.setDebug(debug);

        std::vector<l1ct::PuppiObjEmu> outallch, outallne_nocut, outallne, outselne;
        puEmulator.linpuppi_chs_ref(allpfin[ireg].region, inputs.event().pv(), pfout.pfcharged, outallch);
        puEmulator.linpuppi_ref(allpfin[ireg].region, allpfin[ireg].track, inputs.event().pv(), pfout.pfneutral, outallne_nocut, outallne, outselne);

        outallch.resize(NTRACK);
        outallne.resize(NCALO);
        outallch.insert(outallch.end(), outallne.begin(), outallne.end());
        ilink = 0; channelsPuppi.clear(true);
        for (auto & pup : outallch) channelsPuppi.data[ilink++] = pup.pack();

        pfout.puppi.resize(NPUPPIFINALSORTED);
        hybrid_bitonic_sort_and_crop_ref(NTRACK+NCALO, NPUPPIFINALSORTED, &outallch[0], &pfout.puppi[0]);

        ilink = 0; channelsPuppiSort.clear(true);
        for (auto & pup : pfout.puppi) channelsPuppiSort.data[ilink++] = pup.pack();

        for (int j = 0; j < 6; ++j) {
            channelsPf.dump();
            channelsPuppi.dump();
            channelsPuppiSort.dump();
        }
    }
}

bool Tester::run() {
    unsigned int frame = 0, ilink; 
    bool ok = true, firstOfTrain = true;
    l1ct::PVObjEmu pv_prev; // we have 1 event of delay in the reference regionizer, so we need to use the PV from 54 clocks before

    for (int itest = 0, trainStart = 0; itest < NTEST; ++itest) {
        if (!inputs.nextEvent()) break;
        printf("Processing event %d (train Start %d, index in train %d)\n", itest, trainStart, itest-trainStart);

        // now we make a single endcap setup
        l1ct::RegionizerDecodedInputs in; std::vector<l1ct::PFInputRegion> allpfin;
        readOneEndcap(itest - trainStart, in, allpfin);

        runRegionizer(in, allpfin, TLEN, itest == trainStart, /*tail=*/false);
        runPFPuppi(itest, allpfin, itest <= 0);
        
        ok = runTMuxAndDemux(itest, itest - trainStart, TLEN, /*tail=*/false);

        #ifdef PAUSES
        if (itest - trainStart == 5) { // put a pause after 6 events
            unsigned int pause_length = 200;
            // flush data: regionizer
            runRegionizer(in, allpfin, TLEN, /*start=*/false, /*tail=*/true);
            // flush data and add pause; tdemux and inputs
            runTMuxAndDemux(itest + 1, itest + 1 - trainStart, pause_length, /*tail=*/true);
            // add pause in the expected outputs
            channelsIn.dumpNulls(pause_length);
            channelsReg.dumpNulls(pause_length);
            channelsPf.dumpNulls(pause_length);
            channelsPuppi.dumpNulls(pause_length);
            channelsPuppiSort.dumpNulls(pause_length);
            // re-init muxers
            tk_tmuxer.init(); calo_tmuxer.init(); mu_tmuxer.init(); 
            decoded_validation_index = 0; frame = 0;
            trainStart = itest + 1;
        }
        #endif
        if (!ok) break;
    } 

    return ok;
}

int main(int argc, char **argv) {
    Tester test("TTbar_PU200_HGCal.dump");
    return test.run() ? 0 : 1;
}

