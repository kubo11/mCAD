# mCAD


https://github.com/user-attachments/assets/057933f4-7147-44a4-93f3-b256a99cdf1a

![mcad1](https://github.com/user-attachments/assets/adf83454-3d7b-420c-98e2-6062052da41c)
![mcad2](https://github.com/user-attachments/assets/ae265243-2966-4e44-afc1-4249e2d1125b)

Minimal CAD.

## Requirements

- gcc/g++
- make
- cmake
- git

## Building

In order to build this project you have to download required submodules. This can be achieved by either adding `--recursive` flag to `git clone` command or by running the following command in an already cloned repository directory:
```
git submodule update --init --recursive
```

To build the project run inside the root repository directory:
```
mkdir build && cd build
cmake ..
cmake --build .
```

If you want to speed up the buliding process add `-jN` flag to the last command, where `N` is the number of paralell processes to run (for optimal performance do not run more processes than the number of threads in your system).

## Libraries

This project has been created with use of some other open-source dependencies:
- [mge](https://github.com/kubo11/mGE) - 0.0.1
