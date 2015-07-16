/* 
 * File:   mongowriter.cpp
 * Author: max
 * 
 * Created on December 30, 2014, 5:03 PM
 */
#include <string>

#include <amqp_ssl_socket.h>
#include <amqp_framing.h>

#include "reader.hpp"


//#include <jsoncpp/json/json.h>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "mongowriter.h"

#include <ctime>

mongowriter::mongowriter() : ready(false) {

    // do nothing
}

bool mongowriter::init() {
    std::string errmsg;
    collectionID = (maincol.find_first_of('.') != std::string::npos) ? maincol : (db + "." + maincol);
    dumpID = (dumpcol.find_first_of('.') != std::string::npos) ? dumpcol : (db + "." + dumpcol);

    if (!c.connect(host, errmsg)) {

        std::cerr << errmsg << std::endl;
        return false;
    } else {
        if (!name.empty() && !c.auth(db, name, pass, errmsg, true)) {
            std::cerr << errmsg << std::endl;
            return false;
        } else {
            std::cout << "connected to mongo(" << host << ") -> " << db << std::endl;
            return true;
        }
    }

}

void mongowriter::accept(char *message, int len) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];

    int lenParced = 0;
    tstruct = *localtime(&now);
    strftime(buf, sizeof (buf), "%Y%m%d%H%M%S", &tstruct);
    std::string errJSON;
    try {
        mongo::BSONObj obj;
        obj = mongo::fromjson(message, &lenParced);
        if (!obj.hasField("ts") && !obj.hasElement("ts")) {
            mongo::BSONObjBuilder b;
            b.appendElements(obj);
            b.append("ts", buf);
            obj = b.obj();
        }
        c.insert(collectionID, obj);

        return;
    }    catch (mongo::MsgAssertionException mx) {
        errJSON = mx.toString();
    }


    mongo::BSONObjBuilder b;
    b << "message" << message << "ts" << buf << "err" << errJSON;
    mongo::BSONObj p = b.obj();
    //     std::cout<< dumpID << std::endl;
    c.insert(dumpID, p);
}

mongowriter::mongowriter(const mongoConfig& orig) : mongoConfig(orig), ready(false) {
    ready = init();

}

mongowriter::~mongowriter() {

}

