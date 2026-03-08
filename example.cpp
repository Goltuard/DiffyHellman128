//
// Created by Engineer on 08.03.2026.
//

#include <iostream>
#include "DiffyHellman128.h"

class Endpoint {
public:
    Endpoint(const u128 generator, const u128 key, const std::string &name) {
        this->_generator = generator;
        this->_privateKey = key;
        this->_publicKey = getPublicKey(_generator, _privateKey);
        this->_name = name;
    }

    u128 InitCommunication(u128 publicKey, Endpoint* sender) {
        std::cout << '\n';
        std::cout << _name << " is receiving a communication request. Sending public key.\n";
        std::cout << _name << "'s public key: " << std::hex << _publicKey[1] << _publicKey[0] << std::dec << '\n';

        _communicationKey = getCommunicationKey(this->_privateKey, publicKey);
        std::cout << _name << "'s computed communication key: " << std::hex << _communicationKey[1] << _communicationKey[0] << std::dec << '\n';
        _target = sender;

        return this->_publicKey;
    }

    void Communicate(Endpoint* target) {
        _target = target;
        if (_target) {
            std::cout << '\n';
            std::cout << _name << " is beginning communication.\n";
            std::cout << _name << "'s public key: " << std::hex << this->_publicKey[1] << this->_publicKey[0] << std::dec << '\n';

            u128 publicKey = _target->InitCommunication(this->_publicKey, this);

            this->_communicationKey = getCommunicationKey(this->_privateKey, publicKey);

            std::cout << _name << "'s computed communication key: " << std::hex << this->_communicationKey[1] << this->_communicationKey[0] << std::dec << '\n';
        }
    }

private:
    std::string _name;
    u128 _generator{};
    u128 _privateKey{};
    u128 _publicKey{};
    u128 _communicationKey{};
    Endpoint* _target{};
};

int main() {
    init128({0b10000111,0});
    const u128 gen{2,0};

    Endpoint a(gen,{23656,5426547},"Alpha");
    Endpoint b(gen,{1268976,212415},"Bravo");
    Endpoint* ptr_a = &a;

    // establish communication between Alpha and Bravo
    b.Communicate(ptr_a);

    return 0;
}