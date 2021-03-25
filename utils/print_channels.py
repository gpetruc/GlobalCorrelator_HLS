#!/usr/bin/env python
import sys

from optparse import OptionParser
from patterns import FrameSet, add_common_options, parse_option
parser = OptionParser(usage="%prog [options] ref test")
add_common_options(parser)
parser.add_option("-N", "--num", dest="num", type="int", default=0, help="print only first N frames")
parser.add_option("-s", "--skip", dest="skip", type="int", default=0, help="skip first N frames from test dump")
(options,args) = parser.parse_args()
parse_option(options)

frames = FrameSet(args[0], options)
if options.skip: frames.skipFrames(options.skip)
frames.dump(options.num)

