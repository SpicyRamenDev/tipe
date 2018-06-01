#ifndef IMGSEG_H_
#define IMGSEG_H_

#include <Eigen/Sparse>
#include <iostream>
#include <vector>
#include <random>
#include <math.h>

using namespace std;
// using namespace Eigen;

struct Pixel
{
  char R, G, B;
  void set(char r, char g, char b);
};


struct Node
{
  double degree = 0;
  double transitions[6] = {};
  Node *neighbours[6] = {};
  vector<double> buffer = {};
  vector<double> probabilities = {};
  int label = 0;
  bool isSeed = false;

  void calcDegree();
  void init(int labelCount);
  void reset();
  void update();
  void swap();
};


enum Mode
  {
    cellular,
    directSolver,
    randomWalks
  };


class Graph
{
 public:
  int height;
  int width;
  int depth;
	
  vector<Pixel> pixels;
	
  vector<Pixel> labels;
  int labelCount;
  vector<Node> nodes;
  double beta;
	
  int seededPixelCount;
  int unseededPixelCount;
  vector<int> seededPixels;
  vector<int> unseededPixels;

  Graph() {}
  ~Graph() {}

  Graph(int height, int width, int depth, double beta);

  void setImg(vector<vector<vector<int>>> px);
  void setSeeds(vector<vector<int>> lbls);
  bool isLegal(int i, int j, int k);
  int getNode(int i, int j, int k);
  int getNeighbour(int i, int j, int k, int dir);
  double calcProb(const Pixel& pixelA, const Pixel& pixelB);
  void createTransitions();
  void reset();
  void process(Mode mode, int itCount);
  void randomWalks();
  void directSolver();
  void cellular(int itCount);
  vector<vector<int>> getSegImg(int depth);
  vector<vector<int>> getSegImgChannel(int label, int depth);
};  


#endif
