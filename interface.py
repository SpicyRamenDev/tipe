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
    img.save('/'.join(f[:-1] + ['n_' + f[-1]]))
    img.close()



class ImgSeg:
    def __init__(self, source, beta=10):
        source = Image.open(source)
        self.size = source.size
        self.width, self.height = self.size
        self.graph = imgseg.Graph(self.height, self.width, beta, False)
        self.pixels = list(source.getdata())
        self.graph.setImg(self.pixels)
        source.close()       

    def reset(self):
        self.graph.reset()

    def setBeta(self, beta):
        self.graph.setBeta(beta)

    def setFW(self, fw):
        self.graph.setFW(fw)
    
    def loadSeedsImg(self, filename):
        def mean(i):
            x, y = i%self.width, i//self.height
            c = 1
            m = self.pixels[i][:3]
            for dx,dy in [(0,1),(1,0),(-1,0),(-1,0)]:
                if 0<=x+dx<self.height and 0<=y+dy<self.width:
                    c += 1
                    m = [m[j] + self.pixels[y+dy+(x+dx)*8][j] for j in range(3)]
            m = [m[j] / c for j in range(3)]
            return m
        
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

        self.graph.setSeeds(self.labels, self.seedsLbl)
        self.graph.createTransitions()

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

    def getRandomWalk(self, i, j, color, max=10000):
        path = self.graph.randomWalk(i, j, max)
        seg = Image.new('RGBA', self.size)
        data = list(seg.getdata())
        for n in path:
            data[n] = color
        seg.putdata(data)
        return seg, len(path)


def test():
    g = ImgSeg('a.png')
    g.loadSeedsImg('sa.png')
    return g

def test2():
    g = ImgSeg('temp.png')
    g.loadSeedsImg('stemp.png')
    return g
