from distutils.core import setup, Extension


imgseg_module = Extension('_imgseg',
                          include_dirs=['/usr/local/include'],
                          sources=['imgseg_wrap.cxx', 'imgseg.cxx'],
                          language='c++'
)

setup (name = 'imgseg',
       version = '0.1',
       author      = "SWIG Docs",
       description = """Simple swig example from docs""",
       ext_modules = [imgseg_module],
       py_modules = ["imgseg"],
)
