#include <Eigen/Sparse>

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
#include <cstring>

#include "imgseg.hxx"

using namespace std;
using namespace Eigen;


mt19937 rng;


class Graph;
struct Node;
struct Pixel;
	

void Pixel::set(int r, int g, int b)
{
  R = r;
  G = g;
  B = b;
}


void Node::calcDegree()
{
  degree = 0;
  for(int i = 0; i < 4; i++)
    {
      if(neighbours[i] != nullptr)
	degree += transitions[i];
    }
}
	
void Node::init(int labelCount)
{
  probabilities.resize(labelCount, 1. / labelCount);
  buffer = probabilities;
}

void Node::set(int lbl)
{
  isSeed = lbl == -1 ? false : true;
  if(lbl == -1)
    return;
  label = lbl;
  for(int i = 0; i < probabilities.size(); i++)
    probabilities[i] = 0;
  probabilities[lbl] = 1;
}

void Node::reset()
{
  if(isSeed)
    return;
		
  double p = 1. / probabilities.size();
  for(int i = 0; i < probabilities.size(); i++)
    {
      probabilities[i] = p;
    }
}
	
void Node::update()
{
  if(isSeed)
    return;
		
  double total = 0;
			
  for(int i = 0; i < buffer.size() - 1; i++)
    {
      buffer[i] = 0;
      for(int j = 0; j < 4; j++)
	{
	  if(neighbours[j] != nullptr)
	    {
	      buffer[i] += transitions[j] * neighbours[j]->probabilities[i] / degree;
	    }
	}
			
      total += buffer[i];
    }
		
  buffer[buffer.size() - 1] = 1 - total;
		
  double maxP = 0;
  for(int i = 0; i < buffer.size(); i++)
    {
      if(buffer[i] > maxP)
	{
	  maxP = buffer[i];
	  label = i;
	}
    }
}
	
void Node::swap()
{
  probabilities.swap(buffer);
}

Graph::Graph(int height, int width, double beta)
  :height(height), width(width), beta(beta)
{
  pixels = vector<Pixel>(height * width);
  nodes = vector<Node>(height * width);
}
	
void Graph::setImg(vector<vector<int>> px)
{
  for(int i = 0; i < height * width; i++)
    {
      pixels[i].set(px[i][0], px[i][1], px[i][2]);
    }
}	
	
void Graph::setSeeds(vector<vector<int>> lbls, vector<int> seeds)
{
  int labelCount = lbls.size();
  labels.resize(labelCount);
  for(int i = 0; i < labels.size(); i++)
    {
      labels[i].set(lbls[i][0], lbls[i][1], lbls[i][2]);
    }

  
  for(int i = 0; i < height * width; i++)
    {
      nodes[i].init(labelCount);
      nodes[i].set(seeds[i]);
    }
}	
	
bool Graph::isLegal(int i, int j)
{
  return i >= 0 && i < height && j >= 0 && j < width;
}
	
int Graph::getNode(int i, int j)
{
  if(!isLegal(i, j))
    return -1;
  return j + width * i;
}
	
int Graph::getNeighbour(int i, int j, int dir)
{		
  switch(dir)
    {
    case 0: return getNode(i, j + 1);
    case 1: return getNode(i + 1, j);
    case 2: return getNode(i, j - 1);
    case 3: return getNode(i - 1, j);
    default: return -1;
    }
}
	
double Graph::calcProb(Pixel pixelA, Pixel pixelB)
{
  int dR = (int)pixelA.R - (int)pixelB.R;
  int dG = (int)pixelA.G - (int)pixelB.G;
  int dB = (int)pixelA.B - (int)pixelB.B;
  double p = dR*dR + dG*dG + dB*dB;
  p /= 256 * 256;
  p = exp(-beta * p);
  return p;
}
	
void Graph::createTransitions()
{		
  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
	{
	  int n = getNode(i, j);
					
	  int neigh = getNeighbour(i, j, 0);
	  if (neigh >= 0)
	    {
	      double transition = calcProb(pixels[n], pixels[neigh]);
	      nodes[n].transitions[0] = transition;
	      nodes[n].neighbours[0] = &nodes[neigh];
	      nodes[neigh].transitions[2] = transition;
	      nodes[neigh].neighbours[2] = &nodes[n];
	    }
					
	  neigh = getNeighbour(i, j, 1);
	  if (neigh >= 0)
	    {
	      double transition = calcProb(pixels[n], pixels[neigh]);
	      nodes[n].transitions[1] = transition;
	      nodes[n].neighbours[1] = &nodes[neigh];
	      nodes[neigh].transitions[3] = transition;
	      nodes[neigh].neighbours[3] = &nodes[n];
	    }
					
	  nodes[n].calcDegree();
	}
    }
}
	
void Graph::reset()
{
  for(int i = 0; i < height * width; i++)
    {
      nodes[i].reset();
    }
}
	
void Graph::process(Mode mode, int itCount=0)
{
  switch(mode)
    {
    case Mode::cellular:
      cellular(itCount);
      break;
    case Mode::directSolver:
      directSolver();
      break;
    case Mode::randomWalks:
      randomWalks();
      break;
    }
}
	
void Graph::randomWalks()
{
		
}
	
void Graph::directSolver()
{
  SparseMatrix<double> lu(unseededPixelCount, unseededPixelCount);
  lu.reserve(VectorXi::Constant(unseededPixelCount,5));
  //		for each i,j such that v_ij != 0
  //			lu.insert(i,j) = v_ij;
  lu.makeCompressed();
		
  SparseMatrix<double> b(seededPixelCount, seededPixelCount);
  b.reserve(VectorXi::Constant(seededPixelCount,5));
  //		for each i,j such that v_ij != 0
  //			b.insert(i,j) = v_ij;
  b.makeCompressed();
}
	
void Graph::cellular(int itCount)
{		
  while (itCount-- > 0)
    {
      for (int i = 0; i < height * width; i++)
	{
	  nodes[i].update();
	}
			
      for (int i = 0; i < height * width; i++)
	{
	  nodes[i].swap();
	}
    }
}
	
vector<vector<int>> Graph::getSegImg()
{
  vector<vector<int>> px = vector<vector<int>>(height * width, vector<int>(4));
  for (int i = 0; i < height * width; i++)
    {
      px[i][0] = labels[nodes[i].label].R;
      px[i][1] = labels[nodes[i].label].G;
      px[i][2] = labels[nodes[i].label].B;
      px[i][3] = 255;
    }
  return px;
}
	
vector<vector<int>> Graph::getSegImgChannel(int label)
{		
  vector<vector<int>> px = vector<vector<int>>(height * width, vector<int>(4));
  for(int i = 0; i < height * width; i++)
    {
      px[i][0] = labels[nodes[i].label].R;
      px[i][1] = labels[nodes[i].label].G;
      px[i][2] = labels[nodes[i].label].B;
      px[i][3] = (int)(nodes[i].probabilities[label] * 255);
    }
  return px;
}
