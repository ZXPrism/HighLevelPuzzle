import random
from queue import *
import time

# configurations
minVoxelNum = 1
maxVoxelNum = 10
# puzzle size
width = 10
height = 10

fileName = "test_" + str(int(time.time())) + ".cfg"
fileHandle = open(fileName, "x")

fileHandle.write("1717935966\n")


# generate puzzle piece num
n = random.randint(5, max(5, width * height / 5))

fileHandle.write(str(n) + "\n")

occupiedMap = [[-1] * width for _ in range(height)]
dx = [0, 0, -1, 1]
dz = [-1, 1, 0, 0]

# generate each puzzle
for i in range(n):
    voxelNum = random.randint(minVoxelNum, maxVoxelNum)
    fileHandle.write(str(voxelNum) + "\n")

    possibleStartCoords = []

    for x in range(width):
        for z in range(height):
            if occupiedMap[x][z] == -1:
                possibleStartCoords.append((x, z))

    if len(possibleStartCoords) == 0:  # no puzzle pieces can be put
        break

    # select start coord
    startCoord = possibleStartCoords[random.randint(0, len(possibleStartCoords) - 1)]

    # expand the puzzle piece by BFS (select adjacent grids by chance)
    q = Queue(-1)
    vis = [[False] * width for _ in range(height)]
    q.put(startCoord)

    while not q.empty():
        front = q.get()
        occupiedMap[front[0]][front[1]] = i
        fileHandle.write(str(front[0]) + " " + str(front[1]) + "\n")

        if vis[front[0]][front[1]] == True:
            continue

        for d in range(4):
            nx = startCoord[0] + dx[d]
            nz = startCoord[1] + dz[d]
            if nx >= 0 and nx < width and nz >= 0 and nz < height and not vis[nx][nz]:
                vis[nx][nz] = True
                q.put((nx, nz))

        vis[front[0]][front[1]] = True
