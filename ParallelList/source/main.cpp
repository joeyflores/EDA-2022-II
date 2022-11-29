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
atomic<int> con = 0;


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
                //delete curr;
                flag = true;
            }
            pred->unlock_();    // se desbloquea el predecesor
            return flag;         // retorna Verdadero si es que se eliminó el elemento, caso contrario Falso
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
const int num_trits = 12;
const int kSize = 20;
CList<int> linkedlist;

vector<int> numbers_insert(kSize);
vector<int> numbers_delete(kSize);
vector<int> numbers_search(kSize);


template <typename ElemType>
ElemType random_number_() {	//Funcion para generar numeros aleatorios 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 40);

    return dis(gen);
}

void generate_random_numbers() { // funcion para llenar los vectores de numeros aleatorios para realizar la inserción, elimiación y busqueda

    generate(numbers_insert.begin(), numbers_insert.end(), random_number_<int>);

    generate(numbers_delete.begin(), numbers_delete.end(), random_number_<int>);

    generate(numbers_search.begin(), numbers_search.end(), random_number_<int>);
}


void linkedlist_add(int inicio, int fin) {  // funcion para dividir en cantidades iguales para realizar la inserción paralela
    if (fin >= numbers_insert.size()) fin = numbers_insert.size();
    if (inicio == fin) linkedlist.insert(numbers_insert[inicio]);
    for (int i = inicio; i < fin; i++) {
        con++;
        linkedlist.insert(numbers_insert[i]);
    }
}

void linkedlist_remove(int inicio, int fin) {    // funcion para dividir en cantidades iguales para realizar la eliminación paralela
    if (fin >= numbers_delete.size()) fin = numbers_delete.size();
    if (inicio == fin) linkedlist.remove(numbers_delete[inicio]);
    for (int i = inicio; i < fin; i++) {
        linkedlist.remove(numbers_delete[i]);
    }
}

void linkedlist_search(int inicio, int fin) {    // funcion para dividir en cantidades iguales para realizar la busqueda paralela
    if (fin >= numbers_search.size()) fin = numbers_search.size();
    if (inicio == fin) fin++;
    for (int i = inicio; i < fin; i++) {
        linkedlist.contains(numbers_search[i]);
    }
}



void PruebaInsert() {   // función para realizar la inserción con "n" threads

    vector<thread> threads;
    int intervalo = ceil(float(numbers_insert.size()) / num_trits);
    for (int i = 0; i < numbers_insert.size(); i = i + intervalo) {
        threads.push_back(thread(linkedlist_add, i, i + intervalo));
    }
    for (auto& th : threads) {
        th.join();
    }
    threads.clear();
}

void PruebaRemove() {   // función para realizar la eliminación con "n" threads

    vector<thread> threads;
    int intervalo = ceil(float(numbers_delete.size()) / num_trits);
    for (int i = 0; i < numbers_delete.size(); i = i + intervalo) {
        threads.push_back(thread(linkedlist_remove, i, i + intervalo));
    }
    for (auto& th : threads) {
        th.join();
    }
    threads.clear();
}

void PruebaSearch() {   // función para realizar la busqueda con "n" threads

    vector<thread> threads;
    int intervalo = ceil(float(numbers_search.size()) / num_trits);
    for (int i = 0; i < numbers_search.size(); i = i + intervalo) {
        threads.push_back(thread(linkedlist_search, i, i + intervalo));
    }
    for (auto& th : threads) {
        th.join();
    }
    threads.clear();
}

int random_number(int max_num) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max_num);
    return dis(gen);
}

void linkedlist_combined() {

    int inicio = random_number(numbers_insert.size());
    int fin = inicio + (random_number(numbers_insert.size() - inicio));
    linkedlist_add(inicio, fin);
    linkedlist.print();
    inicio = random_number(numbers_delete.size());
    fin = inicio + (random_number(numbers_delete.size() - inicio));
    linkedlist_remove(inicio, fin);
    linkedlist.print();
    inicio = random_number(numbers_search.size());
    fin = inicio + (random_number(numbers_search.size() - inicio));
    linkedlist_search(inicio, fin);
}



// Línea 170 se pasan los parametros necesarios para realizar el TEST
int main()
{
    cout << "\n----------  Test LinkedList with " << num_trits << " Trits----------" << endl<<endl;

    generate_random_numbers();  // llenar vectores con numeros aleatorios

    cout << "ACABO generate_random_numbers" << endl<<endl;

    cout << "Numeros a insertar: " << numbers_insert.size() << endl;
    copy(numbers_insert.begin(), numbers_insert.end(), std::ostream_iterator<int>(std::cout, " , "));  //Funcion "copy"
    cout << endl << endl;

    //---------------------------------------------- TIEMPO INSERTAR ----------------------------------------------//
    auto start1 = chrono::steady_clock::now();
    PruebaInsert();
    auto end1 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Insertar: " << chrono::duration_cast<chrono::microseconds>(end1 - start1).count() / 1000000.0 << " segundos" << endl;
    //-------------------------------------------------------------------------------------------------------------//


    linkedlist.print();
    cout << "--------------------------------------------------------------------------------------------------------" << endl<<endl;
    cout << "Numeros a buscar: " << numbers_search.size() << endl;
    copy(numbers_search.begin(), numbers_search.end(), std::ostream_iterator<int>(std::cout, " , "));  //Funcion "copy"
    cout << endl << endl;


    //---------------------------------------------- TIEMPO BUSCAR ----------------------------------------------//
    auto start2 = chrono::steady_clock::now();
    PruebaSearch();
    auto end2 = chrono::steady_clock::now();
    cout << "tiempo transcurrido Buscar: " << chrono::duration_cast<chrono::microseconds>(end2 - start2).count() / 1000000.0 << " segundos" << endl;
    //-----------------------------------------------------------------------------------------------------------//
        
         
    linkedlist.print();
    cout << "--------------------------------------------------------------------------------------------------------" << endl;
    cout << endl;
    cout << "Numeros a eliminar: " << numbers_delete.size() << endl;
    copy(numbers_delete.begin(), numbers_delete.end(), std::ostream_iterator<int>(std::cout, " , "));  //Funcion "copy"
    cout << endl << endl;

        //---------------------------------------------- TIEMPO ELIMINAR ----------------------------------------------//
    auto start3 = chrono::steady_clock::now();
    PruebaRemove();
    auto end3 = chrono::steady_clock::now();
    cout << "tiempo transcurrido Eliminar: " << chrono::duration_cast<chrono::microseconds>(end3 - start3).count() / 1000000.0 << " segundos" << endl;
    //--------------------------------------------------------------------------------------------------------------//
        
        
    linkedlist.print();
    cout << "--------------------------------------------------------------------------------------------------------" << endl;

 
   return 0;
}