/* 
 * File:   rabbitreader.cpp
 * Author: max
 *
 * Created on December 14, 2014, 1:28 AM
 */
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <stdint.h>
#include <string>

#include <amqp_ssl_socket.h>
#include <amqp_framing.h>

#include "reader.hpp"


//#include <jsoncpp/json/json.h>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "mongowriter.h"


#include <cstdlib>
#include <iostream>
#include <fstream>
//#include <pthread.h>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
//#include <boost/atomic.hpp>

extern "C" {
#include <signal.h>
#include <libdaemon/dfork.h>
//#include <libdaemon/dsignal.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dexec.h>
}
#define FORKDEBUG 1
/*
struct message {
  message *next;
  int count;
  char data[1];  
};
struct thread_data{
   int  thread_id;
   int  sum;
   char *message;
};

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_threshold_cv=PTHREAD_COND_INITIALIZER;
pthread_t  th;
 */

/*
 * 

extern "C" void *WorkerReader(void *threadarg) {
    pthread_mutex_lock(&mymutex);
    std::cout << "---Hello " <<  std::endl;
    pthread_cond_signal(&count_threshold_cv);
    pthread_mutex_unlock(&mymutex);
}
 */

#ifdef FORKDEBUG
  extern "C" const char* pidFileStub(void) {
      return "/tmp/reader.pid";
  }
#endif


class printAcceptor : public messageAcceptor {
    virtual void accept(char *message, int len);
};

void printAcceptor::accept(char *message, int len) {
    std::cout << "Message:" << message << std::endl;
}

//struct theDaemonSync {

//    boost::mutex quitmutex;
//    boost::barrier inited;
//    theDaemonSync();
//};

//theDaemonSync::theDaemonSync() : inited(2) {}

/*
class unlocker {
    boost::barrier &barier;
    bool lock;
public:
    void complete();
    unlocker(boost::barrier &b);
    ~unlocker();
};
void unlocker::complete() {
    if(lock) {
        lock=false;
        barier.wait();
    }
}

unlocker::unlocker(boost::barrier &b) : barier(b) {

}

unlocker::~unlocker() {
    complete();
}
*/

boost::condition_variable condSignal;
boost::mutex              mutSignal;
volatile bool exitFlag=false;

extern "C" void signalHandler(int) {
  boost::unique_lock<boost::mutex> lock(mutSignal);
  exitFlag=true;
  condSignal.notify_all();
  }

void waitHandler() {
    boost::unique_lock<boost::mutex> lock(mutSignal);
    if(exitFlag) return;
    condSignal.wait(lock);
  }

void installHandler(int signal) {
    struct sigaction action;
    struct sigaction saveaction;

    memset(&action, 0, sizeof(action));
    action.sa_handler=signalHandler;
    int rcaction=sigaction(signal, &action, &saveaction);
}

struct theDaemonRunner {
   AMQPConnector &connector;
   volatile bool quit;
   theDaemonRunner(
            AMQPConnector &connector );
   void operator()();
   };

theDaemonRunner::theDaemonRunner(
        AMQPConnector &amq) : connector(amq), quit(false) {
  }

void theDaemonRunner::operator()() {
    { // start processing
           daemon_log(LOG_INFO, "Starting connector");

            try {
               connector.run();
            } catch (std::string str) {
                daemon_log(LOG_ERR, "Unknown shit in AMQP.");
            } catch (...) {
                daemon_log(LOG_ERR, "Unknown shit while init.");
            }
            daemon_log(LOG_INFO, "Exiting connector");

            connector.close();
    }
    quit=true;
  }

int consoleRun(AMPQParam &amq, mongoConfig &m) {
    {
        mongowriter writer(m);
        if (writer.ready) {
            AMQPConnector connector(&writer, amq);
            try {

                connector.init();
                connector.run();

            } catch (std::string str) {
                std::cerr << "Error in AMQP connector:" << str << std::endl;
                return 5;
            } catch (...) {
                std::cerr << "Unknown shit while run!" << std::endl;
                return 5;
            }
            //finally
            {
                connector.close();
            }
        }
    }
    return 0;
}

int daemonRun(AMPQParam &amq, mongoConfig &m) {
    installHandler(SIGINT);
    installHandler(SIGTERM);
    installHandler(SIGQUIT);
    installHandler(SIGHUP);


    {
    mongowriter writer(m);
    if (!writer.ready) {
        daemon_log(LOG_ERR, "Could not register signal handlers (%s).", strerror(errno));
        daemon_retval_send(3);
        goto finish;
    }
    {
    AMQPConnector connector(&writer, amq);

    try {
        connector.init();
#ifndef FORKDEBUG
                // Send OK to parent process
                daemon_retval_send(0);
#endif
    } catch (std::string str) {
        daemon_log(LOG_ERR, "Error in AMQP connector: (%s).", str.c_str());
        daemon_retval_send(3);
        goto finish;
    } catch (...) {
        daemon_retval_send(3);
        daemon_log(LOG_ERR, "Unknown shit while init.");
        goto finish;
    }
    {
   theDaemonRunner monitor(connector);
   boost::thread monitorTHread(monitor);

   waitHandler();
   connector.inter();

//
//
//

    monitorTHread.join();

    }}}
    finish: daemon_log(LOG_INFO, "Stopping monitor");
    return 0;
}


namespace po = boost::program_options;

void printHelp(po::options_description &desc, const char *argv0) {
    std::cout << "Exec: " << argv0
            //<< "\n" << descConf
            << "\n" << desc << std::endl;
}




int main(int argc, char **argv) {
#ifdef FORKDEBUG
    daemon_pid_file_proc=pidFileStub;
#endif

    AMPQParam amq;

    //printAcceptor acceptor;
    mongoConfig m;

    // std::string  rexch, rexchtype, rexchkey;

    std::string config;
    bool daemon = false;
    bool fKill = false;
    int esetuid = -2;


    po::options_description descConf("Config options");
    descConf.add_options()
            ("config,c", po::value<std::string>(&config)->default_value("")->implicit_value("/etc/reader"), "Config file")
            ("kill,k", po::value<bool>(&fKill)->default_value(false)->implicit_value(true), "Kill instance of daemon process stop if no fork")
            ("fork,f", po::value<bool>(&daemon)->default_value(false)->implicit_value(true), "Fork new instance of daemon process");

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("qhost,H", po::value<std::string>(&amq.hostname)->default_value("10.11.26.145"), "Rabbit MQ host name")
            ("qvhost,V", po::value<std::string>(&amq.vhost)->default_value("/"), "Rabbit MQ vhost")
            ("quser,U", po::value<std::string>(&amq.vhostuser)->default_value("guest"), "Rabbit MQ user")
            ("qpass,P", po::value<std::string>(&amq.vhostpass)->default_value("guest"), "Rabbit MQ password")
            ("exch,X", po::value<std::string>(&amq.exchange)->required(), "Rabbit MQ exchange")
            ("expassive", po::value<bool>(&amq.passive)->default_value(false), "Rabbit MQ exchange is passive")
            ("exdurable", po::value<bool>(&amq.durable)->default_value(true), "Rabbit MQ exchange is durable")
            ("port", po::value<int>(&amq.port)->default_value(5672), "Rabbit MQ port to connect")
            ("exchtype,T", po::value<std::string>(&amq.extype)->default_value("fanout"), "Rabbit MQ exchange type")
            ("exchkey,K", po::value<std::string>(&amq.bindingkey)->default_value(""), "Rabbit MQ routing key")
            ("mhost,m", po::value<std::string>(&m.host)->default_value("127.0.0.1"), "MongoDB host")
            ("mcollection,C", po::value<std::string>(&m.maincol)->required(), "MongoDB collection")
            ("mdb,D", po::value<std::string>(&m.db)->required(), "MongoDB database")
            ("muser,u", po::value<std::string>(&m.name)->default_value(""), "MongoDB  user")
            ("mpass,p", po::value<std::string>(&m.pass)->default_value(""), "MongoDB  password")
            ("merr,E", po::value<std::string>(&m.dumpcol)->default_value("errors"), "MongoDB collection for malformed messages")
            ("fork,f", po::value<bool>(&daemon)->default_value(false)->implicit_value(true), "Fork new instance of daemon process")
            ("kill,k", po::value<bool>(&fKill)->default_value(false)->implicit_value(true), "Kill running instance of daemon process")
            ("setuid,s", po::value<int>(&esetuid)->default_value(0)->implicit_value(-2), "Set UID jail to nobody or to particular UID")
            ("config,c", po::value<std::string>(&config)->default_value("")->implicit_value("/etc/reader"), "Write config file");

    const po::positional_options_description p;

    if (argc < 2) {
        printHelp(desc, argv[0]);
        return 1;
    }

    bool invalidConfigCommanline = true;
    try {

        po::variables_map vm;

        po::variables_map vmc;
        try {
            po::parsed_options parsedConfig = po::command_line_parser(argc, argv).options(descConf).positional(p).run();
            po::store(parsedConfig, vmc);
            po::notify(vmc);
            if (argc >= 0) // sort of useless as allways true bu here to make "too smart" clion happy
                invalidConfigCommanline = false;


        } catch (std::exception &e) {
            // invalidConfigCommanline=true;
        }

//        std::vector<std::string> v = po::collect_unrecognized(parsedConfig.options, po::include_positional);
//

        if (invalidConfigCommanline) {

            po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).positional(p).run();
            po::store(parsed, vm);

            if (vm.count("help")) {
                printHelp(desc, argv[0]);
                return 1;
            }

            po::notify(vm);

            if (!config.empty()) {

                std::ofstream fconf(config.c_str());

                bool ferr = (fconf.bad() || !fconf.good());
                if (ferr) {
                    std::cout << "Error writing to " << config << std::endl;
                    std::cout << "----- Please copy data below manualy -----" << std::endl;
                } else {
                    std::cout << "Writing configuration to " << config << std::endl;
                }

                for (std::vector<po::option>::const_iterator i = parsed.options.begin(); i != parsed.options.end(); ++i) {
                    const po::option &o = *i;
                    //if (vm.find(o.string_key) != vm.end()) {
                    // an unknown option
                    for (std::vector<std::string>::const_iterator ivalue = o.value.begin(); ivalue != o.value.end(); ++ivalue) {
                        if (o.string_key != "config") {
                            std::cout << o.string_key << "=" << *ivalue << std::endl; // o.string_key << "=" << o.value;
                            if (!ferr) fconf << o.string_key << "=" << *ivalue << "\n";
                        }
                    }
                    // }
                }

                fconf.close();
                return 0;
            }

        } else {
            bool saveDaemon = daemon;
            bool saveKill = fKill;
            std::ifstream file;
            file.open(config.c_str());
            po::store(po::parse_config_file(file, desc, true), vm);
            po::notify(vm);

            daemon |= saveDaemon;
            fKill  |= saveKill;
        }



    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout
                //<< descConf << "\n"
                << desc << std::endl;
        return 3;
    } catch (...) {
        std::cerr << "Unknown error!" << "\n";
        return 3;
    }

    if (!daemon && !fKill)
        return consoleRun(amq, m);

    pid_t pid = 0;

    /* Reset signal handlers */
    if (daemon_reset_sigs(-1) < 0) {
        daemon_log(LOG_ERR, "Failed to reset all signal handlers: %s", strerror(errno));
        return 1;
    }

    /* Unblock signals */
    if (daemon_unblock_sigs(-1) < 0) {
        daemon_log(LOG_ERR, "Failed to unblock all signals: %s", strerror(errno));
        return 1;
    }

    daemon_pid_file_ident = daemon_log_ident = daemon_ident_from_argv0(argv[0]);

    if (fKill) {
        int ret;
        // Check if the new function daemon_pid_file_kill_wait() is available, if it is, use it.
        if ((ret = daemon_pid_file_kill_wait(SIGTERM, 5)) < 0) {
            daemon_log(LOG_WARNING, "Failed to kill daemon: %s", strerror(errno));
        }
        if (!daemon) return (ret < 0) ? 1 : 0;
    }


    if ((pid = daemon_pid_file_is_running()) >= 0) {
        daemon_log(LOG_ERR, "Daemon already running on PID file %u", pid);
        return 1;
    }
#ifndef FORKDEBUG

    if (daemon_retval_init() < 0) {
        daemon_log(LOG_ERR, "Failed to create pipe.");
        return 1;
    }

    pid = daemon_fork();
#else
    pid=0;
#endif

    if (pid < 0) { // can't fork
        daemon_retval_done();
        return 1;

    } else if (pid) { // parent
        int ret;

        // Wait for 20 seconds for the return value passed from the daemon process 
        if ((ret = daemon_retval_wait(20)) < 0) {
            daemon_log(LOG_ERR, "Could not recieve return value from daemon process: %s", strerror(errno));
            return 255;
        }

        daemon_log(ret != 0 ? LOG_ERR : LOG_INFO, "Daemon returned %i as return value.", ret);
        return ret;

    } else {
        // The daemon 

        // Close FDs 
        if (daemon_close_all(-1) < 0) {
            daemon_log(LOG_ERR, "Failed to close all file descriptors: %s", strerror(errno));

            // Send the error condition to the parent process 
            daemon_retval_send(1);
            goto finish;
        }

        // Create the PID file
        if (daemon_pid_file_create() < 0) {
            daemon_log(LOG_ERR, "Could not create PID file (%s).", strerror(errno));
            daemon_retval_send(2);
            goto finish;
        }
        // Initialize signal handling
        if (esetuid && setuid(esetuid)) {
            daemon_log(LOG_ERR, "Could not suid (%s).", strerror(errno));
            daemon_retval_send(4);
            goto finish;
        }

        daemonRun(amq, m);

        // Do a cleanup 

        finish:
        daemon_log(LOG_INFO, "Exiting...");
        daemon_retval_send(255);
        daemon_pid_file_remove();

        return 0;
    }

    //Wait-free ring buffer
    /* 
    pthread_mutex_lock(&mymutex);


    boost::atomic<message *> exchangePoint;

    std::cout << "--- ==== ---" <<  std::endl;

     int rc = pthread_create(&th, NULL, WorkerReader, NULL);
          if (rc){
              std::cout << "ERROR; return code from pthread_create() is " <<  rc << std::endl;

             exit(-1);
          }

        pthread_cond_wait(&count_threshold_cv, &mymutex);

        pthread_mutex_unlock(&mymutex);
    
        std::cout << "--- World " <<  std::endl;
    
        return 0;
     * 
     * */
}

