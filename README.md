# influxopmon - Influx DB banse plugin for Operational Monitoring
The library converts a JSON object into an influxdb insert statement, then send the insert statement to an influx database.

### Building and running :
The library's constructor takes an URI argument. The URI's syntax is the database :
Application name: influx://<host>:<port>/<endpoint>?db=<mydb>. Authentication is managed using a reverse proxy, meaning that if the database should be accessed using authentication, the endpoint and port should be setfor the reverse proxy.

The library should be used calling the library's "publish" function with as argument a "nlohmann::json" object.
  
There are no console outputs others than errors. 
  
The system output is the insert statement to the DBMS.

### Notes :
The database is querried using cpr.

In dbt-build-order.cmake, "influxopmon" should be added after "opmonlib".

Influxopmon changes, if necessary, UNIX EPOCH time format to UNIX EPOCH time format in nanoseconds.
