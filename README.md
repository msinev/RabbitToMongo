# RabbitToMongo
Simple tool to log data of RabbitMQ into MongoDB.
Written in c++ and based on boost librarries. Tested under Ubuntu 14.04.

It would bind a queue to exchange provided and then dump all JSON messages into one MongoDB collection.
Messages failed to be parsed as JSON would be dumped into another "error" collection.

