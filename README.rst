====================
Fledge delta Filter
====================

Fledge "delta" filter passes deltas of reading data. A new reading
is only sent onwards if the value of one or more data points in the new
reading differs from the previous reading sent by the specified tolerance
percentage/value.

By defining a minimum rate it is possible to force readings to be sent
at that defined rate when there is no change in the value of the reading.

Rates may be defined as per second, per minute, per hour or per day.

Configuration items
-------------------

The following configuration items are supported:

  toleranceMeasure
    Tells whether 'tolerance' is specified as a percentage or an absolute value.

  tolerance
    The percentage tolerance when comparing reading data. Only values
    that differ by more than this percentage will be considered as different
    from each other.

  processingMode
    Controls when the delta filter outputs a reading and what to include in this 
    output reading. This is an enumeration with following options:
        a. Include full reading if any Datapoint exceeds tolerance
        b. Include full reading if all Datapoints exceed tolerance
        c. Include only the Datapoints that exceed tolerance

  minRate
    The minimum rate at which readings should be sent. This is the rate at
    which readings will appear if there is no change in value.

  rateUnit
    The units in which minRate is define (per second, minute, hour or day)

  overrides
    A JSON document that can be used to define specific tolerance values for an 
    asset. This is defined as a set of name/value pairs for those assets that 
    should use a tolerance percentage/value other than the global tolerance value 
    specified above.

Example
-------

Send only readings that differ by more than 1 percent from the the
previous reading sent or at a rate of one reading every half hour if
the change is less than this.

  toleranceMeasure
    percentage

  tolerance
    1

  processingMode
    Include full reading if any Datapoint exceeds tolerance

  minRate
    2

  rateUnit
    per hour

  overrides
    { "temperature" : 15, "pressure" : 5 }

Build
-----
To build Fledge "delta" C++ filter plugin:

.. code-block:: console

  $ mkdir build
  $ cd build
  $ cmake ..
  $ make

- By default the Fledge develop package header files and libraries
  are expected to be located in /usr/include/fledge and /usr/lib/fledge
- If **FLEDGE_ROOT** env var is set and no -D options are set,
  the header files and libraries paths are pulled from the ones under the
  FLEDGE_ROOT directory.
  Please note that you must first run 'make' in the FLEDGE_ROOT directory.

You may also pass one or more of the following options to cmake to override 
this default behaviour:

- **FLEDGE_SRC** sets the path of a Fledge source tree
- **FLEDGE_INCLUDE** sets the path to Fledge header files
- **FLEDGE_LIB sets** the path to Fledge libraries
- **FLEDGE_INSTALL** sets the installation path of Random plugin

NOTE:
 - The **FLEDGE_INCLUDE** option should point to a location where all the Fledge 
   header files have been installed in a single directory.
 - The **FLEDGE_LIB** option should point to a location where all the Fledge
   libraries have been installed in a single directory.
 - 'make install' target is defined only when **FLEDGE_INSTALL** is set

Examples:

- no options

  $ cmake ..

- no options and FLEDGE_ROOT set

  $ export FLEDGE_ROOT=/some_fledge_setup

  $ cmake ..

- set FLEDGE_SRC

  $ cmake -DFLEDGE_SRC=/home/source/develop/Fledge  ..

- set FLEDGE_INCLUDE

  $ cmake -DFLEDGE_INCLUDE=/dev-package/include ..
- set FLEDGE_LIB

  $ cmake -DFLEDGE_LIB=/home/dev/package/lib ..
- set FLEDGE_INSTALL

  $ cmake -DFLEDGE_INSTALL=/home/source/develop/Fledge ..

  $ cmake -DFLEDGE_INSTALL=/usr/local/fledge ..
