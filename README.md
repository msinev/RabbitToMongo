# RabbitToMongo
Simple tool to log data of RabbitMQ exchange  into MongoDB.
Written in c++ and based on boost librarries and c++ RabbitMQ and MongoDB clients. Tested under Ubuntu 14.04.

It would bind a queue to exchange provided and then dump all JSON messages into one MongoDB collection.
Messages failed to be parsed as JSON would be dumped into another "error" collection.

Portions are under MIT license (see inside util.*)

Build system based on cmake

#Command line option
```
Allowed options:
  -h [ --help ]                        produce help message
  -H [ --qhost ] arg (=127.0.0.0)      Rabbit MQ host name
  -V [ --qvhost ] arg (=/)             Rabbit MQ vhost
  -U [ --quser ] arg (=guest)          Rabbit MQ user
  -P [ --qpass ] arg (=guest)          Rabbit MQ password
  -X [ --exch ] arg                    Rabbit MQ exchange
  --expassive arg (=0)                 Rabbit MQ exchange is passive
  --exdurable arg (=1)                 Rabbit MQ exchange is durable
  --port arg (=5672)                   Rabbit MQ port to connect
  -T [ --exchtype ] arg (=fanout)      Rabbit MQ exchange type
  -K [ --exchkey ] arg                 Rabbit MQ routing key
  -m [ --mhost ] arg (=127.0.0.1)      MongoDB host
  -C [ --mcollection ] arg             MongoDB collection
  -D [ --mdb ] arg                     MongoDB database
  -u [ --muser ] arg                   MongoDB  user
  -p [ --mpass ] arg                   MongoDB  password
  -E [ --merr ] arg (=errors)          MongoDB collection for malformed 
                                       messages
  -f [ --fork ] [=arg(=1)] (=0)        Fork new instance of daemon process
  -k [ --kill ] [=arg(=1)] (=0)        Kill running instance of daemon process
  -s [ --setuid ] [=arg(=-2)] (=0)     Set UID jail to nobody or to particular 
                                       UID
  -c [ --config ] [=arg(=/etc/reader)] Write config file

```
