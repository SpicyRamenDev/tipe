from PIL import Image
import random
from matplotlib import pyplot as pl
import scipy as sc
import numpy as np
import math
import time

import imgseg


def addnoise():
    filename = input()
    img = Image.open(filename)
    noise = Image.new(img.mode, img.size)
    pixels = noise.load()
    for i in range(noise.height):
        for j in range(noise.width):
            pixels[i,j] = random.randrange(1<<32)
            noise.putalpha(64)
    img.alpha_composite(noise)
    img.save('noise_' + filename)
    img.close()



class ImgSeg:
    def __init__(self, source, beta=10):
        source = Image.open(source)
        self.size = source.size
        self.height, self.width = self.size
        self.graph = imgseg.Graph(self.height, self.width, beta)
        self.graph.setImg(source.getdata())
        source.close()

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

    def directSolver(self):
        self.graph.directSolver()

    def getSegImg(self):
        seg = Image.new('RGBA', self.size)
        seg.putdata(self.graph.getSegImg())
        return seg

    def getSegImgChannel(self, label):
        seg = Image.new('RGBA', self.size)
        seg.putdata(self.graph.getSegImgChannel(label))
        return seg

def main():
    src_name = 'a.png'
    seeds_name = 'sa.png'

    img = Image.open(src_name)
    seeds = Image.open(seeds_name)

    height, width = img.size
    
    labels = [px[:3] for c, px in seeds.getcolors() if px[-1] != 0]
    seedsLbl = [0] * (height * width)
    seedsData = seeds.getdata()
    for i in range(height * width):
        px = seedsData[i]
        if px[-1] == 0:
            seedsLbl[i] = -1
        else:
            for lbl, lblPx in enumerate(labels):
                if px[:3] == lblPx:
                    seedsLbl[i] = lbl
                    break

    graph = imgseg.Graph(height, width, 10)

    graph.setImg(img.getdata())
    graph.createTransitions()
    graph.setSeeds(labels, seedsLbl)

    graph.cellular(10000)

    seg = Image.new('RGBA', img.size)
    seg.putdata(graph.getSegImg())
    seg.show()

    img.close()
    seeds.close()    
    
    return graph


def test():
    g = ImgSeg('a.png')
    g.loadSeedsImg('sa.png')
    return g

def test2():
    g = ImgSeg('temp.png')
    g.loadSeedsImg('stemp.png')
    return g
