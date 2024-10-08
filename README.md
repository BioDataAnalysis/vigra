VIGRA Computer Vision Library
=============================

[![Build Status](https://dev.azure.com/ullrichkoethe/vigra/_apis/build/status/ukoethe.vigra)](https://dev.azure.com/ullrichkoethe/vigra/_build/latest?definitionId=1)

                Copyright 1998-2013 by Ullrich Koethe


    This file is part of the VIGRA computer vision library.
    You may use, modify, and distribute this software according
    to the terms stated in the LICENSE.txt file included in
    the VIGRA distribution.

    The VIGRA Website is
        http://ukoethe.github.io/vigra/
    Please direct questions, bug reports, and contributions to
        ullrich.koethe@iwr.uni-heidelberg.de    or
        vigra@informatik.uni-hamburg.de


    THIS SOFTWARE IS PROVIDED AS IS AND WITHOUT ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
    WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.


Installation
------------

Installation instructions can be found in the file
```
$VIGRA_PATH/doc/vigra/Installation.html
```
If the documentation has not yet been generated (e.g. when you build from a development
snapshot), you find these instructions in
```
$VIGRA_PATH/docsrc/installation.dxx
```
or online at
  http://ukoethe.github.io/vigra/doc-release/vigra/Installation.html

Documentation
-------------

If you downloaded an official release, the documentation can be found in `$VIGRA_PATH/doc/vigra/`, the start file
is `$VIGRA_PATH/doc/vigra/index.html` or online at http://ukoethe.github.io/vigra/#documentation.

When you use the development version from github, you can generate documentation by `make doc`.

Download
--------

VIGRA can be downloaded at http://ukoethe.github.io/vigra/#download. The official development
repository is at https://github.com/ukoethe/vigra

What is VIGRA
-------------

VIGRA is a computer vision library that puts its main emphasis on flexible
algorithms, because algorithms represent the principal know-how of this field.
The library was consequently built using generic programming as introduced by
Stepanov and Musser and exemplified in the C++ Standard Template Library. By
writing a few adapters (image iterators and accessors) you can use VIGRA's
algorithms on top of your data structures, within your environment.
Alternatively, you can also use the data structures provided within VIGRA,
which can be easily adapted to a wide range of applications. VIGRA's
flexibility comes almost for free: Since the design uses compile-time
polymorphism (templates), performance of the compiled program approaches that
of a traditional, hand tuned, inflexible, solution.


Making a New Release
--------------------

1. Update the version in the header file. `include/vigra/config_version.hxx`
2. Create a short release in `docsrc/credits_changelog.dxx`.
3. Create a merge request.
4. Give "reasonable time" for others to chime in.
5. Create a tag on the main branch. The tag should follow `Version-MAJOR-MINOR-PATCH` format.
