import numpy as np
import pynq
lo, hi = 0, 1000
N = 256
x = np.random.randint(lo, hi, N, dtype='uint16')

# load the bitfile onto device and get handle for IP
overlay = pynq.Overlay('proj.xclbin')

func = overlay.func_many_streams_1 # the "_1" is added by Vitis

# print the function signature to verify
print(f'IP signature: {func.signature}')

# allocate buffers for in/out
xbuf = pynq.allocate(N, dtype='uint16')
rbuf = pynq.allocate(N, dtype='uint64')

import time

t0 = time.perf_counter()
# copy data to input buffer
xbuf[:] = x

# synchronise buffer to device (Host->Device)
xbuf.sync_to_device()

# execute the IP function
func.call(N, xbuf, rbuf)

# synchronise output buffer from device (Device->Host)
rbuf.sync_from_device()
t1 = time.perf_counter()

ok = True
for i in range(N):
    n = int(xbuf[i])
    c = (n*(n+1)//2)**2
    if rbuf[i] != c:
        ok = False
    print(f'{i:3d}  {xbuf[i]:4d}  {rbuf[i]:20d}   {c:20d}  {c==rbuf[i]}')
print("Done after %.3f us, ok? %s" % ((t1-t0)*1e6, ok))
