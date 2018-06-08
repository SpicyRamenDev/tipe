from PIL import Image
import random
from matplotlib import pyplot as pl
import scipy as sc
import numpy as np
import math
import time

import imgseg


def addnoise(filename, p=.25):
    img = Image.open(filename)
    noise = Image.new(img.mode, img.size)
    pixels = noise.load()
    for i in range(noise.height):
        for j in range(noise.width):
            pixels[i,j] = tuple(random.randrange(256) for _ in range(3)) + (int(256*p),)
    img.alpha_composite(noise)
    f = filename.split('/')
    img.save('/'.join(f[:-1] + ['/n_' + f[-1]]))
    img.close()



class ImgSeg:
    def __init__(self, source, beta=10):
        source = Image.open(source)
        self.size = source.size
        self.width, self.height = self.size
        self.graph = imgseg.Graph(self.height, self.width, beta)
        self.graph.setImg(source.getdata())
        source.close()

    def reset(self):
        self.graph.reset()

    def setBeta(self, beta):
        pass
    
    def loadSeedsImg(self, filename):
        seeds = Image.open(filename)
        self.labels = [px[:3] for c, px in seeds.getcolors() if px[-1] != 0]
        self.seedsLbl = [0] * (self.height * self.width)
        seedsData = seeds.getdata()
        seeds.close()
        for i in range(self.height * self.width):
            px = seedsData[i]
            if px[-1] == 0:
                self.seedsLbl[i] = -1
            else:
                for lbl, lblPx in enumerate(self.labels):
                    if px[:3] == lblPx:
                        self.seedsLbl[i] = lbl
                        break

        self.graph.createTransitions()
        self.graph.setSeeds(self.labels, self.seedsLbl)

    def cellular(self, itCount):
        self.graph.cellular(itCount)

    def directSolver(self, itCount):
        self.graph.directSolver(itCount)

    def getSegImg(self):
        seg = Image.new('RGBA', self.size)
        seg.putdata(self.graph.getSegImg())
        return seg

    def getSegImgChannel(self, label):
        seg = Image.new('RGBA', self.size)
        seg.putdata(self.graph.getSegImgChannel(label))
        return seg
    
    def getTrans(self):
        seg = Image.new('RGBA', self.size)
        seg.putdata(self.graph.getTrans())
        return seg


def test():
    g = ImgSeg('a.png')
    g.loadSeedsImg('sa.png')
    return g

def test2():
    g = ImgSeg('temp.png')
    g.loadSeedsImg('stemp.png')
    return g
