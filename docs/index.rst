.. Images
.. |delta| image:: images/delta.jpg

Delta Filter
============

The *fledge-filter-delta* is a filter that only forwards data that changes by more than a configurable percentage. It is used to remove duplicate data values from an asset stream. The definition of duplicate however allows for some noise in the reading value by requiring a delta percentage.

By defining a minimum rate it is possible to force readings to be sent at that defined rate when there is no change in the value of the reading. Rates may be defined as per second, per minute, per hour or per day.

Delta filters are added in the same way as any other filters.

  - Click on the Applications add icon for your service or task.

  - Select the *delta* plugin from the list of available plugins.

  - Name your delta filter.

  - Click *Next* and you will be presented with the following configuration page

+---------+
| |delta| |
+---------+

  - Configure the parameters of the delta filter

    - **Tolerance %**:  The percentage tolerance when comparing reading data. Only values that differ by more than this percentage will be considered as different from each other.

    - **Minimum Rate**: The minimum rate at which readings should be sent. This is the rate at which readings will appear if there is no change in value.

    - **Minimum Rate Units**: The units in which minimum rate is define (per second, minute, hour or day)

    - **Individual Tolerances**: A JSON document that can be used to define specific tolerance values for an asset. This is defines as a set of name/value pairs for those assets that should use a tolerance percentage other than the global tolerances specified above. The following example would set the tolerance for the temperature asset to 15% and for the pressure asset to 5%. All other assets would use the tolerance specified in *Tolerance %*.

      .. code-block:: json

         {
             "temperature" : 15,
             "pressure" : 5
         }

  - Enable the filter and click *Done* to complete the process of adding the new filter.
