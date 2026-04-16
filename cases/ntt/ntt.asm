# NTT N=64
pmodld p2, 0, 0
pshcfg p3, 0, 0
pntt p1, p0, 0, 0, 0
pntt p0, p1, 1, 0, 0
pntt p1, p0, 2, 0, 0
pntt p0, p1, 3, 0, 0
pntt p1, p0, 4, 0, 0
pntt p0, p1, 5, 0, 0
psync 0, 0
