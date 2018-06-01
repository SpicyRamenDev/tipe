all:
	rm -f *.so *.o *_wrap.* *.pyc
	swig -c++ -python -I/usr/local/include imgseg.i
	# g++ -lstdc++ -fPIC -c imgseg.hxx imgseg_wrap.cxx imgseg.cxx -I/usr/include/python3.6 -I/usr/local/include
	# g++ -lstdc++ -shared imgseg.o imgseg_wrap.o -o _imgseg.so
	python3 setup.py build_ext --inplace

clean:
	rm -f -r __pycache__ build
	rm -f *.so *.o *_wrap.* *.pyc
