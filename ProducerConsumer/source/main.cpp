// Copyright

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <vector>
#include <memory>
#include <random>
#include <functional>
#include <string>
#include <sstream>

template <typename T>
class ConcurrentQueue {
public:
    void push(const T& data);       // productor
    T pop();                        // consumidor
private:
    std::queue<T> queue_;
    std::mutex queue_mutex_;
    std::condition_variable c1_;    // permite notificar a las demas threads que cierta condicion ha sido valida 
};


template <typename T>
void ConcurrentQueue<T>::push(const T& data) {

    std::unique_lock<std::mutex>lock(queue_mutex_);
    queue_.push(data);
    //lock.unlock();    // no es necesario usar "unlock" debido a que el unique_lock en su destructor lo hace
    c1_.notify_one();   // notificar a uno 
}

template <typename T>
T ConcurrentQueue<T>::pop(){

    std::unique_lock<std::mutex>lock(queue_mutex_); // cuando un POP(consumidor) pase tiene que esperar
    while (queue_.empty()) {                        // si es que la cola esta vacia va a esperar la condicion
        c1_.wait(lock);                       // va esperar y va a notificar con ese mismo lock que esta esperando 
    }
    T result = queue_.front();
    queue_.pop();
    //lock.unlock();    // no es necesario usar "unlock" debido a que el unique_lock en su destructor lo hace
    return result;
}


class Producer {
public:
    Producer(unsigned int id, ConcurrentQueue<std::string>* queue)
        : id_(id), queue_(queue) {}

    void operator()() {
        int data = 0;
        while (true) {
            std::stringstream stream;
            stream << "Producer: " << id_ << " Data: " << data++ << std::endl;
            queue_->push(stream.str());
            std::cout << stream.str() << std::endl;
        }
    }

private:
    unsigned int id_;
    ConcurrentQueue<std::string>* queue_;
};

class Consumer {
public:
    Consumer(unsigned int id, ConcurrentQueue<std::string>* queue)
        : id_(id), queue_(queue) {}

    void operator()() {
        while (true) {
            std::stringstream stream;
            stream << "Consumer: " << id_ << " Data: " << queue_->pop().c_str()
                << std::endl;

            std::cout << stream.str() << std::endl;
        }
    }

private:
    unsigned int id_;
    ConcurrentQueue<std::string>* queue_;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        return 1;
    }
    int number_producers = std::stoi(argv[1]);
    int number_consumers = std::stoi(argv[2]);

    ConcurrentQueue<std::string> queue;

    std::vector<std::thread*> producers;
    for (unsigned int i = 0; i < number_producers; ++i) {
        std::thread* producer_thread = new std::thread(Producer(i, &queue));
        producers.push_back(producer_thread);
    }

    std::vector<std::thread*> consumers;
    for (unsigned int i = 0; i < number_consumers; ++i) {
        std::thread* consumer_thread = new std::thread(Consumer(i, &queue));
        consumers.push_back(consumer_thread);
    }

    int stop;
    std::cin >> stop;
    // join

    return 0;
}