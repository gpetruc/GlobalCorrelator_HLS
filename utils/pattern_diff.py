#!/usr/bin/env python
import re
import sys

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] ref test")
parser.add_option("-f", "--format", dest="format", default="plain", help="format: plain, emp")
parser.add_option("--emp", action="store_const", dest="format", const="emp", help="set format to emp")
parser.add_option("-l", "--latency",dest="latency", type=int, default=0, help="Latency to add to the ref patterns to match the test one")
parser.add_option("-N", "--max-frames", dest="maxFrames", type="int", default=10000, help="number of channels")
parser.add_option("--max-attempts", dest="maxAttempts", type="int", default=10, help="max attempts to look for a matching frame")
parser.add_option("-s", "--skip", dest="skip", type="int", default=0, help="skip first N frames from test dump")
parser.add_option("--sr", "--skipRef", dest="skipRef", type="int", default=0, help="skip first N frames from ref dump")
parser.add_option("-c", "--channels", dest="channels", default=None, help="channels to look at: e.g. 0,2,7-9 ")
parser.add_option("--si", "--skip-invalid", dest="skipInvalid", action="store_true", default=False, help="skip invalid frames (starting with 0v)")
parser.add_option("--svb", "--skip-valid-bit", dest="skipValidBit", action="store_true", default=False, help="strip away the valid bit")
parser.add_option("-v", action="count",  dest="verbose", default=1, help="increase verbosity")
parser.add_option("-q", action="store_const", dest="verbose", const=0, help="reduce verbosity")
parser.add_option("-E", dest="numberOfErrors", type=int, default=1, help="Number of errors after which to stop when using 'exact' matching")

(options,args) = parser.parse_args()
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

class FrameSet:
    def __init__(self, filename, isRef, options):
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

def match_exact(fs1, fs2, nmax):
    frames1 = fs1.listFrames()
    frames2 = fs2.listFrames()
    min1, min2 = min(frames1), min(frames2)
    max1, max2 = max(frames1), max(frames2)
    min12 = max(min1, min2)
    max12 = min(max1, max2)
    iframe = 0
    nmatch = 0
    errs = 0
    for f in xrange(min12, max12+1):
        iframe += 1
        if iframe > nmax: 
            print "%d successfully matched frames requested" % nmatch
            return True
        if options.verbose >= 2: print "frame %04d " % f,
        d1, d2 = fs1[f], fs2[f]
        if len(d1) != len(d2): raise RuntimeError("Frame sizes mismatch: %d vs %d!" % (len(d1), len(d2)))
        if d1 == d2: 
            nmatch += 1
            if options.verbose >= 2: print " match ok: ", "  ".join(d1)
        else:
            if options.verbose: 
                if options.verbose < 2: 
                    print "mismatch at frame %04d, after %d successfully matched frames:" % (f, nmatch)
                else:
                    print " mismatch."
                for i in xrange(len(d1)):
                    print "\tchannel % 3d:  %s  vs  %s : %s " % (i, d1[i], d2[i], "ok" if d1[i] == d2[i] else "FAIL")
            else:
                print "mismatch at frame %04d, after %d successfully matched frames" % (f, nmatch)
            errs += 1
            if errs >= options.numberOfErrors:
                return False
    print "%d successfully matched frames (%d errors)" % (nmatch-errs, errs)
    return True

def try_find_match(fs1, fs2, nmax, maxattempts):
    n1, n2 = fs1.nFrames(), fs2.nFrames()
    fno1, d1 = fs1.firstFrame()
    for fno2, d2 in fs2.allFrames():
        if fno2 < fno1: continue
        if d2 == d1: 
            if options.verbose: print "possible match found for #1[%04d] vs #2[%04d]" % (fno1, fno2)
            ok = True
            for i in xrange(1,nmax):
                if fno1+i >= n1 or fno2+i >= n2:
                    nmax = i
                    if options.verbose: print "   ---> stop after %d frames for end of file" % i
                    break
                if fs1[fno1+i] != fs2[fno2+i]:
                    if options.verbose: print "   ---> but fails after %d frames" % i
                    maxattempts -= 1
                    if maxattempts == 0:
                        if options.verbose: print "   ---> giving up"
                        return False
                    ok = False
                    break
            if ok:
                print "   ---> successful match for %d consecutive frames starting from #1[%04d] vs #2[%04d]." % (nmax, fno1, fno2)
                return True
    print "Could not find frame %04d of #1 in #2: %s"  % (fno1, "  ".join(d1))
    return False
ref = FrameSet(args[0], True, options)
test = FrameSet(args[1], False, options)
if options.skipRef: ref.skipFrames(options.skipRef) 
if options.latency: ref.delay(options.latency)
if options.skip: test.skipFrames(options.skip)

if len(args) == 3 and args[2] == "exact":
    if not match_exact(ref, test, options.maxFrames):
        sys.exit(1)
else:
    if not try_find_match(ref, test, options.maxFrames, options.maxAttempts):
        sys.exit(1)

