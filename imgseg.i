%module imgseg

%include "typemaps.i"
%include "std_vector.i"

%{
  #define SWIG_FILE_WITH_INIT
  #include "imgseg.hxx"
%}

namespace std {
  %template(VectorInt) vector<int>;
  %template(VectorDouble) vector<double>;
  %template(VectorInt2D) vector<vector<int>>;
  %template(VectorInt3D) vector<vector<vector<int>>>;
  %template(VectorNode) vector<Node>;
  %template(VectorPixel) vector<Pixel>;
};


%include "imgseg.hxx"
