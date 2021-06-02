# influxopmon - Influx DB banse plugin for Operational Monitoring
The library converts a JSON object into an influxdb insert statement, then send the insert statement to an influx database.

##Building and running :
The library's constructor takes an URI argument. The URI's syntax is the database :
Application name: `influxdb://<host>:<port>/<endpoint>?db=<mydb>`. Authentication is managed using a reverse proxy, meaning that if the database should be accessed using authentication, the endpoint and port should be setfor the reverse proxy.

More information in influxopmon/docs/README.md
