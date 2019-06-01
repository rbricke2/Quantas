//
//  main.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <set>
#include "Blockchain.hpp"
#include "Peer.hpp"
#include "ExamplePeer.hpp"
#include "PBFT_Peer.hpp"
#include "syncBFT_Peer.hpp"
#include "Network.hpp"
#include "bCoin_Peer.hpp"
#include "PBFTPeer_Sharded.hpp"
#include "PBFTReferenceCommittee.hpp"
#include <iostream>
#include <chrono>
#include <random>


const int peerCount = 10;
const int blockChainLength = 100;
Blockchain *blockchain;
int shuffleByzantineInterval = 0;
int syncBFT_Peer::peerCount = 3;
const static std::string s_pbft_header = "Total Request:,Max Ledger:,Ratio Defeated To Honest 1,Ratio Defeated To Honest 2,Ratio Defeated To Honest 3,Ratio Defeated To Honest 4,Ratio Defeated To Honest 5,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5, total honest 1, total honest 2, total honest 3, total honest 4, total honest 5, total defeated 1, total defeated 2, total defeated 3, total defeated 4, total defeated 5\n";

// util functions
void buildInitialChain(std::vector<std::string>);
std::set<std::string> getPeersForConsensus(int);
int getNumberOfConfimedTransactionsBGS_PBFT(const std::vector<PBFTPeer_Sharded>&);
int sumMessagesSentBGS(const PBFTReferenceCommittee&);
void calculateResults(const PBFTReferenceCommittee system,std::ofstream &csv);
double ratioOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
double waitTimeOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
int totalNumberOfDefeatedCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
int totalNumberOfCorrectCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);

void Example(std::ofstream &logFile);
void PBFT(std::ofstream &out,int);
void syncBFT(std::ofstream &,int );
void bitcoin(std::ofstream &,int );
void Sharded_PBFT(std::ofstream &csv, std::ofstream &log,int,double);

int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    if(argc < 3){
        std::cerr << "Error: need algorithm and output path" << std::endl;
        return 0;
    }


    std::string algorithm = argv[1];
    std::string filePath = argv[2];

    if(algorithm == "example"){
        std::ofstream out;
        std::string file = filePath + "/example.log";
        out.open(file);
        if ( out.fail() ){
            std::cerr << "Error: could not open file"<< file  << std::endl;
        }
        Example(out);
    }
    else if (algorithm== "pbft"){
        for(int delay = 1; delay < 51; delay = delay + 10){
            std::cout<< "PBFT"<<std::endl;
            std::ofstream out;
            out.open(filePath + "/PBFT_Delay"+std::to_string(delay) + ".log");
            if ( out.fail() ){
                std::cerr << "Error: could not open file" << std::endl;
            }
            for(int run = 0; run < 10; run++){
                PBFT(out,delay);
            }
            out.close();
            if(delay == 1){
                delay = 0;
            }
        }

    }else if (algorithm == "syncBFT") {
        std::ofstream out;
        for(int delay = 1;delay<2; delay+=10){
            std::cout<< "Start with Delay "+std::to_string(delay)<< std::endl;
            for(int run = 0; run < 2; run++){
                syncBFT(out, delay);
            }

        }
    }else if (algorithm == "pbft_s") {
        std::cout<< "pbft_s"<<std::endl;
        std::ofstream csv;
        std::ofstream log;
        std::string file = filePath + "pbft_s";
        csv.open(file + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< file + ".csv" << std::endl;
        }
        log.open(file + ".log");
        if ( log.fail() ){
            std::cerr << "Error: could not open file: "<< file + ".csv" << std::endl;
        }
            //ceil(256*0.3)
        for(int delay = 1; delay < 10; delay++){
            csv<< "Delay,"<< std::to_string(delay)<< std::endl;
            csv<< s_pbft_header;
            double fault = 0.1;
            while(ceil(256*fault) != 256){
                csv<< "fault,"<< std::to_string(fault)<< std::endl;
                for(int run = 0; run < 2; run++){
                    Sharded_PBFT(csv,log,delay,fault);
                }
                fault += 0.1;
            }
        }
//        for(int delay = 10; delay < 50; delay = delay + 10){
//             csv<< "Delay,"<< std::to_string(delay);
//             csv<< s_pbft_header;
//             double fault = 0.0;
//             while(ceil(256*fault) != 256){
//                 csv<< "fault,"<< std::to_string(fault)<< std::endl;
//                 for(int run = 0; run < 2; run++){
//                     Sharded_PBFT(csv,log,delay,fault);
//                 }
//                 fault += 0.1;
//             }
//         }
        log.close();
        csv.close();
    }else if (algorithm == "bitcoin") {
        std::ofstream out;
        bitcoin(out, 1);
    }
    
    return 0;
}

void Example(std::ofstream &logFile){
    ByzantineNetwork<ExampleMessage,ExamplePeer> system;
    system.setLog(logFile); // set the system to write log to file logFile
    system.setToRandom(); // set system to use a uniform random distribution of weights on edges (channel delays) 
    system.setMaxDelay(3); // set the max weight an edge can have to 3 (system will now pick randomly between [1, 3])
    system.initNetwork(5); // Initialize the system (create it) with 5 peers given the above settings

    for(int i =0; i < 3; i++){
       logFile<< "-- STARTING ROUND "<< i<< " --"<<  std::endl; // write in the log when the round started

        system.receive(); // do the receive phase of the round 
        //system.log(); // log the system state
        system.preformComputation();  // do the preform computation phase of the round 
        //system.log();
        system.transmit(); // do the transmit phase of the round 
        //system.log();

        logFile<< "-- ENDING ROUND "<< i<< " --"<<  std::endl; // log the end of a round
    }

    system = ByzantineNetwork<ExampleMessage,ExamplePeer>(); // clear old setup by creating a fresh object
    system.setLog(std::cout); // set the system to write log to terminal 
    system.setToRandom(); 
    system.setMaxDelay(10);
    system.initNetwork(3); 

    for(int i =0; i < 3; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl; // print outwhen the round started

        system.receive(); 
        system.log(); // log now goes to the terminal
        system.preformComputation(); 
        system.log();
        system.transmit();
        system.log();

        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl; // print out the end of a round
    }

    // The base peer class tracks number of messages sent by a(1) peer. To calculate the total number of messages that where in the system we need 
    //      to add up each peers indivudal message count (example of looping thouth the Network class)

    int numberOfMessages = 0;
    for(int i = 0; i < system.size(); i++){
        numberOfMessages += system[i]->getMessageCount(); // notice that the index operator ([]) return a pointer to a peer. NEVER DELETE THIS PEER INSTANCE. 
                                                          //    The netwrok class deconstructor will get ride off ALL peer instances once it is deconstructed. 
                                                          //    Use the -> to call method on the peer instance. The Network class will also cast the instance to
                                                          //    your derived class so all methods that you add will be avalable via the -> operator
    }
    std::cout<< "Number of Messages: "<< numberOfMessages<< std::endl;
}

void PBFT(std::ofstream &out,int delay){
    ByzantineNetwork<PBFT_Message, PBFT_Peer> system;
    system.setLog(out);
    system.setToRandom();
    system.setMaxDelay(delay);
    system.initNetwork(1024);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
        system[i]->init();
    }
    
    int numberOfRequests = 0;
    for(int i =-1; i < 1000; i++){
        int randIndex = rand()%system.size();
        while(system[randIndex]->isPrimary()){
            randIndex = rand()%system.size();
        }
        system.makeRequest(randIndex);
        numberOfRequests++;
        
        system.receive();
        system.preformComputation();
        system.transmit();
        system.log();
        std::cout<< '.'<< std::flush;
    }

    int min = (int)system[0]->getLedger().size();
    int max = (int)system[0]->getLedger().size();
    int totalMessages = system[0]->getMessageCount();
    for(int i = 0; i < system.size(); i++){
        //out<< "Peer ID:"<< system[i].id() << " Ledger Size:"<< system[i].getLedger().size()<< std::endl;
        if(system[i]->getLedger().size() < min){
            min = (int)system[i]->getLedger().size();
        }
        if(system[i]->getLedger().size() > max){
            max = (int)system[i]->getLedger().size();
        }
        totalMessages += system[i]->getMessageCount();
    }
    out<< "Min Ledger:,"<< min<< std::endl;
    out<< "Max Ledger:,"<< max<< std::endl;
    out<< "Total Messages:,"<< totalMessages<< std::endl;
    out<< "Total Request:,"<< numberOfRequests<<std::endl;
    //std::cout<< std::endl;
}

void syncBFT(std::ofstream &out,int maxDelay){
//    srand(1);
    srand(time(nullptr));

    syncBFT_Peer::changeLeader = true;
    syncBFT_Peer::leaderIdCandidates = {};
    syncBFT_Peer::leaderId = "";
    syncBFT_Peer::syncBFTsystemState = 0;

    ByzantineNetwork<syncBFTmessage, syncBFT_Peer> n;
    ByzantineNetwork<PBFT_Message, PBFT_Peer> system;

    n.setMaxDelay(maxDelay);
    n.setToRandom();
    n.initNetwork(syncBFT_Peer::peerCount);

    int consensusSize = 0;

    vector<string> byzantinePeers;
    while(byzantinePeers.size()<(syncBFT_Peer::peerCount-1)/2) {
        int index = rand()%syncBFT_Peer::peerCount;
        if (std::find(byzantinePeers.begin(),byzantinePeers.end(),n[index]->id())!= byzantinePeers.end()){
        }else{
            n[index]->setByzantineFlag(true);
            byzantinePeers.push_back(n[index]->id());
        }
    }

    std::cerr<<"Byzantine Peers are ";
    for(auto &peer: byzantinePeers){
        std::cerr<<peer<<" ";
    }

    int lastConsensusAt = 0;

    //start with the first peer as leader

    bool shuffleByzantinePeers = false;
    int shuffleByzantineInterval = 20;
    for(int i =1; i<100; i++){

        if(i%shuffleByzantineInterval == 0){
            std::cerr<<"Iteration "<<i<<std::endl;
            shuffleByzantinePeers = true;
        }

        n.receive();
        n.preformComputation();
        n.transmit();

        if(syncBFT_Peer::syncBFTsystemState == 3){
            if(shuffleByzantinePeers){
                n.shuffleByzantines ((syncBFT_Peer::peerCount-1)/2);
                shuffleByzantinePeers = false;
            }
            //check if all peers terminated
            bool consensus = true;
            for(int peerId = 0; peerId<n.size(); peerId++ ) {
                if (!n[peerId]->getTerminationFlag()) {
                    //the leader does not need to terminate
                    if (n[peerId]->id() == syncBFT_Peer::getLeaderId()) {
                        continue;
                    }
                    consensus = false;
                    break;
                }
            }
            if (consensus){
                consensusSize++;
                std::cerr<<"++++++++++++++++++++++++++++++++++++++++++Consensus reached at iteration "<<i<<std::endl;
                //refresh the peers
                for(int i = 0;i<n.size();i++){
                    n[i]->refreshSyncBFT();

                }
                //reset system-wide sync state
                syncBFT_Peer::syncBFTsystemState = 0;

                lastConsensusAt = i;
                continue;
            }
        }

        if((i-lastConsensusAt)%n.maxDelay() ==0){
            //reset sync status of the peers after a notify step is done.
            if(syncBFT_Peer::syncBFTsystemState==3){
                for(int i = 0; i<n.size();i++){
                    n[i]->setSyncBFTState(0);
                    n[i]->iter++;
                }
            }
            syncBFT_Peer::incrementSyncBFTsystemState();
        }
    }

    std::cerr<<"Shuffled byzantine peers every "<<shuffleByzantineInterval<<"\tMax delay: "<<maxDelay<<"\t Consensus count: "<<n[0]->getBlockchain()->getChainSize()-1<<std::endl;

}

void bitcoin(std::ofstream &out, int avgDelay){
    ByzantineNetwork<bCoinMessage, bCoin_Peer> n;
    n.setToPoisson();
    n.setAvgDelay(avgDelay);
    n.setLog(std::cout);
    n.initNetwork(10);

    //mining delays at the beginning
    for(int i = 0; i<n.size(); i++){
        n[i]->setMineNextAt( bCoin_Peer::distribution(bCoin_Peer::generator) );
    }

    for(int i = 1; i<100; i++){
        std::cerr<<"Iteration "<<i<<std::endl;
        n.receive();
        n.preformComputation();
        n.transmit();
    }
    int maxChain = 0;
    for(int i =0;i<n.size();i++){
        if(n[i]->getBlockchain()->getChainSize()>maxChain)
            maxChain = i;
    }
    std::cerr<<"Number of confirmations = "<<n[maxChain]->getBlockchain()->getChainSize()<<std::endl;
}


void Sharded_PBFT(std::ofstream &csv, std::ofstream &log,int delay, double fault){
    std::cout<< std::endl<< "########################### PBFT_Sharded ###########################"<< std::endl;
    csv<< ""<< std::endl;
    PBFTReferenceCommittee system = PBFTReferenceCommittee();
    system.setGroupSize(8);
    system.setToRandom();
    system.setMaxDelay(delay);
    system.setLog(log);
    system.initNetwork(256);
    system.setFaultTolerance(0.3);
    system.makeByzantines(256*fault);

    int numberOfRequests = 0;
    for(int i =0; i < 1000; i++){
        system.makeRequest();numberOfRequests++;
        system.makeRequest();numberOfRequests++;
        system.makeRequest();numberOfRequests++;

         if(i%5 == 0){
             system.shuffleByzantines(256*fault);
         }

        system.receive();
        std::cout<< 'r'<< std::flush;
        system.preformComputation();
        std::cout<< 'p'<< std::flush;  
        system.transmit();
        std::cout<< 't'<< std::flush;
        //system.log();

        if(i%100 == 0){
            calculateResults(system,csv);
            system.log();
        }
    }
    calculateResults(system,csv);
}

//
// util functions
//

void buildInitialChain(std::vector<std::string> peerIds) {
    std::cerr << "Building initial chain" << std::endl;
    srand((time(nullptr)));
    Blockchain *preBuiltChain = new Blockchain(true);
    int index = 1;                  //blockchain index starts from 1;
    int blockCount = 1;
    std::string prevHash = "genesisHash";
    
    while (blockCount < blockChainLength) {
        std::set<std::string> publishers;
        
        std::string peerId = peerIds[rand()%peerIds.size()];
        std::string blockHash = std::to_string(index) + "_" + peerId;
        publishers.insert(peerId);
        preBuiltChain->createBlock(index, prevHash, blockHash, publishers);
        prevHash = blockHash;
        index++;
        blockCount++;
    }
    
    std::cerr << "Initial chain build complete." << std::endl;
    std::cerr << "Prebuilt chain: " << std::endl;
    std::cerr << *preBuiltChain << std::endl;
    std::cerr << preBuiltChain->getChainSize() << std::endl;
    blockchain = preBuiltChain;
    
    
}

std::set<std::string> getPeersForConsensus(int securityLevel) {
    int numOfPeers = peerCount / securityLevel;
    std::set<std::string> peersForConsensus;
    int chainSize = blockchain->getChainSize ();
    int i = blockchain->getChainSize() - 1;             //i keeps track of index in the chain
    while (peersForConsensus.size() < numOfPeers) {
        std::string peerId = *(blockchain->getBlockAt(i).getPublishers()).begin();
        peersForConsensus.insert(peerId);
        //random value
        int randVal = rand()%peerCount;
        
        int skip = randVal;
        if ((i - skip) <= 0) {
            i = chainSize - i - skip;
            
        } else
            i = i - skip;
    }
    
    return peersForConsensus;
}

int sumMessagesSentBGS(const PBFTReferenceCommittee &system){
    int sum = 0;
    for(int i = 0; i < system.size(); i++){
        sum += system[i]->getMessageCount();
    }
    return sum;
}

double ratioOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    double honest = 0;
    double defeated = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            if(entry->first.defeated){
                defeated++;
            }else{
                honest++;
            }
        }
    }
    if(honest == 0 && defeated == 0){
        return 0;
    }else if(honest == 0){
        return 1;
    }else{
        return defeated/honest;
    }
}

double waitTimeOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    double sumOfWaitingTime = 0;
    double totalNumberOfTrnasactions = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            sumOfWaitingTime += entry->first.commit_round - entry->first.submission_round;
            totalNumberOfTrnasactions++;
        }
    }
    if(totalNumberOfTrnasactions == 0){
        return 0;
    }else{
        return sumOfWaitingTime/totalNumberOfTrnasactions;
    }
}

int totalNumberOfDefeatedCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    int total = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            if(entry->first.defeated){
                total++;
            }
        }
    }
    return total;
}

int totalNumberOfCorrectCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    int total = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            if(!entry->first.defeated){
                total++;
            }
        }
    }
    return total;
}

//"Total Request:,Max Ledger:,Ratio Defeated To Honest 1,Ratio Defeated To Honest 2,Ratio Defeated To Honest 3,Ratio Defeated To Honest 4,Ratio Defeated To Honest 5,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5, total honest 1, total honest 2, total honest 3, total honest 4, total honest 5, total defeated 1, total defeated 2, total defeated 3, total defeated 4, total defeated 5\n"
void calculateResults(const PBFTReferenceCommittee system, std::ofstream &csv){
    std::vector<std::pair<PBFT_Message,int> > globalLedger = system.getGlobalLedger();
    csv<<std::to_string(system.totalSubmissions());
    csv<< ","<< std::to_string(globalLedger.size());
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel5()*system.getGroupSize()));
    
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel5()*system.getGroupSize()));
    
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel5()*system.getGroupSize()));
    
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel5()*system.getGroupSize()));
    csv<< std::endl;
    
}
