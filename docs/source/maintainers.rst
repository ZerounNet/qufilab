
For maintainers
===============

Adding new indicators
*********************
The process of adding a new indicator to the library consists of several steps
and is documented here for future reference.

1. When creating a new indicator, start by declaring what type of indicator 
   it is. For example, the rsi indicator is a momentum indicator,
   and hence should be implemented in the *momentum.cc* file. The first line
   of the docstring in the *.cc* file should be **Implementation of INDICATOR**, i.e.
   **Implementation of SMA**. This is needed for the linking of source code to github,
   since *docs/source/conf.py* manually retrieves the source code from github and searches
   for that line to get the correct line number.
   If the indicator have dependencies from other functions in other script, be sure to include
   the header files and update *qufilab/indicators/CMakeLists.txt*. 
   This is needed when building manually, since the linking won't work otherwise.

2. When the implementation is done, the documentation for the indicator needs
   to be created. Start by adding the indicator to the correct entry
   in the *docs/source/indicators.yaml* file. This file ensures that correct 
   links to the underlying source code can be made when building the docs.

3. Add the correct specifications to the *docs/source/indicators.rst* directory. 
