# influxopmon - Influx DB banse plugin for Operational Monitoring
The library converts a JSON object into an influxdb insert statement, then inserts the statement to an influx database.

### Building and running :
The library's constructor takes an URI argument. The URI's syntax is the following (without spaces):
Application name: influx://host name:port:database name:database account name:database account password:protocol (http or https):json delimiter (usually time):influx db tags (0..N tags separated by ':').


The library should be used calling the library's "publish" function with as argument a "nlohmann::json" object.


The console output is the system/database reply to the curl insert statement.

The system output is the insert statement to the DBMS.

### URI example :
influx://dbod-testinfluxyd.cern.ch:8095:db1:usr:pwd:https:.time=:.class_name=

### Notes :
The database is querried using the system's curl library.

In dbt-build-order.cmake, "influxopmon" should be added after "opmonlib".

Influxopmon changes, if necessary, UNIX EPOCH time format to UNIX EPOCH time format in nanoseconds.
