/* 
 * File:   mongowriter.h
 * Author: max
 *
 * Created on December 30, 2014, 5:03 PM
 */

#ifndef MONGOWRITER_H
#define	MONGOWRITER_H

struct mongoConfig {
    std::string host, db, maincol, dumpcol, name, pass;
};

class mongowriter : public messageAcceptor, mongoConfig {
    std::string collectionID, dumpID;
public:
    bool ready;
    mongowriter(const mongoConfig&);

    virtual ~mongowriter();
private:
    mongo::DBClientConnection c;
    bool init();
    mongowriter();
    virtual void accept(char *message, int len);
};

#endif	/* MONGOWRITER_H */

