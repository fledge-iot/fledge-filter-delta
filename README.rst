====================
FogLAMP delta Filter
====================

FogLAMP "delta" filter passes deltas of reading data. A new reading
is only sent onwards if the value of one or more data points in the new
reading differs from the previous reading sent by the specified tolerance
percentage.

By defining a minimum rate it is possible to force readings to be sent
at that defined rate when there is no change in the value of the reading.

Rates may be defined as per second, per minute, per hour or per day.

Configuration items
-------------------

The following configuration items are supported:

  tolerance
    The percentage tolerance when comparing reading data. Only values
    that differ by more than this percentage will be considered as different
    from each other.

  minRate
    The minimum rate at which readings should be sent. This is the rate at
    which readings will appear if there is no change in value.

  rateUnit
    The units in which minRate is define (per second, minute, hour or day)

Example
-------

Send only readings that differ by more than 1 percent from the the
previous reading sent or at a rate of one reading every half hour if
the change is less than this.

  tolerance
    1

  minRate
    2

  rateUnit
    per hour

Build
-----
To build FogLAMP "delta" C++ filter plugin:

.. code-block:: console

  $ mkdir build
  $ cd build
  $ cmake ..
  $ make

- By default the FogLAMP develop package header files and libraries
  are expected to be located in /usr/include/foglamp and /usr/lib/foglamp
- If **FOGLAMP_ROOT** env var is set and no -D options are set,
  the header files and libraries paths are pulled from the ones under the
  FOGLAMP_ROOT directory.
  Please note that you must first run 'make' in the FOGLAMP_ROOT directory.

You may also pass one or more of the following options to cmake to override 
this default behaviour:

- **FOGLAMP_SRC** sets the path of a FogLAMP source tree
- **FOGLAMP_INCLUDE** sets the path to FogLAMP header files
- **FOGLAMP_LIB sets** the path to FogLAMP libraries
- **FOGLAMP_INSTALL** sets the installation path of Random plugin

NOTE:
 - The **FOGLAMP_INCLUDE** option should point to a location where all the FogLAMP 
   header files have been installed in a single directory.
 - The **FOGLAMP_LIB** option should point to a location where all the FogLAMP
   libraries have been installed in a single directory.
 - 'make install' target is defined only when **FOGLAMP_INSTALL** is set

Examples:

- no options

  $ cmake ..

- no options and FOGLAMP_ROOT set

  $ export FOGLAMP_ROOT=/some_foglamp_setup

  $ cmake ..

- set FOGLAMP_SRC

  $ cmake -DFOGLAMP_SRC=/home/source/develop/FogLAMP  ..

- set FOGLAMP_INCLUDE

  $ cmake -DFOGLAMP_INCLUDE=/dev-package/include ..
- set FOGLAMP_LIB

  $ cmake -DFOGLAMP_LIB=/home/dev/package/lib ..
- set FOGLAMP_INSTALL

  $ cmake -DFOGLAMP_INSTALL=/home/source/develop/FogLAMP ..

  $ cmake -DFOGLAMP_INSTALL=/usr/local/foglamp ..
