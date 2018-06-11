#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
#include <cstring>
#include <stdlib.h>
#include <time.h>

#include "imgseg.hxx"

using namespace std;
using namespace Eigen;


class Graph;
struct Node;
struct Pixel;


void Pixel::set(int r, int g, int b)
{
  R = r;
  G = g;
  B = b;
}

double Pixel::distSqr(Pixel p)
{
  double d = (R-p.R)*(R-p.R)+(G-p.G)*(G-p.G)+(B-p.B)*(B-p.B);
  return d / (256 * 256);
}

double Pixel::dist(Pixel p)
{
  return sqrt(distSqr(p));
}

int Pixel::getBin()
{
  return R/32+(G/32)*8+(B/32)*64;
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
}

void Node::calcLabel()
{
  if(isSeed)
    return;
  double maxP = 0;
  for(int i = 0; i < probabilities.size(); i++)
    {
      if(probabilities[i] > maxP)
	{
	  maxP = probabilities[i];
	  label = i;
	}
    }
}

void Node::swap()
{
  probabilities.swap(buffer);
}

Node* Node::randomNext()
{
  float r = (float) (rand()) / (float) (RAND_MAX);
  Node *last = nullptr;
  for(int i = 0; i < 4; i++)
    {
      if(neighbours[i] != nullptr)
	{
	  r -= transitions[i] / degree;
	  if(r <= 0)
	    return neighbours[i];
	  last = neighbours[i];
	}
    }
  return last;
}

Graph::Graph(int height, int width, double beta, bool isFW)
  :height(height), width(width), beta(beta), isFW(isFW)
{
  srand(time(NULL));
  
  pixels = vector<Pixel>(height * width);
  nodes = vector<Node>(height * width);
  for(int i = 0; i < height * width; i++)
    nodes[i].index = i;
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
  labelCount = lbls.size();
  labels.resize(labelCount);
  for(int i = 0; i < labels.size(); i++)
    {
      labels[i].set(lbls[i][0], lbls[i][1], lbls[i][2]);
    }

  for(int i = 0; i < 512; i++)
    for(int j = 0; j < 512; j++)
      {
	double r = (i%8-j%8), g = ((i/8)%8-(j/8)%8), b = (i/64-j/64);
	fw[i][j] = sqrt(r*r + g*g + b*b) / 7 * 224 / 256;
      }

  vector<vector<Pixel>> unseeded(labelCount);
  seededCount = 0;
  unseededCount = 0;
  for(int i = 0; i < height * width; i++)
    {
      nodes[i].init(labelCount);
      nodes[i].set(seeds[i]);
      if(seeds[i] == -1)
	{
	  nodes[i].cmpIndex = unseededCount;
	  unseededCount++;
	}
      else
	{
	  nodes[i].cmpIndex = seededCount;
	  seededCount++;
	  
	  for(int j = 0; j < unseeded[seeds[i]].size(); j++)
	    fw[pixels[i].getBin()][unseeded[seeds[i]][j].getBin()] =
	      fw[unseeded[seeds[i]][j].getBin()][pixels[i].getBin()] = 0;
	  unseeded[seeds[i]].push_back(pixels[i]);
	}
    }
  
  for(int k = 0; k < 512; k++)
    for(int i = 0; i < 512; i++)
      for(int j = 0; j < 512; j++)
	if(fw[i][j] > fw[i][k] + fw[k][j])
	  fw[i][j] = fw[i][k] + fw[k][j];
  
  for(int i = 0; i < 512; i++)
    for(int j = 0; j < 512; j++)
      {
	cout << fw[i][j] << " ";
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
  double p;
  if(!isFW)
    p = pixelA.distSqr(pixelB);
  else
    {
      p = fw[pixelA.getBin()][pixelB.getBin()];
      p *= p;
    }
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
	  for(int k = 0; k < 4; k++)
	    {
	      int neigh = getNeighbour(i, j, k);
	      if(neigh == -1)
		 continue;
	      double transition = calcProb(pixels[n], pixels[neigh]);
	      nodes[n].transitions[k] = transition;
	      nodes[n].neighbours[k] = &nodes[neigh];
	    }
	  nodes[n].calcDegree();
	}
    }
}

void Graph::setBeta(double b)
{
  beta = b;
  createTransitions();
}

void Graph::setFW(bool b)
{
  isFW = b;
  createTransitions();
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
      directSolver(itCount);
      break;
    case Mode::randomWalks:
      randomWalks(itCount);
      break;
    }
}

vector<int> Graph::randomWalk(int i, int j, int maxCount = 100000)
{
  vector<int> path;
  int itCount = 0;
  int n = getNode(i, j);
  Node *node = &nodes[n];
  while(!node->isSeed && itCount++ < maxCount)
    {
      path.push_back(node->index);
      node = node->randomNext();
    }
  return path;
}

void Graph::randomWalks(int itCount)
{
  
}

void Graph::directSolver(int itCount)
{
  SparseMatrix<double> L(unseededCount, unseededCount);
  L.reserve(VectorXi::Constant(unseededCount,5));
  for(int i = 0; i < height * width; i++)
    {
      if(nodes[i].isSeed)
	continue;
      for(int k = 0; k < 4; k++)
	if(nodes[i].neighbours[k] != nullptr && !nodes[i].neighbours[k]->isSeed)
	  L.insert(nodes[i].cmpIndex, nodes[i].neighbours[k]->cmpIndex) = -nodes[i].transitions[k];
      L.insert(nodes[i].cmpIndex, nodes[i].cmpIndex) = nodes[i].degree;
    }
  L.makeCompressed();

  SparseMatrix<double> B(seededCount, unseededCount);
  B.reserve(VectorXi::Constant(seededCount,4));
  for(int i = 0; i < height * width; i++)
    {
      if(nodes[i].isSeed)
	for(int k = 0; k < 4; k++)
	  if(nodes[i].neighbours[k] != nullptr && !nodes[i].neighbours[k]->isSeed)
	    B.insert(nodes[i].cmpIndex, nodes[i].neighbours[k]->cmpIndex) = -nodes[i].transitions[k];
    }
  B.makeCompressed();

  MatrixXd x(unseededCount, labelCount - 1);
  MatrixXd xSeeds(seededCount, labelCount - 1);
  for(int i = 0; i < height * width; i++)
    {
      for(int j = 0; j < labelCount - 1; j++)
	{
	  if(nodes[i].isSeed)
	    xSeeds(nodes[i].cmpIndex, j) = nodes[i].label == j ? 1 : 0;
	  else
	    x(nodes[i].cmpIndex, j) = nodes[j].probabilities[j];
	}
    }

  LeastSquaresConjugateGradient<SparseMatrix<double>> lscg;
  lscg.setMaxIterations(itCount);
  lscg.compute(L);
  for(int i = 0; i < labelCount - 1; i++)
    {
      x.col(i) = lscg.solveWithGuess(-B.transpose() * xSeeds.col(i), x.col(i));
      cout << "#iterations:     " << lscg.iterations() << endl;
    }

  for(int i = 0; i < height * width; i++)
    {
      if(nodes[i].isSeed)
  	continue;
      double total = 0;
      for(int j = 0; j < labelCount - 1; j++)
  	{
  	  double p = x(nodes[i].cmpIndex, j);
  	  nodes[i].probabilities[j] = p;
  	  total += p;
  	}
      nodes[i].probabilities[labelCount - 1] = 1 - total;
    }      
}

void Graph::cellular(int itCount)
{
  while (itCount-- > 0)
    {
      for (int i = 0; i < height * width; i++)
	  nodes[i].update();

      for (int i = 0; i < height * width; i++)
	  nodes[i].swap();
    }
}

vector<vector<int>> Graph::getSegImg()
{
  for(int i = 0; i < height * width; i++)
    nodes[i].calcLabel();
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
  for(int i = 0; i < height * width; i++)
    nodes[i].calcLabel();
  vector<vector<int>> px = vector<vector<int>>(height * width, vector<int>(4));
  for(int i = 0; i < height * width; i++)
    {
      px[i][0] = labels[label].R;
      px[i][1] = labels[label].G;
      px[i][2] = labels[label].B;
      px[i][3] = (int)(nodes[i].probabilities[label] * 255);
    }
  return px;
}

vector<vector<int>> Graph::getTrans()
{
  vector<vector<int>> px = vector<vector<int>>(height * width, vector<int>(4));
  for (int i = 0; i < height * width; i++)
    {
      px[i][0] = px[i][1] = px[i][2] = 255;
      px[i][3] = (int)(255 * nodes[i].degree / labelCount);
    }
  return px;
}
  
