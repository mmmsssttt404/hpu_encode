# NTT
pmodld p2, 0, 0
pshcfg p3, 0, 0
pntt p1, p0, 0, 0, 0
pntt p0, p1, 1, 0, 0
pntt p1, p0, 2, 0, 0
pntt p0, p1, 3, 0, 0
pntt p1, p0, 4, 0, 0
pntt p0, p1, 5, 0, 0
psync 0, 0

# INTT
pmodld p2, 0, 0
pshcfg p3, 0, 0
pintt p1, p0, 0, 0, 0
pintt p0, p1, 1, 0, 0
pintt p1, p0, 2, 0, 0
pintt p0, p1, 3, 0, 0
pintt p1, p0, 4, 0, 0
pintt p0, p1, 5, 0, 0
psync 0, 0

# MM
pmodld p3, 0, 0
pmul p0, p1, p2
psync 0, 0

# BCONV
pmodld p5, 0, 0
pmul p1, p0, p3
pmodld p6, 0, 0
pmul p2, p1, p4
psync 0, 0

# DMA
# load_type: 0 fragment, 1 full poly, 2 mod ctx, 3 shuffle cfg
# dstore arg4: rel
 dload x10, x11, p2, 2
 dload x12, x13, p3, 3
 dstore x14, x15, p1, 1
