#!/bin/bash

rm -R html
rm -R dox
doxygen Doxyfile
python postprocess.py
cp -R dox html/dox
echo "Done!"
