#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <math.h>
#include <iterator>
#include <limits>
#include <mutex>
#include <map>
#include<thread>
#include <atomic>
#include <random>
#include <stdlib.h>
#include <chrono>

#include "concurrent_skip_list.hpp
#include "sequential_skip_list.hpp"

using namespace std;



//------------------------------------------------- TEST 1: MEDICION DE TIEMPOS -------------------------------------------------//

int num_threads;
Concurrent::SkipList<int> concurrent_skiplist(16);

vector<int> numbers_insert;
vector<int> numbers_delete;
vector<int> numbers_search;


template <typename ElemType>
ElemType random_number_() {	//Funcion para generar numeros aleatorios. 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 50000000);

    return dis(gen);
}

void generate_random_numbers() {    // Se genera numeros aleatorios y se los guarda en un vector para hacer las pruebas.

    generate(numbers_insert.begin(), numbers_insert.end(), random_number_<int>);

    generate(numbers_delete.begin(), numbers_delete.end(), random_number_<int>);

    generate(numbers_search.begin(), numbers_search.end(), random_number_<int>);
}


void skiplist_insert(int inicio, int fin) { 
    for (int i = inicio; i < fin; i++) {
        concurrent_skiplist.insert(numbers_insert[i]);
    }
}

void skiplist_remove(int inicio, int fin) {
    for (int i = inicio; i < fin; i++) {
        concurrent_skiplist.remove(numbers_delete[i]);
    }
}

void skiplist_search(int inicio, int fin) {
    for (int i = inicio; i < fin; i++) {
        concurrent_skiplist.search(numbers_search[i]);
    }
}

void skiplist_optimized_search(int inicio, int fin) {
    for (int i = inicio; i < fin; i++) {
        concurrent_skiplist.optimized_search(numbers_search[i]);
    }
}

void PruebaInsert() {   // Función que llamar a los threads para realizar la inserción.

    vector<thread> vec_threads;
    int resto = numbers_insert.size() % num_threads;
    int aux = floor(numbers_insert.size() / num_threads);
    int fin = 0;
    int inicio = 0;
    int num_operaciones_asignadas = 0;

    for (int i = 0; i < num_threads; i++) {

        if (i < resto) num_operaciones_asignadas = aux + 1;
        else num_operaciones_asignadas = aux;
        inicio = fin;
        fin = fin + num_operaciones_asignadas;
        vec_threads.push_back(thread(skiplist_insert, inicio, fin));
    }
    
    for (auto& i : vec_threads) i.join();
    vec_threads.clear();
}

void PruebaRemove() {   // Función que llamar a los threads para realizar la Eliminación.

    vector<thread> vec_threads;
    int resto = numbers_delete.size() % num_threads;
    int aux = floor(numbers_delete.size() / num_threads);
    int fin = 0;
    int inicio = 0;
    int num_operaciones_asignadas = 0;

    for (int i = 0; i < num_threads; i++) {

        if (i < resto) num_operaciones_asignadas = aux + 1;
        else num_operaciones_asignadas = aux;
        inicio = fin;
        fin = fin + num_operaciones_asignadas;
        vec_threads.push_back(thread(skiplist_remove, inicio, fin));

    }
    
    for (auto& i : vec_threads) i.join();
    vec_threads.clear();
}

void PruebaSearch() {   // Función que llamar a los threads para realizar la busqueda.

    vector<thread> vec_threads;
    int resto = numbers_search.size() % num_threads;
    int aux = floor(numbers_search.size() / num_threads);
    int fin = 0;
    int inicio = 0;
    int num_operaciones_asignadas = 0;

    for (int i = 0; i < num_threads; i++) {

        if (i < resto) num_operaciones_asignadas = aux + 1;
        else num_operaciones_asignadas = aux;
        inicio = fin;
        fin = fin + num_operaciones_asignadas;
        vec_threads.push_back(thread(skiplist_search, inicio, fin));
    }

    for (auto& i : vec_threads) i.join();
    vec_threads.clear();
}

void PruebaOptimizedSearch() {  // Función que llamar a los threads para realizar la busqueda optimizada.

    vector<thread> vec_threads;
    int resto = numbers_search.size() % num_threads;
    int aux = floor(numbers_search.size() / num_threads);
    int fin = 0;
    int inicio = 0;
    int num_operaciones_asignadas = 0;

    for (int i = 0; i < num_threads; i++) {

        if (i < resto) num_operaciones_asignadas = aux + 1;
        else num_operaciones_asignadas = aux;
        inicio = fin;
        fin = fin + num_operaciones_asignadas;
        vec_threads.push_back(thread(skiplist_optimized_search, inicio, fin));
    }

    for (auto& i : vec_threads) i.join();
    vec_threads.clear();
}

void Measuring_time_concurrent(int n_threads, int elements) {   // Función para calcular el tiempo de la SkipList Concurrente.

    num_threads = n_threads;
    numbers_insert.resize(elements);
    numbers_delete.resize(elements);
    numbers_search.resize(elements);
    
    cout << "Generando numeros aleatorios....." << endl << endl;
    generate_random_numbers();
    cout << "Numeros aleatorios generados correctamente" << endl << endl;

    cout << "\n----------  TEST MEDICION DE TIEMPO SKIPLIST PARALELO  ------------" << endl;
    cout << "\n----------  Prueba de Tiempo con " << num_threads << " Threads  ----------" << endl << endl;

   
    auto start1 = chrono::steady_clock::now();
    PruebaInsert();
    auto end1 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Insertar: " << chrono::duration_cast<chrono::microseconds>(end1 - start1).count() / 1000000.0 << " segundos" << endl;


   /* auto start2 = chrono::steady_clock::now();
    PruebaSearch();
    auto end2 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Buscar: " << chrono::duration_cast<chrono::microseconds>(end2 - start2).count() / 1000000.0 << " segundos" << endl;*/



    auto start5 = chrono::steady_clock::now();
    PruebaOptimizedSearch();
    auto end5 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Buscar Optimizado: " << chrono::duration_cast<chrono::microseconds>(end5 - start5).count() / 1000000.0 << " segundos" << endl;


    auto start3 = chrono::steady_clock::now();
    PruebaRemove();
    auto end3 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Eliminar: " << chrono::duration_cast<chrono::microseconds>(end3 - start3).count() / 1000000.0 << " segundos" << endl;

    //concurrent_skiplist.print();
}


void Measuring_time_sequential(int elements) {   // Función para calcular el tiempo de la SkipList Secuencial.

    cout << endl;
    cout << "\n----------  TEST MEDICION DE TIEMPO SKIPLIST SECUENCIAL  ----------" << endl;

    Sequential::SkipList<int> sequential_skip_list(16);

    auto start5 = chrono::steady_clock::now();
    for (int i = 0; i < elements; i++) {
        //int x = random_number_<int>();
        sequential_skip_list.insert(numbers_insert[i]);
    }
    auto end5 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Insertar: " << chrono::duration_cast<chrono::microseconds>(end5 - start5).count() / 1000000.0 << " segundos" << endl;

 
    /*auto start6 = chrono::steady_clock::now();
    for (int i = 0; i < elements; i++) {
        int x = random_number_<int>();
        sequential_skip_list.search(numbers_search[i]);
    }
    auto end6 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Buscar: " << chrono::duration_cast<chrono::microseconds>(end6 - start6).count() / 1000000.0 << " segundos" << endl;*/


    auto start7 = chrono::steady_clock::now();
    for (int i = 0; i < elements; i++) {
        //int x = random_number_<int>();
        sequential_skip_list.optimized_search(numbers_search[i]);
    }
    auto end7 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Buscar Optimizado: " << chrono::duration_cast<chrono::microseconds>(end7 - start7).count() / 1000000.0 << " segundos" << endl;


    auto start8 = chrono::steady_clock::now();
    for (int i = 0; i < elements; i++) {
        int x = random_number_<int>();
        sequential_skip_list.remove(numbers_delete[i]);
    }
    auto end8 = chrono::steady_clock::now();
    cout << "Tiempo transcurrido Eliminar: " << chrono::duration_cast<chrono::microseconds>(end8 - start8).count() / 1000000.0 << " segundos" << endl;

}

//-------------------------------------------------------------------------------------------------------------------------------//





//---------------------------------------------- TEST 2: OPERACIONES EN PARALELO ------------------------------------------------//

int num_operaciones_insertar = 10000;  //cantidad de veces que se realizará la insercion en cada thread.
int num_operaciones_buscar = 10000;    //cantidad de veces que se realizará la busqueda en cada thread.
int num_operaciones_borrar = 10000;    //cantidad de veces que se realizará la eliminacion en cada thread.

int get_random(int low, int high) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribution(low, high);
    return distribution(gen);
}

struct Insert {

    int min_;
    int max_;
    Concurrent::SkipList<int>* ptr_;

    Insert(int min, int max, Concurrent::SkipList<int>* ptr) :min_(min), max_(max), ptr_(ptr) {}
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
    Concurrent::SkipList<int>* ptr_;

    Remove(int min, int max, Concurrent::SkipList<int>* ptr) :min_(min), max_(max), ptr_(ptr) {}
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
    Concurrent::SkipList<int>* ptr_;

    Contain(int min, int max, Concurrent::SkipList<int>* ptr) :min_(min), max_(max), ptr_(ptr) {}
    void operator()(int operaciones) {

        for (int i = 0; i < operaciones; i++) {
            cout << "FIND" << endl;
            if (ptr_->optimized_search(get_random(min_, max_))) {
                cout << "LO ENCONTRO" << endl;
            }
            else {
                cout << "NO LO ENCONTRO" << endl;
            }
        }
    }
};


void All_operations_in_parallel(int n_threads) {    // Función para realizar las operaciones de insertar, borrar y buscar en paralelo, lo enseñó el profe Eddie.

    cout << "\n----------  TEST: OPERACIONES EN PARALELO  ----------" << endl;

    Concurrent::SkipList<int> concurrent_skiplist2(16);   // cantidad de niveles que tendra la SkipList.
    vector<thread>insert_t;
    vector<thread>remove_t;
    vector<thread>contain_t;

    int min = 0;        // valor minimo para generar los numeros aleatorios.
    int max = 1000000;  // valor maximo para generar los numeros aleatorios.

    for (int i = 0; i < n_threads; i++) {

        insert_t.emplace_back(thread(Insert(min, max, &concurrent_skiplist2), num_operaciones_insertar));
        contain_t.emplace_back(thread(Contain(min, max, &concurrent_skiplist2), num_operaciones_buscar));
        remove_t.emplace_back(thread(Remove(min, max, &concurrent_skiplist2), num_operaciones_borrar));
    }


    for (int i = 0; i < n_threads; i++) {

        insert_t[i].join();
        contain_t[i].join();
        remove_t[i].join();
    }

    //concurrent_skiplist2.print();
    cout << endl << endl;

    /*
       Al momento de realizar la insercion,busqueda y eliminacion se usan contadores que son variables atomicas,con el fin de poder
       evitar el RACE CONDITION y ver si es que se realizó correctamente la insercion,busqueda o eliminacion, como son 8 threads
       entonces el resultado esperado debe ser el numero de operaciones a insertar,buscar o eliminar (lineas 263 a 265) por la cantidad de threads
       que se esta usando, por ejemplo el numero de operaciones para realizar la insercion es 10000 (linea 263) y se esta usando 8 threads,
       por lo tanto la cantidad de veces que se realiza la insercion en total es 80000,
       contador_insertar = num_operaciones_insertar * cantidad de threads  ---->  contador_insertar = 10000*8 = 80000.
       Mismo procedimiento para la busqueda y eliminacion.
    */
    cout << "Cantidad de veces que se realizo la insercion: " << concurrent_skiplist2.contador_insertar << endl;
    cout << "Cantidad de veces que se realizo la busqueda: " << concurrent_skiplist2.contador_buscar << endl;
    cout << "Cantidad de veces que se realizo la eliminacion: " << concurrent_skiplist2.contador_eliminar << endl;
}

//-------------------------------------------------------------------------------------------------------------------------------//




int main()
{
   
    int n_threads = 8;  // cantidad de threads que se usaran para las pruebas.
    int elements = 1000000; //cantidad de elementos que se usaran para la prueba de Tiempo.

   
    Measuring_time_concurrent(n_threads, elements);    // como parametro recibe la cantidad de threads y la cantidad de elementos para realizar cada operacion de manera individual y tomar el tiempo que se demora.
    Measuring_time_sequential(elements);    // como parametro recibe la cantidad de elementos para realizar cada operacion
  
    /*
     Este es el test que el profesor Eddie nos enseñó en las clases de laboratorio, lo que hace es ejecutar las 3 operaciones en paralelo, 
     para probarlo solo descomentar la linea 392 y esperar que acabe la ejecucion.
    */

    //All_operations_in_parallel(n_threads); // como parametro recibe la cantidad de threads que se usaran para realizar las 3 operaciones en paralelo.

    return 0;

}

