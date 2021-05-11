from distutils.core import setup, Extension

def main():
    setup(name="Korbit",
          version="1.0.0",
          description="Python interface for the kepler C library function",
          author="cxa",
          author_email="1@qq.com",
          ext_modules=[Extension("Korbit", ["pythonExtend.cpp"])])

if __name__ == "__main__":
    main()