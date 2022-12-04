#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <sstream>

using namespace std;

template<class T>
struct Node {
    T value;
    Node* next;
    Node(T value_ = T(), Node* n = nullptr) {
        value = value_;
        next = n;
    }

};

template<class T>
struct ConcurrentQueue {
    Node<T>* head;
    Node<T>* tail;
    mutex head_lock;
    mutex tail_lock;

    ConcurrentQueue() {
        Node<T>* node = new Node<T>;    // Se crea un nodo ficticio con valor "por defecto" del tipo de dato
        node->next = nullptr;
        head = node;                    
        tail = node;
    }

    void push(T value) {
        Node<T>* node = new Node<T>(value);
        node->next = nullptr;
        tail_lock.lock();
        tail->next = node;
        tail = node;
        tail_lock.unlock();
    }

    T pop() {
        head_lock.lock();
        T tmp;
        Node<T>* node = head;
        Node<T>* new_head = node->next;
        if (new_head == nullptr) {
            head_lock.unlock();       
            return T(); // Si no hay elementos en la cola retorna el valor "por defecto" del tipo de dato que se este usando
        }
        else {          // Si es que new_head no es null entonces se guarda en el tmp el valor a eliminar y luego se lo retorna en la linea 62
            tmp = new_head->value;
            new_head->value = node->value;
            head = new_head;
            
        }
        head_lock.unlock();
        delete node;
        return tmp;
    }
};


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
