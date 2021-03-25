#!/usr/bin/env python
def add_common_options(parser):
    parser.add_option("-f", "--format", dest="format", default="plain", help="format: plain, emp")
    parser.add_option("--plain", action="store_const", dest="format", const="plain", help="set format to emp")
    parser.add_option("--emp", action="store_const", dest="format", const="emp", help="set format to emp")
    parser.add_option("-c", "--channels", dest="channels", default=None, help="channels to look at: e.g. 0,2,7-9 ")
    parser.add_option("--si", "--skip-invalid", dest="skipInvalid", action="store_true", default=False, help="skip invalid frames (starting with 0v)")
    parser.add_option("--svb", "--skip-valid-bit", dest="skipValidBit", action="store_true", default=False, help="strip away the valid bit")
    parser.add_option("-v", action="count",  dest="verbose", default=1, help="increase verbosity")
    parser.add_option("-q", action="store_const", dest="verbose", const=0, help="reduce verbosity")
    parser.add_option("--decode", dest="decode", default=None, help="Decoding algorithm")


def parse_option(options):
    if options.channels:
        channels_enable = {};
        for cmap in options.channels.split(","):
            channels = []; openEnd = False
            for cpair in cmap.split(":"):
                if cpair[-1] == "-":
                    channels.append([int(cpair[:-1])])
                    openEnd = True
                elif "-" in cpair:
                    first, last = map(int,cpair.split("-"))
                    channels.append(range(first,last+1))
                else:
                    channels.append([int(cpair)])
            if len(channels) == 1:
                channels_enable.update(dict((i,i) for i in channels[0]))
            elif  len(channels) == 2:
                if len(channels[0]) == len(channels[1]): 
                    channels_enable.update(dict((i,j) for (i,j) in zip(channels[0],channels[1])))
                elif len(channels[0]) > 1 and len(channels[1]) == 1 and openEnd:
                    offs = channels[1][0] - channels[0][0]
                    channels_enable.update(dict((i,i+offs) for i in channels[0]))
                else:
                    raise RuntimeError("Bad channel map %s" % cmap)
            else:
                raise RuntimeError("Bad channel map %s" % cmap)
        options.channels = channels_enable
    if options.decode == "puppi64":
        options.decode = lambda x : map(decode_puppi64, x)


def sub_unsigned(pattern, hi, lo):
    return (pattern >> lo) & ((2 << (hi-lo))-1);
def sub_signed(pattern, hi, lo):
    raw  = sub_unsigned(pattern, hi, lo)
    return raw-2**(hi-lo+1) if pattern & (1<<hi) else raw

def decode_puppi64(pattern):
    if pattern.startswith("1v") or pattern.startswith("0v"):
        pattern = pattern[2:]
    word = long(pattern, 16)
    if word == 0:
        return "%-47s | " % "<null>"
    pidlabels = [ "h0", "g ", "h-", "h+", "e-", "e+", "m-", "m+" ];
    rawpid = sub_unsigned(word, 39, 37);
    core = "%2s pt%5d eta %+4d phi %+4d" % (
                pidlabels[rawpid],
                sub_unsigned(word, 13,  0),
                sub_signed(  word, 25, 14),
                sub_signed(  word, 36, 26))
    if rawpid <= 1:
        extra = "pW %4d rest %5x" % ( sub_unsigned(word,49,40), sub_unsigned(word,63,50) )
    else:
        extra = "z %+4d xy %+4d q %d" % ( sub_signed(word,49,40), sub_signed(word,57,50), sub_unsigned(word,60,58) )
    return "%s %-18s |" % (core, extra)

class FrameSet:
    def __init__(self, filename, options, isRef=True):
        self._filename = filename
        self._frames = []
        for line in open(filename, "r"):
            fields = line.strip().split()
            if options.format == "emp":
                if not line.startswith("Frame"): continue
                if fields[0] != "Frame": continue
                if fields[2] != ":": raise RuntimeError("Malformed line in file %s: %s" % (filename, line))
                frameno = int(fields[1])
                fdata   = map(lambda s : s.lower(), fields[3:])
            else:
                frameno = int(fields[0]); 
                fdata = fields[1:]
            if options.channels:
                if isRef:
                    fdata = [ v for (i,v) in enumerate(fdata) if i in options.channels ]
                else:
                    fdata = [ fdata[c2] for (c1,c2) in sorted(options.channels.items()) ]
            if options.format == "emp":
                if options.skipInvalid and all(d.startswith("0v") for d in fdata): continue
                if options.skipValidBit: fdata = [d[2:] for d in fdata]
            if options.decode:
                fdata = options.decode(fdata)
            self._frames.append( [frameno, fdata] )
        if not self._frames: raise RuntimeError("No valid patterns in file %s" % filename)
        maxflen = max(len(f[1]) for f in self._frames)
        minflen = min(len(f[1]) for f in self._frames)
        if maxflen != minflen: raise RuntimeError("Frame length mismatch in file: min %d, max %d" % (minflen, maxflen))
        self._nchannels = minflen
        if options.verbose > 0:
            print "Loaded %d frames from %s with %d channels" % (len(self._frames), self._filename, self._nchannels)
    def delay(self,nframes):
        for f in self._frames: f[0] += nframes
    def __getitem__(self,index):
        for f in self._frames: 
            if f[0] == index: return f[1]
        raise IndexError
    def listFrames(self):
        return [f[0] for f in self._frames]
    def firstFrame(self):
        return self._frames[0]
    def allFrames(self):
        return self._frames
    def nFrames(self):
        return len(self._frames)
    def nChannels(self):
        return self._nchannels
    def skipFrames(self, nframes):
        self._frames = self._frames[nframes:]
    def cropChannels(self,nchannels):
        if nchannels < self._nchannels:
            self._frames = [ [ f[0], f[1][:nchannels] ] for f in self._frames ]
            if options.verbose > 0:
                print "Cropped %s to %d channels" % (self._filename, nchannels)
    def dump(self, maxFrames):
        chann_w = max(max(len(str(c)) for c in f[1]) for f in self._frames)
        for i,( fn, fd) in enumerate(self._frames):
            print("%04d :  %s" % (fn, "  ".join(("%-*s" % (chann_w, c)) for c in fd)))
            if i+1 == maxFrames: break



