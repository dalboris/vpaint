#!/bin/bash

rm -R out
mkdir out

rm -R src
mkdir src
cp -R ../src/OpenVac/ src/

FILES=$(find src/ -name '*.h')

# Extract macros to expand
echo "Extracting macros..."
cat $FILES > out/concatenated.h
python expandmacros-extract.py
echo "Done."

# Expand macros
echo ""
echo "Expanding macros..."

for f in $FILES
do
    echo "Processing $f"

    # Guard preprocessor directives so that they are not processed
    cp $f temp.h
    python expandmacros-guardmacros.py
    
    # Concatenate with extracted macros
    cat out/extracted.h temp2.h > temp3.h
    
    # Apply preprocessor
    #   P:  Inhibit generation of linemarkers in the output from the preprocessor.
    #   C: Do not discard comments, including during macro expansion. #WEIRD INTRO COMMENT
    #   CC: Do not discard comments, including during macro expansion. #BREAKS AT /* No Prefix */
    cpp -P temp3.h > temp4.h
    
    # Remove ___GUARD___
    sed 's/___GUARD___//g' temp4.h > temp5.h
    
    # replace ___NEWLINE___ by an actual newline
    sed -i 's/___NEWLINE___/\
    /g' temp5.h
    
    # Remove all empty lines
    sed -i '/^\s*$/d' temp5.h
    
    # Fix indentation
    #  -q: quiet
    #  -w: indent macro continuation lines #BROKEN
    cp temp5.h temp6.h
    astyle temp6.h --quiet

    # Copy to destination
    OUT=out/${f:4}          # out/OpenVac/Topology/Cell.h
    DIR=$(dirname "${OUT}") # out/OpenVac/Topology
    mkdir -p $DIR
    cp temp6.h $OUT
done
echo "Done."

rm temp.h temp2.h temp3.h temp4.h temp5.h temp6.h temp6.h.orig
