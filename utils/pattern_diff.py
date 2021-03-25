#!/usr/bin/env python
import sys

from optparse import OptionParser
from patterns import FrameSet, add_common_options, parse_option
parser = OptionParser(usage="%prog [options] ref test")
add_common_options(parser)
parser.add_option("--max-attempts", dest="maxAttempts", type="int", default=10, help="max attempts to look for a matching frame")
parser.add_option("-l", "--latency",dest="latency", type=int, default=0, help="Latency to add to the ref patterns to match the test one")
parser.add_option("-s", "--skip", dest="skip", type="int", default=0, help="skip first N frames from test dump")
parser.add_option("--sr", "--skipRef", dest="skipRef", type="int", default=0, help="skip first N frames from ref dump")
parser.add_option("-N", "--max-frames", dest="maxFrames", type="int", default=10000, help="number of channels")
parser.add_option("-E", dest="numberOfErrors", type=int, default=1, help="Number of errors after which to stop when using 'exact' matching")
(options,args) = parser.parse_args()
parse_option(options)

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

ref = FrameSet(args[0], options, True)
test = FrameSet(args[1], options, False)
if options.skipRef: ref.skipFrames(options.skipRef) 
if options.latency: ref.delay(options.latency)
if options.skip: test.skipFrames(options.skip)

if len(args) == 3 and args[2] == "exact":
    if not match_exact(ref, test, options.maxFrames):
        sys.exit(1)
else:
    if not try_find_match(ref, test, options.maxFrames, options.maxAttempts):
        sys.exit(1)

