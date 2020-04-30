
# How to generate the files

By [Kitware SAS](https://www.kitware.eu), 2020

node-js with npm is required to generate the files. The following packages are required:

* hexo-cli
* kw-doc

To generate html files and to test it with hexo server, follow the instructions below:

1. git clone https://gitlab.kitware.com/f3d/f3d.git
2. cd documentation
3. kw-doc -c config.js -m
4. cd build-tmp
5. hexo server
