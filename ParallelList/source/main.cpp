#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>
#include <chrono>
#include <atomic>

using namespace std;


atomic<int> contador_insertar = 0;
atomic<int> contador_buscar = 0;
atomic<int>contador_eliminar = 0;


template <class T>
struct CNode {
    T value;
    CNode<T>* next;
    CNode(T _v) { value = _v; next = 0; }

    mutex node_lock;
    bool marked = false;
    void lock_() {
        node_lock.lock();
    }
    void unlock_() {
        node_lock.unlock();
    }
};

template <class T>
class CList {

public:
    CList();
    ~CList();
    bool find(T x, CNode<T>**& p, CNode<T>*& ant);
    bool insert(T x);
    bool remove(T x);
    void print();

    bool contains(T x);
    bool validate(CNode<T>* pred, CNode<T>* curr);
private:
    CNode<T>* head;
    CNode<T>* tail;
    T min = numeric_limits<T>::min();
    T max = numeric_limits<T>::max();

};

template <class T>
CList<T>::CList() {     //constructor
    tail = new CNode<T>(max);
    head = new CNode<T>(min);
    head->next = tail;
}

template <class T>
CList<T>::~CList() {    // destrructor
    cout << "Entro al destructor" << endl;
    CNode<T>* a = head, * b;
    while (a) {
        //cout << "eliminando: " << a->value << endl;
        b = a->next;
        delete a;
        a = b;
    }
}


template <class T>
bool CList<T>::insert(T x) {    //insertar elementos en paralelo 
    
    bool flag = false;
    while (true) {
        CNode<T>* pred = head;
        CNode<T>* curr = head->next;
        while (curr->value < x) {
            pred = curr;
            curr = curr->next;
        }
        pred->lock_();  // bloquear predecesor
        curr->lock_();  // bloquear nodo actual

        if (validate(pred, curr)) { // validar si es que se puede realizar la inserción
            if (curr->value == x) { // si el valor actual es igual al valor a insertar, no se inserta porque no acepta repetidos.
                flag = false;
            }
            else {  // caso contrario se procede con la inserción del nodo y se actualiza los punteros
                CNode<T>* new_node = new CNode<T>(x);
                new_node->next = curr;
                pred->next = new_node;
                flag = true;
            }
            pred->unlock_();    // se desbloquea el predecesor
            curr->unlock_();    // se desbloquea el nodo actual
            contador_insertar++;// contar la cantidad de veces que se realiza la insercion
            return flag;        // retorna Verdadero si es que se insertó el elemento, caso contrario Falso
        }
        // si no entra al IF entonces se desbloquea tanto el predecesor como el nodo actual y se 
        // vuelve a intentar la inserción
        pred->unlock_();
        curr->unlock_();
    }
}

template <class T>
bool CList<T>::remove(T x) {    // eliminar elementos en paralelo
    bool flag = false;
    while (true) {
        CNode<T>* pred = head;
        CNode<T>* curr = head->next;
        while (curr->value < x) {
            pred = curr;
            curr = curr->next;
        }
        pred->lock_();  // bloquear predecesor
        curr->lock_();  // bloquear nodo actual
        if (validate(pred, curr)) { // validar si es que se puede realizar la eliminación   
            if (curr->value != x) { // si el valor actual es diferente al valor a eliminar, entonces no se puede eliminar ese elemento porque no existe en la lista enlazada
                curr->unlock_();    // se desbloquea el nodo actual
                flag = false;
            }
            else {  // caso contrario, el nodo a eliminar se "marca" y se actualiza los punteros
                curr->marked = true;
                pred->next = curr->next;
                curr->unlock_();    // se desbloquea el nodo actual para luego eliminarlo
                delete curr;
                flag = true;
            }
            pred->unlock_();    // se desbloquea el predecesor
            contador_eliminar++;// contar la cantidad de veces que se realiza la eliminacion
            return flag;        // retorna Verdadero si es que se eliminó el elemento, caso contrario Falso
        }
        // si no entra al IF entonces se desbloquea tanto el predecesor como el nodo actual y se 
        // vuelve a intentar la eliminación
        pred->unlock_();
        curr->unlock_();
    }
}

template <class T>
bool CList<T>::validate(CNode<T>* pred, CNode<T>* curr) {   // función para comprobar si se puede realizar la inserción o eliminación

    return !pred->marked && !curr->marked && pred->next == curr;
}

template <class T>
bool CList<T>::contains(T x) {  // función para buscar elementos en paralelo


    CNode<T>* curr = head;
    while (curr->value < x) {

        curr = curr->next;
    }
    contador_buscar++;  // contar la cantidad de veces que se realiza la busqueda
    return curr->value == x && !curr->marked;
}

template <class T>
void CList<T>::print() {    // imprimir los elementos de la lista enlazada
    cout << "Head->";
    for (CNode<T>* a = head->next; a && a != tail; a = a->next)
        cout << a->value << "->";
    cout << "NULL" << endl;

}


//------------------------------------- TEST -------------------------------------//

int num_operaciones_insertar = 100;
int num_operaciones_buscar = 100;
int num_operaciones_borrar = 100;

int get_random(int low, int high) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribution(low, high);
    return distribution(gen);
}


struct Insert {

    int min_;
    int max_;
    CList<int>* ptr_;

    Insert(int min, int max, CList<int>* ptr) :min_(min), max_(max), ptr_(ptr) {}
    void operator()(int operaciones) {

        for (int i = 0; i < operaciones; i++) {
            cout << "INSERT" << endl;
            ptr_->insert(get_random(min_, max_));
        }
    }
};


struct Remove {

    int min_;
    int max_;
    CList<int>* ptr_;

    Remove(int min, int max, CList<int>* ptr) :min_(min), max_(max), ptr_(ptr) {}
    void operator()(int operaciones) {

        for (int i = 0; i < operaciones; i++) {
            cout << "REMOVE" << endl;
            ptr_->remove(get_random(min_, max_));
        }
    }
};



struct Contain {

    int min_;
    int max_;
    CList<int>* ptr_;

    Contain(int min, int max, CList<int>* ptr) :min_(min), max_(max), ptr_(ptr) {}
    void operator()(int operaciones) {

        for (int i = 0; i < operaciones; i++) {
            cout << "FIND" << endl;
            if (ptr_->contains(get_random(min_, max_))) {
                cout << "LO ENCONTRO" << endl;
            }
            else {
                cout << "NO LO ENCONTRO" << endl;
            }
        }
    }
};


// Línea 176 se pasan los parametros necesarios para realizar el TEST
int main()
{

    CList<int> A;
    vector<thread>insert_t;
    vector<thread>remove_t;
    vector<thread>contain_t;

    for (int i = 0; i < 8; i++) {


        insert_t.emplace_back(thread(Insert(0, 100000, &A), num_operaciones_insertar));
        contain_t.emplace_back(thread(Contain(0, 100000, &A), num_operaciones_buscar));
        remove_t.emplace_back(thread(Remove(0, 100000, &A), num_operaciones_borrar));

    }


    for (int i = 0; i < 8; i++) {
        insert_t[i].join();
        contain_t[i].join();
        remove_t[i].join();

    }
    A.print();
    cout << endl << endl;

    // Al momento de realizar la insercion,busqueda y eliminacion se usan contadores que son variables atomicas (lineas 14 a 16),
    // con el fin de poder evitar el RACE CONDITION y ver si es que se realizó correctamente la insercion,busqueda o eliminacion, como son 8 threads
    // entonces el resultado esperado debe ser el numero de operaciones a insertar,buscar o eliminar (lineas 178 a 180) por la cantidad de threads
    // que se esta usando, por ejemplo el numero de operaciones para realizar la insercion es 100 (linea 178) y se esta usando 8 threads,
    // por lo tanto la cantidad de veces que se realiza la insercion en total es 800,
    // contador_insertar = num_operaciones_insertar * cantidad de threads --> contador_insertar = 100*8= 800.
    // Mismo procedimiento para la busqueda y eliminacion.

    cout << "Cantidad de veces que se realizo la insercion: " << contador_insertar << endl;
    cout << "Cantidad de veces que se realizo la busqueda: " << contador_buscar << endl;
    cout << "Cantidad de veces que se realizo la eliminacion: " << contador_eliminar << endl;
    return 0;
}