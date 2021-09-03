import sys
import os
import pathlib
from os import listdir
from os.path import isfile, join

if len(sys.argv)==1:
    print("Folder!")
    exit()
path = str(pathlib.Path.cwd().parent) + "/" + sys.argv[1]

onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]

for file in onlyfiles:
    print(file)
    # d = []
    zeroed = 0
    with open(path+ "/" + file) as file:
        for line in file:
            v = int(line)
            if (v==0):
                zeroed += 1
            # d.append(v)
    print(zeroed)
    # print(len(d))