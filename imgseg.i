%module imgseg

%include "typemaps.i"
%include "std_vector.i"

%{
#define SWIG_FILE_WITH_INIT
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include "imgseg.hxx"
%}

namespace std {
  %template(VectorInt) vector<int>;
  %template(VectorDouble) vector<double>;
  %template(VectorInt2D) vector<vector<int>>;
  %template(VectorInt3D) vector<vector<vector<int>>>;
  %template(VectorChar) vector<unsigned char>;
  %template(VectorChar2D) vector<vector<unsigned char>>;
  %template(VectorNode) vector<Node>;
  %template(VectorPixel) vector<Pixel>;
  %template(VectorVector) vector<Eigen::VectorXd>;
};
%include <Eigen/Core>
%include <Eigen/Sparse>
%include <Eigen/Dense>
%include "imgseg.hxx"
