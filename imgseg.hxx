#ifndef IMGSEG_H_
#define IMGSEG_H_

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <random>
#include <math.h>

using namespace std;
// using namespace Eigen;

struct Pixel
{
  int R, G, B;
  void set(int r, int g, int b);
};


struct Node
{
  int index = 0;
  int cmpIndex = 0;
  double degree = 0;
  double transitions[4] = {};
  Node *neighbours[4] = {};
  vector<double> buffer = {};
  vector<double> probabilities = {};
  int label = 0;
  bool isSeed = false;
  int itCount = 0;

  void calcDegree();
  void init(int labelCount);
  void set(int lbl);
  void reset();
  void update();
  void calcLabel();
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

  int seededCount;
  int unseededCount;

  // Eigen::MatrixXd x;
  // Eigen::MatrixXd xSeeds;
  // Eigen::SparseMatrix<double> B;
  // Eigen::SparseMatrix<double> L;

  Graph() {}
  ~Graph() {}

  Graph(int height, int width, double beta);

  void setImg(vector<vector<int>> px);
  void setSeeds(vector<vector<int>> lbls, vector<int> seeds);
  bool isLegal(int i, int j);
  int getNode(int i, int j);
  int getNeighbour(int i, int j, int dir);
  double calcProb(Pixel pixelA, Pixel pixelB);
  void createTransitions();
  void reset();
  void process(Mode mode, int itCount);
  void randomWalks(int itCount);
  void directSolver();
  void cellular(int itCount);
  vector<vector<int>> getSegImg();
  vector<vector<int>> getSegImgChannel(int label);
};


#endif
