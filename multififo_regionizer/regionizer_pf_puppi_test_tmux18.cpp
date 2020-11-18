#include "firmware/regionizer.h"
//typedef ap_uint<64> PackedTkObj;
//typedef ap_uint<64> PackedCaloObj;
//typedef ap_uint<64> PackedMuObj;
#include "../utils/pattern_serializer.h"
#include "../utils/pattern_multiplexer.h"
#include "../utils/test_utils.h"

#include "../ref/pfalgo2hgc_ref.h"
#include "../puppi/linpuppi_ref.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <queue>

#define TLEN REGIONIZERNCLOCKS 

bool tk_router_ref(bool newevent, const TkObj tracks_in[NTKSECTORS][NTKFIBERS], TkObj tracks_out[NTKOUT]) ;
bool calo_router_ref(bool newevent, const HadCaloObj calo_in[NCALOSECTORS][NCALOFIBERS], HadCaloObj calo_out[NCALOOUT]) ;
bool mu_router_ref(bool newevent, const glbeta_t etaCenter, const GlbMuObj mu_in[NMUFIBERS], MuObj mu_out[NMUOUT]) ;
bool readEventTkTM18(FILE *file, std::vector<TkObj> inputs[NTKSECTORS], uint32_t &run, uint32_t &lumi, uint64_t &event) ;
bool readEventCalo(FILE *file, std::vector<HadCaloObj> inputs[NCALOSECTORS][NCALOFIBERS], bool zside, uint32_t &run, uint32_t &lumi, uint64_t &event) ;
bool readEventMuTM18(FILE *file, std::vector<GlbMuObj> inputs, uint32_t &run, uint32_t &lumi, uint64_t &event) ;
bool readEventVtx(FILE *file, std::vector<std::pair<z0_t,pt_t>> & inputs, uint32_t &irun, uint32_t &ilumi, uint64_t &ievent) ;


int main(int argc, char **argv) {
    // initialization of components
    pfalgo_config pfcfg(NTRACK,NCALO,NMU, NSELCALO,
                        PFALGO_DR2MAX_TK_MU, PFALGO_DR2MAX_TK_CALO,
                        PFALGO_TK_MAXINVPT_LOOSE, PFALGO_TK_MAXINVPT_TIGHT);
    linpuppi_config pucfg(NTRACK, NALLNEUTRALS, NNEUTRALS,
                          LINPUPPI_DR2MIN, LINPUPPI_DR2MAX, LINPUPPI_ptMax, LINPUPPI_dzCut,
                          LINPUPPI_etaCut, LINPUPPI_invertEta,
                          LINPUPPI_ptSlopeNe, LINPUPPI_ptSlopeNe_1, LINPUPPI_ptSlopePh, LINPUPPI_ptSlopePh_1, 
                          LINPUPPI_ptZeroNe, LINPUPPI_ptZeroNe_1, LINPUPPI_ptZeroPh, LINPUPPI_ptZeroPh_1, 
                          LINPUPPI_alphaSlope, LINPUPPI_alphaSlope_1, LINPUPPI_alphaZero, LINPUPPI_alphaZero_1, LINPUPPI_alphaCrop, LINPUPPI_alphaCrop_1, 
                          LINPUPPI_priorNe, LINPUPPI_priorNe_1, LINPUPPI_priorPh, LINPUPPI_priorPh_1,
                          LINPUPPI_ptCut, LINPUPPI_ptCut_1);

    // input file initialization -- event based 
    FILE *fMC_calo  = fopen("caloDump_hgcal.TTbar_PU200.txt", "r");
    FILE *fMC_tk  = fopen("trackDump_hgcalPos.TTbar_PU200.txt", "r");
    FILE *fMC_mu  = fopen("muonDump_all.TTbar_PU200.txt", "r");
    FILE *fMC_vtx = fopen("vertexDump_all.TTbar_PU200.txt", "r");
    if (!fMC_calo || !fMC_tk || !fMC_mu || !fMC_vtx) return 2;
    const glbeta_t etaCenter = 2*PFREGION_ETA_SIZE; // eta = +2.0

    // streams to be produced (inputs to the  correlator)
    //PatternSerializer serPatternsIn("input-emp.txt"), 
    //                  serPatternsReg("output-emp-regionized-ref.txt"), 
    //                  serPatternsPf("output-emp-pf-ref.txt"), 
    //                  serPatternsPuppi("output-emp-puppi-ref.txt");
    PatternMultiplexer muxPatternsIn("input-emp.txt");

    // INPUT DESCRIPTION
    // TLEN x NTKSECTORS x NTKFIBERS // TRACKER x 3
    typedef PatternMultiplexer::mask_t mask_t;
    unsigned shift_trk = 0;  // first link of tracker
    mask_t trk0=mask_t( (uint64_t(1)<<( NTKSECTORS))-1 ); // 2^n-1
    mask_t trk1=trk0<<( NTKSECTORS ); 
    mask_t trk2=trk1<<( NTKSECTORS ); 
    
    muxPatternsIn.add_tmux(1,trk0,trk1,TLEN);
    muxPatternsIn.add_tmux(2,trk0,trk2,TLEN*2);

    unsigned shift_calo = 3* NTKSECTORS ; 
    mask_t calo0=( mask_t( (  uint64_t(1)<<(NCALOSECTORS*NCALOFIBERS) )-1) )<<shift_calo;
    mask_t calo1=calo0 <<(NCALOSECTORS*NCALOFIBERS);
    mask_t calo2=calo1 <<(NCALOSECTORS*NCALOFIBERS);

    muxPatternsIn.add_tmux(1,calo0,calo1,TLEN);
    muxPatternsIn.add_tmux(2,calo0,calo2,TLEN*2);

    unsigned shift_mu = shift_calo+ 3*(NCALOSECTORS*NCALOFIBERS);
    mask_t mu0 = (mask_t( (uint64_t(1)<<1) -1))<<shift_mu;
    mask_t mu1=mu0 <<(1);
    mask_t mu2=mu1 <<(1);

    muxPatternsIn.add_tmux(1,mu0,mu1,TLEN);
    muxPatternsIn.add_tmux(2,mu0,mu2,TLEN*2);

    unsigned shift_z0 = shift_mu+3;
    mask_t z0_0 = mask_t(1)<<shift_z0;
    //mask_t z0_1 = z0_0<<1;
    //mask_t z0_2 = z0_1<<1;

    //muxPatternsIn.add_tmux(1,z0_0,z0_1,6);
    //muxPatternsIn.add_tmux(2,z0_0,z0_2,12);

    std::cout<<"----- FIBERS CONFIGURATION -----"<<std::endl;
    std::cout<<"trk0: "<< trk0<<std::endl;
    std::cout<<"trk1: "<< trk1<<std::endl;
    std::cout<<"trk2: "<< trk2<<std::endl;
    std::cout<<"cal0: "<< calo0<<std::endl;
    std::cout<<"cal1: "<< calo1<<std::endl;
    std::cout<<"cal2: "<< calo2<<std::endl;
    std::cout<<" mu0: "<< mu0<<std::endl;
    std::cout<<" mu1: "<< mu1<<std::endl;
    std::cout<<" mu2: "<< mu2<<std::endl;
    std::cout<<"z0_0: "<< z0_0<<std::endl;
    //std::cout<<"z0_1: "<< z0_1<<std::endl;
    //std::cout<<"z0_2: "<< z0_2<<std::endl;
    std::cout<<"--------------------------"<<std::endl;

    //muxPatternsIn.debug_print();
    

    PatternSerializer 
                      serPatternsReg("output-emp-regionized-ref.txt"), 
                      serPatternsPf("output-emp-pf-ref.txt"), 
                      serPatternsPuppi("output-emp-puppi-ref.txt");



    // debug and assumptions
    assert(PACKING_NCHANN >= NTKSECTORS*NTKFIBERS + NCALOSECTORS*NCALOFIBERS + NMUFIBERS + 1);
    assert(PACKING_NCHANN >= NTKOUT + NCALOOUT + NMUOUT);

    // column storage -- we will read a frame per clock
    ap_uint<64> all_channels_in[PACKING_NCHANN], 
        all_channels_regionized[PACKING_NCHANN], 
        all_channels_pf[PACKING_NCHANN], 
        all_channels_puppi[PACKING_NCHANN];

    // initialized arrays to zero
    for (unsigned int i = 0; i < PACKING_NCHANN; ++i) {
        all_channels_in[i] = 0; all_channels_regionized[i] = 0; all_channels_pf[i] = 0; all_channels_puppi[i] = 0;  
    }

    std::cout<<"Before null frame"<<std::endl; 
    muxPatternsIn.debug_print(true);

    // prepend one null frame at the beginning
    muxPatternsIn(all_channels_in, false,0); 
    muxPatternsIn(all_channels_in, false,1); 
    muxPatternsIn(all_channels_in, false,2); 
   
    //std::cout<<"After null frame"<<std::endl; 
    //muxPatternsIn.debug_print(true);


    // loop over the events // copied actually
    int frame = 0; 
    bool ok = true; z0_t oldZ0;
    for (int itest = 0; itest < 30; ++itest) {
        std::vector<TkObj>      tk_inputs[NTKSECTORS];
        std::vector<HadCaloObj> calo_inputs[NCALOSECTORS][NCALOFIBERS];
        std::vector<GlbMuObj>   mu_inputs;
        std::vector<std::pair<z0_t,pt_t>> vtx_inputs;

        uint32_t run = 0, lumi = 0; uint64_t event = 0;
        if (!readEventTkTM18(fMC_tk, tk_inputs, run, lumi, event) || 
                !readEventCalo(fMC_calo, calo_inputs, /*zside=*/true, run, lumi, event) ||
                !readEventMuTM18(fMC_mu, mu_inputs, run, lumi, event) ||
                !readEventVtx(fMC_vtx, vtx_inputs, run, lumi, event)) break;

        z0_t vtxZ0 = vtx_inputs.empty() ? z0_t(0) : vtx_inputs.front().first;
        //if (itest == 0) printf("Vertexis at z0 = %d\n", vtxZ0.to_int());
       
        // loop to enqueue TMUX=18 informaton into the muxer 
        //std::cout <<"TLEN="<<TLEN<<std::endl;
        for (int i = 0; i < 3*TLEN; ++i, ++frame) {
            TkObj tk_links_in[NTKSECTORS];
            PackedTkObj tk_links64_in[NTKSECTORS];

            unsigned int ilink = 0;

            for (int s = 0; s < NTKSECTORS; ++s) {
                clear(tk_links_in[s]);
                if (i < 3*TLEN-1 && i < int(tk_inputs[s].size())) { // emp protocol, must leave one null frame at the end
                    tk_links_in[s]  = tk_inputs[s][i];
                }
                tk_links64_in[s] = l1pf_pattern_pack_one(tk_links_in[s]);
                all_channels_in[shift_trk+ilink++] = tk_links64_in[s];
            }
             
            ilink=0; // reset so I skip the fibers for the extra trk

            HadCaloObj    calo_links_in[NCALOSECTORS][NCALOFIBERS];
            PackedCaloObj calo_links64_in[NCALOSECTORS][NCALOFIBERS];
            for (int s = 0; s < NCALOSECTORS; ++s) {
                for (int f = 0; f < NCALOFIBERS; ++f) {
                    clear(calo_links_in[s][f]);
                    if (i < 3*TLEN-1 && i < int(calo_inputs[s][f].size())) { // emp protocol, must leave one null frame at the end
                        calo_links_in[s][f]  = calo_inputs[s][f][i];
                    }
                    calo_links64_in[s][f] = l1pf_pattern_pack_one(calo_links_in[s][f]);
                    all_channels_in[shift_calo+ilink++] = calo_links64_in[s][f];
                }
            }

            ilink=0;
            GlbMuObj    mu_links_in;
            PackedMuObj mu_links64_in;
            clear(mu_links_in);
            if (i < 3*TLEN-1 && i < int(mu_inputs.size())) { // emp protocol, must leave one null frame at the end
                mu_links_in  = mu_inputs[i];
            }
            mu_links64_in = l1pf_pattern_pack_one(mu_links_in);
            all_channels_in[shift_mu+ilink++] = mu_links64_in;
    
            ilink=0;
            all_channels_in[shift_z0+ilink++] = oldZ0;

            /*
            TkObj        tk_links_ref[NTKOUT];
            HadCaloObj calo_links_ref[NCALOOUT];
            MuObj        mu_links_ref[NMUOUT]; 

            bool newevt_ref = (i == 0);
            bool   tk_ref_good =   tk_router_ref(i == 0,   tk_links_in, tk_links_ref);
            bool calo_ref_good = calo_router_ref(i == 0, calo_links_in, calo_links_ref);
            bool   mu_ref_good =   mu_router_ref(i == 0, etaCenter, mu_links_in, mu_links_ref);

            l1pf_pattern_pack<NTKOUT,0>(tk_links_ref, all_channels_regionized);
            l1pf_pattern_pack<NCALOOUT,NTKOUT>(calo_links_ref, all_channels_regionized);
            l1pf_pattern_pack<NMUOUT,NTKOUT+NCALOOUT>(mu_links_ref, all_channels_regionized);

            if ((itest > 0) && (i % PFLOWII == 0) && (i/PFLOWII < NPFREGIONS)) {
                ////  ok we can run PF and puppi
                // PF objects
                PFChargedObj pfch[NTRACK], pfmu[NMU]; PFNeutralObj pfallne[NALLNEUTRALS];
                assert(NTKOUT == NTRACK && NCALOOUT == NCALO && NMUOUT == NMU);
                //pfalgo2hgc_ref_set_debug(itest == 1);
                pfalgo2hgc_ref(pfcfg, calo_links_ref, tk_links_ref, mu_links_ref, pfch, pfallne, pfmu); 
                pfalgo2hgc_pack_out(pfch, pfallne, pfmu, all_channels_pf);
                // Puppi objects
                //if (itest == 1) printf("Will run Puppi with z0 = %d\n", oldZ0.to_int());
                PFChargedObj outallch[NTRACK];
                PFNeutralObj outallne_nocut[NALLNEUTRALS], outallne[NALLNEUTRALS], outselne[NNEUTRALS]; 
                linpuppi_ref(pucfg, tk_links_ref, oldZ0, pfallne, outallne_nocut, outallne, outselne, false); //itest == 1);
                linpuppi_chs_ref(pucfg, oldZ0, pfch, outallch, false); //itest == 1);
                l1pf_pattern_pack<NTRACK,0>(outallch, all_channels_puppi);
                l1pf_pattern_pack<NALLNEUTRALS,NTRACK>(outallne, all_channels_puppi);
            }
            */

            muxPatternsIn(all_channels_in, (i < 3*TLEN-1), itest %3);
            //serPatternsReg(all_channels_regionized);
            //serPatternsPf(all_channels_pf);
            //serPatternsPuppi(all_channels_puppi);

            /*
            if (i == 3*TLEN-1) {
                oldZ0 = vtxZ0; 
            }
            */
	


        }
    	std::cout<<"After event: "<<itest<<" "<< " IMUX " <<itest %3 <<std::endl; 
    	muxPatternsIn.debug_print(true);
        muxPatternsIn.print();
    } 
    // moved after one event
    muxPatternsIn.flush();
    fclose(fMC_tk);
    fclose(fMC_calo);
    fclose(fMC_mu);
    fclose(fMC_vtx);
    return ok ? 0 : 1;

}// end main
