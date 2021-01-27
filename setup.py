from distutils.core import setup, Extension

def main():
    setup(name="kepler",
          version="1.0.0",
          description="Python interface for the kepler C library function",
          author="cxa",
          author_email="1598828268@qq.com",
          ext_modules=[Extension("kepler", ["pythonExtend.cpp"])])

if __name__ == "__main__":
    main()