from matplotlib import patches
import matplotlib.pyplot as plt
import numpy as np
from math import sqrt
from matplotlib.path import Path
from matplotlib.patches import Polygon
from numpy.core.numeric import binary_repr
from numpy.linalg import norm

# def offsetPoly(verts, offset):
#     oldX, oldY = [p[0] for p in verts], [p[1] for p in verts]
#     newX, newY = [], []
#     num_verts = len(oldX)

#     for curr in range(num_verts):
#         prev = (curr + num_verts - 1) % num_verts
#         next = (curr + 1) % num_verts

#         nX, nY =  oldX[next]-oldX[curr], oldY[next]-oldY[curr]
#         nNorm = norm([nX,nY])
#         nextNormalX, nextNormalY = nY/nNorm, -nX/nNorm

#         pX, pY =  oldX[curr]-oldX[prev], oldY[curr]-oldY[prev]
#         nNorm = norm([pX,pY])
#         prevNormalX, prevNormalY = pY/nNorm, -pX/nNorm

#         bisX = nextNormalX + prevNormalX
#         bisY = nextNormalY + prevNormalY

#         bisNorm = norm([bisX,bisY])
#         offset_len = offset / bisNorm

#         newX.append(oldX[curr] + offset_len * bisX)
#         newY.append(oldY[curr] + offset_len * bisY)

#     new_verts = []
#     for i in range(num_verts):
#         new_verts.append((newX[i],newY[i]))

#     return new_verts


# fig, ax = plt.subplots()
# ax.set_xlim(0,10)
# ax.set_ylim(0,10)

# poly = [(1.0,1.0),(2.0,1.0),(1.0,2.0)]
# poly2 = offsetPoly(poly, 0.5)
# print(poly2)

# path = Polygon(np.array(poly), True).get_path()
# path2 = Polygon(np.array(poly2), True).get_path()

# patch2 = patches.PathPatch(path2, facecolor='blue')
# patch = patches.PathPatch(path, facecolor='red')

# ax.add_patch(patch2)
# ax.add_patch(patch)

# plt.show()