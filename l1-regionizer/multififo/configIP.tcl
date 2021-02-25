set hlsTopFunc router_nomerge
if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "none" }
if { [ info exists env(pfReg) ] } { set pfReg $env(pfReg) } { set pfReg "HGCal" }
if { [ info exists env(regionizerMode) ] } { set regionizerMode $env(regionizerMode) } { set regionizerMode "mux" }

if { $regionizerMode == "mux" } {
    set regionizerCFlags "-DROUTER_NOSTREAM -DNO_VALIDATE -DROUTER_ISMUX=1 -DROUTER_ISSTREAM=0";
} elseif { $regionizerMode == "stream" } {
    set regionizerCFlags "-DROUTER_STREAM -DNO_VALIDATE -DROUTER_ISMUX=1 -DROUTER_ISSTREAM=1";
} elseif { $regionizerMode == "nomux" } {
    set regionizerCFlags "-DROUTER_NOMUX -DROUTER_ISMUX=0 -DROUTER_ISSTREAM=0";
}

set hlsIPVersion 1.0
