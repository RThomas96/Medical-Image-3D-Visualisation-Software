import matplotlib.pyplot as plt
import matplotlib.colors
import numpy as np
import scipy.stats as st
import random
from mpl_toolkits import mplot3d

from skimage import metrics
from skimage import measure
from skimage import io as skio

from scipy.spatial import distance

def subsampleMesh(vertices, nbVertices):
    permutation = [i for i in range(vertices.shape[0])]
    random.shuffle(permutation)
    return vertices[permutation[0:nbVertices]]

def computeDistances(atlas, registred):
    distances = np.zeros(atlas.shape[0])
    for i in range(0, distances.size):
        arrDistance = distance.cdist([atlas[i]], registred)
        distances[i] = arrDistance[0].min()
    
    distances = distances * 25.
    return distances

def plotData(ax, data, color, front):
    X = [i[0] for i in data]
    Y = [i[1] for i in data]
    Z = [i[2] for i in data]
    im = ax.scatter(X, Y, Z, s = 100, c = color, edgecolor='k')
    
    ax.set_title('')
    if front:
        ax.view_init(22, -129)
    else:
        ax.view_init(24, -44)
    
    # Set axes label
    ax.set_xlabel('x', labelpad=20)
    ax.set_ylabel('y', labelpad=20)
    ax.set_zlabel('z', labelpad=20)

    ax.set_xlim3d(0, 350)
    ax.set_ylim3d(0, 331)
    ax.set_zlim3d(0, 250)
    #ax.set_xlim3d(0, 30)
    #ax.set_ylim3d(0, 512)
    #ax.set_zlim3d(0, 512)
    return im

def plot2DData(ax, data, color, slice):
    X = [i[0] for i in data if i[2] == slice]
    Y = [i[1] for i in data if i[2] == slice]
    Z = [i[2] for i in data if i[2] == slice]
    color = [i for id, i in enumerate(color) if data[id][2] == slice]
    im = ax.scatter(X, Y, Z, s = 100, c = color, edgecolor='k')
    
    ax.set_title('')
    
    # Set axes label
    ax.set_xlabel('x', labelpad=20)
    ax.set_ylabel('y', labelpad=20)
    ax.set_zlabel('z', labelpad=20)

    ax.set_xlim3d(0, 350)
    ax.set_ylim3d(0, 331)
    ax.set_zlim3d(0, 250)
    return im

# Data computation
registred_solo = skio.imread("atlas_edges.tiff", plugin="tifffile")

plt.rcParams.update({'font.size': 22})

maxValue = 0

atlas = np.ndarray(shape=(10), dtype=object)
registred = np.ndarray(shape=(10), dtype=object)
slice = np.ndarray(shape=(10), dtype=int)

# I
atlas[0] = skio.imread("registred_edges.tiff", plugin="tifffile")
atlas[0] = atlas[0][0:282, 50:400, 100:296]
registred[0] = registred_solo[0:282, 50:400:, 100:296]
slice[0] = 0

atlas[1] = skio.imread("data/4GG_edge.tiff", plugin="tifffile")
atlas[1][151:, 246:, 4:246] = 0
atlas[1] = atlas[1][0:282, 50:400, 100:296]
registred[1] = registred_solo[0:282, 50:400:, 100:296]
slice[1] = 0

# II
atlas[2] = skio.imread("data/2-1D_edges.tif", plugin="tifffile")
atlas[2] = atlas[2][0:282, 50:400, 100:296]
atlas[2][158:, :37, :136] = 0
registred[2] = registred_solo[0:282, 50:400:, 100:296]
slice[2] = 0

atlas[3] = skio.imread("data/2-2D_edges.tiff", plugin="tifffile")
atlas[3] = atlas[3][248:, 50:400, 100:296]
registred[3] = registred_solo[248:, 50:400:, 100:296]
slice[3] = 0

atlas[4] = skio.imread("data/2-3D_edges.tiff", plugin="tifffile")
atlas[4] = atlas[4][290:, 50:400, 100:296]
registred[4] = registred_solo[290:, 50:400:, 100:296]
slice[4] = 0

# III
atlas[5] = skio.imread("data/3-1D_edges.tiff", plugin="tifffile")
atlas[5] = atlas[5][283:, 50:400, 100:296]
registred[5] = registred_solo[283:, 50:400:, 100:296]
slice[5] = 0

atlas[6] = skio.imread("data/3-2D_edges.tiff", plugin="tifffile")
atlas[6] = atlas[6][175:, 50:400, 100:296]
registred[6] = registred_solo[175:, 50:400:, 100:296]
slice[6] = 0

atlas[7] = skio.imread("data/3-3D_edges.tiff", plugin="tifffile")
atlas[7] = atlas[7][125:, 50:400, 100:296]
registred[7] = registred_solo[125:, 50:400:, 100:296]
atlas[7][:91, 170:, 111:] = 0
slice[7] = 0

atlas[8] = skio.imread("data/3-4D_edges.tiff", plugin="tifffile")
atlas[8] = atlas[8][119:, 50:400, 100:296]
registred[8] = registred_solo[125:, 50:400:, 100:296]
slice[8] = 0

atlas[9] = skio.imread("IRM/registred-1D-edges.tiff", plugin="tifffile")
atlas[9] = atlas[9][7:25, :, :]
registred[9] = skio.imread("IRM/MRI_edges.tif", plugin="tifffile")[7:25, :, :]
slice[9] = 0

#skio.imsave("data/atlas_crop.tiff", registred)

outliersValue = 750

finalDistances = np.array([])

mean = 0
std = 0
median = 0
for i in range(0, 9):
#for i in range(0, 0):
    atlas_verts_full = np.argwhere(atlas[i] == 255)
    registred_verts = np.argwhere(registred[i] == 255)
    
    atlas_verts = subsampleMesh(atlas_verts_full, 5000)
    
    distances = computeDistances(atlas_verts, registred_verts)

    outliers = np.where(distances > outliersValue)
    distances = np.delete(distances, outliers)
    atlas_verts = np.delete(atlas_verts, outliers, axis=0)

    currentMax = distances.max()
    if currentMax > maxValue:
        maxValue = currentMax

    fig = plt.figure(figsize = (10,10))
    ax = plt.axes(projection='3d')
    im = plotData(ax, atlas_verts, distances, i <= 2)

    fig.tight_layout()
    
    plt.savefig("fig" + str(i) + ".png")
    skio.imsave("data/2DSlice" + str(i)  + ".tiff", atlas[i][:, :, 0])
    print("Save figure " + str(i))
    print("Mean: " + str(round(distances.mean(), 3)))
    print("Std: " + str(round(distances.std(), 3)))
    print("Median: " + str(round(np.median(distances), 3)))
    print(str(round(distances.mean(), 3)) + " & " + str(round(distances.std(), 3)) + " & " + str(round(np.median(distances), 3)))
    mean += distances.mean()
    std += distances.std()
    median += np.median(distances)
    finalDistances = np.concatenate((finalDistances, distances))

    # 2D plot

    #fig = plt.figure(figsize = (10,10))
    #ax = plt.axes(projection='3d')

    ##atlas_verts_2D = np.array([j for j in atlas_verts_full if j[2] == slice[i]])
    ##atlas_verts_2D = np.argwhere(atlas[i][:, :, 0] == 255)
    #distances = computeDistances(atlas_verts_2D, registred_verts)

    #outliers = np.where(distances > outliersValue)
    #distances = np.delete(distances, outliers)
    #atlas_verts_2D = np.delete(atlas_verts_2D, outliers, axis=0)

    #im = plotData(ax, atlas_verts_2D, distances, i<=2)
    #plt.savefig("fig2D" + str(i) + ".pdf")

print("Mean: " + str(mean/9.))
print("Std: " + str(std/9.))
print("Median: " + str(median/9.))

fig = plt.figure(figsize = (10,10))
ax = plt.axes(projection='3d')

atlas_verts_full = np.argwhere(atlas[0] == 255)
registred_verts = np.argwhere(registred[0] == 255)

atlas_verts = subsampleMesh(atlas_verts_full, 5000)

#distances = np.logspace(0, 2, atlas_verts.shape[0])
distances = np.zeros(atlas_verts.shape[0])
distances[0] = maxValue
im = plotData(ax, atlas_verts, distances, True)

fig.tight_layout()

bar = fig.colorbar(im, orientation='vertical')
bar.ax.set_title('\u03BCm')

plt.savefig("fakeColorBar.pdf")
print("Save figure color bar")

fig = plt.figure(figsize = (10,10))
ax = plt.axes()

print("Size final distance: " + str(finalDistances.size))
q25, q75 = np.percentile(finalDistances, [25, 75])
bin_width = 2 * (q75 - q25) * len(finalDistances) ** (-1/3)
bins = round((finalDistances.max() - finalDistances.min()) / bin_width)
plt.hist(finalDistances, density=True, bins=bins);
mn, mx = plt.xlim()
plt.xlim(mn, mx)
kde_xs = np.linspace(mn, mx, 300)
kde = st.gaussian_kde(finalDistances)
plt.hist(finalDistances, bins=bins);
plt.plot(kde_xs, kde.pdf(kde_xs), label="PDF")
plt.legend(loc="upper left")
plt.ylabel("Probability")
plt.xlabel("Data")
plt.title("Histogram");

plt.show()
plt.savefig("histogram.pdf")
