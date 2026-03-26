# ===== NTT ASM =====
# Function: hpu_ntt_complete_N64_l16

# Stage 1: Stride = 32
pmodsw 0
ptwld 0
sload 0, p0
sload 2, p1
pntt p0, p1, p2
ptwid
sstore p0, 0
sstore p1, 2
sload 1, p0
sload 3, p1
pntt p0, p1, p2
ptwid
sstore p0, 1
sstore p1, 3

# Stage 2: Stride = 16
pmodsw 0
ptwld 1
sload 0, p0
sload 1, p1
pntt p0, p1, p2
ptwid
sstore p0, 0
sstore p1, 1
sload 2, p0
sload 3, p1
pntt p0, p1, p2
ptwid
sstore p0, 2
sstore p1, 3

# Stage 3: Stride = 8
pmodsw 0
ptwld 2
pshcfg 12
sload 0, p0
sload 1, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 0
sstore p1, 1
sload 2, p0
sload 3, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 2
sstore p1, 3

# Stage 4: Stride = 4
pmodsw 0
ptwld 3
pshcfg 14
sload 0, p0
sload 1, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 0
sstore p1, 1
sload 2, p0
sload 3, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 2
sstore p1, 3

# Stage 5: Stride = 2
pmodsw 0
ptwld 4
pshcfg 18
sload 0, p0
sload 1, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 0
sstore p1, 1
sload 2, p0
sload 3, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 2
sstore p1, 3

# Stage 6: Stride = 1
pmodsw 0
ptwld 5
pshcfg 26
sload 0, p0
sload 1, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 0
sstore p1, 1
sload 2, p0
sload 3, p1
pshuf2 p0, p1, p2
pntt p0, p1, p2
ptwid
sstore p0, 2
sstore p1, 3

# ===== MM ASM =====
# Function: hpu_mm_complete_N64_l16

pmodsw 1
sload 256, p0
sload 512, p1
pmul p0, p1, p2
sstore p2, 768
sload 272, p0
sload 528, p1
pmul p0, p1, p2
sstore p2, 784
sload 288, p0
sload 544, p1
pmul p0, p1, p2
sstore p2, 800
sload 304, p0
sload 560, p1
pmul p0, p1, p2
sstore p2, 816

# ===== BCONV ASM =====
# Function: hpu_bconv_complete_N64_l16

pmodsw 2
sload 1024, p0
pbcast c0, p1
pmul p0, p1, p2
pmodsw 3
sload 1280, p3
padd p3, p2, p3
sstore p3, 1280

pmodsw 2
sload 1025, p0
pbcast c0, p1
pmul p0, p1, p2
pmodsw 3
sload 1281, p3
padd p3, p2, p3
sstore p3, 1281

pmodsw 2
sload 1026, p0
pbcast c0, p1
pmul p0, p1, p2
pmodsw 3
sload 1282, p3
padd p3, p2, p3
sstore p3, 1282

pmodsw 2
sload 1027, p0
pbcast c0, p1
pmul p0, p1, p2
pmodsw 3
sload 1283, p3
padd p3, p2, p3
sstore p3, 1283
