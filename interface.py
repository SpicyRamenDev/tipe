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
    graph.setSeeds(labels, seedsLbl)
    graph.createTransitions()

    graph.cellular(10000)

    seg = Image.new('RGBA', img.size)
    seg.putdata(graph.getSegImg())
    seg.show()
    
    
    return graph
