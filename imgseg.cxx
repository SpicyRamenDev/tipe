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
	

void Pixel::set(char r, char g, char b)
{
  R = r;
  G = g;
  B = b;
}


void Node::calcDegree()
{
  degree = 0;
  for(int i = 0; i < 6; i++)
    {
      if(neighbours[i] != nullptr)
	degree += transitions[i];
    }
}
	
void Node::init(int labelCount)
{
  if(isSeed)
    {
      probabilities.resize(labelCount, 0);
      probabilities[label] = 1;
    }
  else
    {
      probabilities.resize(labelCount, 1. / labelCount);
    }
  buffer = probabilities;
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
      for(int j = 0; j < 6; j++)
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


Graph::Graph(int height, int width, int depth, double beta)
  :height(height), width(width), depth(depth), beta(beta)
{
  pixels = vector<Pixel>(height * width * depth);
  nodes = vector<Node>(height * width * depth);
}
	
void Graph::setImg(vector<vector<vector<int>>> px)
{
  for(int i = 0; i < depth; i++)
    {
      for(int j = 0; j < height * width; j++)
	{
	  pixels[j + height * width * i].set(px[i][j][0], px[i][j][0], px[i][j][0]);
	}
    }
}	
	
void Graph::setSeeds(vector<vector<int>> lbls)
{
  labels.resize(lbls.size());
  for(int i = 0; i < labels.size(); i++)
    {
      labels[i].set(lbls[i][0], lbls[i][1], lbls[i][2]);
    }
  //
  //		for (int i = 0; i < height * width * depth; i++)
  //		{
  //			Uint32 pixel = pixels[i];
  //			if ((pixel & fmt->Amask) == 0)
  //			{
  //				nodes[i].label = -1;
  //				continue;
  //			}
  //			
  //			Uint8 lbl;
  //			for (lbl = 0; lbl < labels.size() && pixel != labels[lbl]; lbl++) {}
  //			if (lbl == labels.size())
  //				labels.push_back(pixel);
  //				
  //			nodes[i].isSeed = true;
  //			nodes[i].label = lbl;
  //		}
  //		
  //		labelCount = labels.size();
  //		
  //		for(int i = 0; i < height * width * depth; i++)
  //		{
  //			nodes[i].init(labelCount);
  //		}
}	
	
bool Graph::isLegal(int i, int j, int k)
{
  return i >= 0 && i < height && j >= 0 && j < width && k >= 0 && k < depth;
}
	
int Graph::getNode(int i, int j, int k)
{
  if(!isLegal(i, j, k))
    return -1;
  return j + width * i + height*width * k;
}
	
int Graph::getNeighbour(int i, int j, int k, int dir)
{		
  switch(dir)
    {
    case 0: return getNode(i, j + 1, k);
    case 1: return getNode(i + 1, j, k);
    case 2: return getNode(i, j - 1, k);
    case 3: return getNode(i - 1, j, k);
    case 4: return getNode(i, j, k + 1);
    case 5: return getNode(i, j, k - 1);
    default: return -1;
    }
}
	
double Graph::calcProb(const Pixel& pixelA, const Pixel& pixelB)
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
	  for(int k = 0; k < depth; k++)
	    {
	      int n = getNode(i, j, k);
					
	      int neigh = getNeighbour(i, j, k, 0);
	      if (neigh >= 0)
		{
		  double transition = calcProb(pixels[n], pixels[neigh]);
		  nodes[n].transitions[0] = transition;
		  nodes[n].neighbours[0] = &nodes[neigh];
		  nodes[neigh].transitions[2] = transition;
		  nodes[neigh].neighbours[2] = &nodes[n];
		}
					
	      neigh = getNeighbour(i, j, k, 1);
	      if (neigh >= 0)
		{
		  double transition = calcProb(pixels[n], pixels[neigh]);
		  nodes[n].transitions[1] = transition;
		  nodes[n].neighbours[1] = &nodes[neigh];
		  nodes[neigh].transitions[3] = transition;
		  nodes[neigh].neighbours[3] = &nodes[n];
		}
					
	      neigh = getNeighbour(i, j, k, 4);
	      if (neigh >= 0)
		{
		  double transition = calcProb(pixels[n], pixels[neigh]);
		  nodes[n].transitions[4] = transition;
		  nodes[n].neighbours[4] = &nodes[neigh];
		  nodes[neigh].transitions[5] = transition;
		  nodes[neigh].neighbours[5] = &nodes[n];
		}
					
	      nodes[n].calcDegree();
	    }
	}
    }
}
	
void Graph::reset()
{
  for(int i = 0; i < height * width * depth; i++)
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
      for (int i = 0; i < height * width * depth; i++)
	{
	  nodes[i].update();
	}
			
      for (int i = 0; i < height * width * depth; i++)
	{
	  nodes[i].swap();
	}
    }
}
	
vector<vector<int>> Graph::getSegImg(int depth=0)
{
  vector<vector<int>> px = vector<vector<int>>(height * width, vector<int>(4));
  for (int i = 0; i < height * width; i++)
    {
      int n = i + height * width * depth;
      px[i][0] = labels[nodes[i].label].R;
      px[i][1] = labels[nodes[i].label].G;
      px[i][2] = labels[nodes[i].label].B;
      px[i][3] = 255;
    }
  return px;
}
	
vector<vector<int>> Graph::getSegImgChannel(int label, int depth=0)
{		
  vector<vector<int>> px = vector<vector<int>>(height * width, vector<int>(4));
  for(int i = 0; i < height * width; i++)
    {
      int n = i + height * width * depth;
      px[i][0] = labels[nodes[i].label].R;
      px[i][1] = labels[nodes[i].label].G;
      px[i][2] = labels[nodes[i].label].B;
      px[i][3] = (char)(nodes[i].probabilities[label] * 255);
    }
  return px;
}
