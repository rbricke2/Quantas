//
//  Network.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/24/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Network_hpp
#define Network_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <iostream>
#include <iomanip>
#include "Peer.hpp"

static const std::string POISSON = "POISSON";
static const std::string RANDOM = "RANDOM";

template<class type_msg, class peer_type>
class Network{
protected:
    
    std::vector<Peer<type_msg>*>        _peers;
    std::default_random_engine          _randomGenerator;
    int                                 _avgDelay;
    int                                 _maxDelay;
    int                                 _minDelay;
    std::string                         _distribution;
    
    std::string                         createId            ();
    bool                                idTaken             (std::string);
    std::string                         getUniqueId         ();
    void                                addEdges            (Peer<type_msg>*);
    int                                 getDelay            ();
    
public:
    Network                                                 ();
    Network                                                 (const Network<type_msg,peer_type>&);
    ~Network                                                ();
    
    // setters
    void                                initNetwork         (int, int); // initialize network with peers
    void                                setMaxDelay         (int d)                              {_maxDelay = d;};
    void                                setAvgDelay         (int d)                              {_avgDelay = d;};
    void                                setMinDelay         (int d)                              {_minDelay = d;};
    void                                setToRandom         ()                                   {_distribution = RANDOM;};
    void                                setToPoisson        ()                                   {_distribution = POISSON;};
    
    // getters
    int                                 size                ()const                              {return (int)_peers.size();};
    int                                 maxDelay            ()const                              {return _maxDelay;};
    int                                 avgDelay            ()const                              {return _avgDelay;};
    int                                 minDelay            ()const                              {return _minDelay;};
    std::string                         distribution        ()const                              {return _distribution;};
    //mutators
    void                                receive             ();
    void                                preformComputation  ();
    void                                transmit            ();
    void                                makeRequest         (int i)                              {_peers[i]->makeRequest();};
    
    // operators
    Network&                            operator=           (const Network&);
    peer_type*                          operator[]          (int);
    friend std::ostream&                operator<<          (std::ostream&, const Network&);
};

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::Network(){
    _peers = std::vector<Peer<type_msg>*>();
    _randomGenerator = std::default_random_engine();
    _avgDelay = 0;
    _maxDelay = std::numeric_limits<int>::max();;
    _minDelay = std::numeric_limits<int>::min();
    _distribution = RANDOM;
}

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::Network(const Network<type_msg,peer_type> &rhs){
    _peers = rhs._peers;
    _randomGenerator = std::default_random_engine();
    _avgDelay = rhs._avgDelay;
    _maxDelay = rhs._maxDelay;
    _minDelay = rhs._minDelay;
    _distribution = rhs._distribution;
}

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::~Network(){
    for(int i = 0; i < _peers.size(); i++){
        delete _peers[i];
    }
}

template<class type_msg, class peer_type>
std::string Network<type_msg,peer_type>::createId(){
    char firstPos = '*';
    char secondPos = '*';
    char thirdPos = '*';
    char fourthPos = '*';
    char fifthPos = '*';

    std::uniform_int_distribution<int> uniformDist(0,25);
    // add 'A' to shift char into rnage of upper case letters
    firstPos = uniformDist(_randomGenerator) + 'A';
    secondPos = uniformDist(_randomGenerator) + 'A';
    thirdPos = uniformDist(_randomGenerator) + 'A';
    fourthPos = uniformDist(_randomGenerator) + 'A';
    fifthPos = uniformDist(_randomGenerator) + 'A';
    
    std::string id = "";
    id = id + firstPos + secondPos + thirdPos + fourthPos + fifthPos;
    return id;
}

template<class type_msg, class peer_type>
bool Network<type_msg,peer_type>::idTaken(std::string id){
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() == id){
            return true;
        }
    }
    return false;
}

template<class type_msg, class peer_type>
std::string Network<type_msg,peer_type>::getUniqueId(){
    std::string id = createId();
    
    while(idTaken(id)){
        id = createId();
    }
    
    return id;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::addEdges(Peer<type_msg> *peer){
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() != peer->id()){
            if(!_peers[i]->isNeighbor(peer->id())){
                int delay = getDelay();
                // guard agenst 0 and negative numbers
                while(delay < 1 && delay > _maxDelay && delay < _minDelay){
                    delay = getDelay();
                }
                peer->addNeighbor(*_peers[i], delay);
                _peers[i]->addNeighbor(*peer,delay);
            }
        }
    }
}

template<class type_msg, class peer_type>
int Network<type_msg,peer_type>::getDelay(){
    if(_distribution == RANDOM){
        std::uniform_int_distribution<int> randomDistribution(_minDelay,_maxDelay);
        return randomDistribution(_randomGenerator);
    }
    if(_distribution == POISSON){
        std::poisson_distribution<int> poissonDistribution(_avgDelay);
        return poissonDistribution(_randomGenerator);
    }
    return -1;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::initNetwork(int numberOfPeers, int avgDelay){
    _avgDelay = avgDelay;
    
    for(int i = 0; i < numberOfPeers; i++){
        _peers.push_back(new peer_type(getUniqueId()));
    }
    for(int i = 0; i < _peers.size(); i++){
        addEdges(_peers[i]);
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::receive(){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->receive();
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::preformComputation(){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->preformComputation();
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::transmit(){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->transmit();
    }
}

template<class type_msg, class peer_type>
Network<type_msg, peer_type>& Network<type_msg,peer_type>::operator=(const Network<type_msg, peer_type> &rhs){
    _peers = rhs._peers;
    _randomGenerator = std::default_random_engine();
    _avgDelay = rhs._avgDelay;
    _maxDelay = rhs._maxDelay;
    _minDelay = rhs._minDelay;
    _distribution = rhs._distribution;
    
    return *this;
}

template<class type_msg, class peer_type>
peer_type* Network<type_msg,peer_type>::operator[](int i){
    return dynamic_cast<peer_type*>(_peers[i]);
}

template<class type_msg, class peer_type>
std::ostream& operator<<(std::ostream &out, const Network<type_msg,peer_type> &system){
    out<< "--- NETWROK SETUP ---"<< std::endl;
    out<< std::left;
    out<< std::setw(20)<< "Number of Peers"<< std::setw(20)<< "Distribution"<< std::setw(20)<< "Min Delay"<< std::setw(20)<< "Average Delay"<< std::setw(20)<< "Max Delay"<< std::endl;
    out<< std::setw(20)<< system.size()<< std::setw(20)<< system.distribution()<< std::setw(20)<< system.minDelay()<< std::setw(20)<< system.avgDelay()<< std::setw(20)<< system.maxDelay()<< std::endl;
    
//    for(int i = 0; i < system.size(); i++){
//        system[i]->print(out);
//    }
    
    return out;
}

#endif /* Network_hpp */
