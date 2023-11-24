import numpy as np
import pynq
lo, hi = 0, 1000
N = 100
x = np.random.randint(lo, hi, N, dtype='int16')
y = np.random.randint(lo, hi, N, dtype='int16')

# reference evaluation. I need to upcast manually
z_cpu = x.astype('int32') * y.astype('int32') 

# load the bitfile onto device and get handle for IP
overlay = pynq.Overlay('proj.xclbin')

func = overlay.func_1 # the "_1" is added by Vitis

# print the function signature to verify
print(f'IP signature: {func.signature}')

# allocate buffers for in/out
xbuf = pynq.allocate(N, dtype='int16')
ybuf = pynq.allocate(N, dtype='int16')
zbuf = pynq.allocate(N, dtype='int32')

# copy data to input buffer
xbuf[:] = x
ybuf[:] = y

# synchronise buffer to device (Host->Device)
xbuf.sync_to_device()
ybuf.sync_to_device()

# execute the IP function
func.call(N, xbuf, ybuf, zbuf)

# synchronise output buffer from device (Device->Host)
zbuf.sync_from_device()

for i in range(min(N,10)):
    print(f'{i:3d}  {xbuf[i]:4d}  {ybuf[i]:4d}   {zbuf[i]:8d}  {z_cpu[i]:8d}     {z_cpu[i]==zbuf[i]}')
# verify correctness
print(f'CPU matches Alveo evaluation : {np.all(z_cpu == zbuf)}')