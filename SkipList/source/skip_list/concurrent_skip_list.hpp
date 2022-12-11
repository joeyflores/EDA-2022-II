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

using namespace std;


namespace Concurrent {

    template <typename ElemType>
    struct SkipListNode {

        ElemType value;
        vector<SkipListNode<ElemType>*> next;
        mutex node_lock;
        bool marked = false;    // Si el nodo está marcado significa que en paralelo ya hay otro thread que está realizando la operación de eliminar.
        bool fully_linked = false;  // Se vuelve verdadero después de que un nodo se haya vinculado en todos sus niveles.
        int top_level;

        SkipListNode(int level, ElemType value_ = ElemType());
        void lock();
        void unlock();
    };

    template <typename ElemType>
    SkipListNode<ElemType>::SkipListNode(int level, ElemType value_) {
        value = value_;
        next.resize(level + 1);
        for (size_t i = 0; i < next.size(); i++) {
            next[i] = nullptr;
        }
        top_level = level;
    }

    template <typename ElemType>
    void SkipListNode<ElemType>::lock() {
        node_lock.lock();
    }

    template <typename ElemType>
    void SkipListNode<ElemType>::unlock() {
        node_lock.unlock();
    }



    template <typename ElemType>
    class SkipList {
    private:

        SkipListNode<ElemType>* head;
        SkipListNode<ElemType>* tail;
        int max_level;
        ElemType min = numeric_limits<ElemType>::min();
        ElemType max = numeric_limits<ElemType>::max();

    public:
        SkipList() {};
        SkipList(int max_level_);
        ~SkipList();

        bool FindNode(ElemType value, vector<SkipListNode<ElemType>*>& predecessors, vector<SkipListNode<ElemType>*>& successors, int& current_level);
        bool insert(ElemType value);
        bool search(ElemType value);    
        bool optimized_search(ElemType value);   // search optimizado
        bool remove(ElemType value);
        void print();

        bool readyToDelete(SkipListNode<ElemType>* candidate, int lFound);
        int random_number();
        int random_level();

        atomic<int> contador_insertar = 0;
        atomic<int> contador_buscar = 0;
        atomic<int>contador_eliminar = 0;
    };



    template <typename ElemType>
    int SkipList<ElemType>::random_number() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        return dis(gen);
    }

    template <typename ElemType>
    int SkipList<ElemType>::random_level() {
        int level = 0;
        while (level < max_level) {
            int aux = random_number();
            if (aux == 1) {	//Si el numero aleatorio es 1 el nivel del nodo incrementa en 1.
                level++;
            }
            else {	//Si el numero aleatorio es 0 entonces se detiene y retorna el nivel actual que tendra nuestro nodo a insertar.
                break;
            }
        }
        return level;
    }

    template <typename ElemType>
    SkipList<ElemType>::SkipList(int max_level_) {  // Constructor.

        max_level = max_level_;
        head = new SkipListNode<ElemType>(max_level, min);
        tail = new SkipListNode<ElemType>(max_level, max);

        for (size_t i = 0; i < head->next.size(); i++) {
            head->next[i] = tail;
        }
    }

    /*
       Esta funcion guarda los predecesores y sucesores en cada nivel del nodo a insertar o eliminar,
       y devuelve verdadero si es que se encontró el elemento, y caso contrario devuelve falso.
    */
    template <typename ElemType>
    bool SkipList<ElemType>::FindNode(ElemType value, vector<SkipListNode<ElemType>*>& predecessors, vector<SkipListNode<ElemType>*>& successors, int& current_level) {

        predecessors.resize(max_level + 1);
        successors.resize(max_level + 1);

        for (size_t i = 0; i < predecessors.size(); i++) {
            predecessors[i] = nullptr;
            successors[i] = nullptr;
        }

        bool found = false;
        SkipListNode<ElemType>* prev = head;

        for (int level = max_level; level >= 0; level--) {
            SkipListNode<ElemType>* curr = prev->next[level];

            while (curr->value < value) {
                prev = curr;
                curr = prev->next[level];
            }

            if (!found && curr->value == value) {
                current_level = level;  // Guardar el nivel actual en el que se encontró el elemento.
                found = true;
            }

            predecessors[level] = prev;
            successors[level] = curr;
        }
        return found;
    }



    template <typename ElemType>
    bool SkipList<ElemType>::insert(ElemType value) {

        int top_level = random_level(); // Nivel que tendra el nodo a insertar.
        vector<SkipListNode<ElemType>*> preds;
        vector<SkipListNode<ElemType>*> succs;
        int current_level = 0;

        /* Este bucle ayuda a intentar la inserción nuevamente en caso de que el nodo este marcado, o se cambien los predecesores
           y sucesores. */

        while (true) {

            bool found = FindNode(value, preds, succs, current_level);  // Encontrar los predecesores y sucesores del elemento a insertar.

            // Si el elemento ya está presente en la SkipList y el nodo no está marcado, entonces no insertamos el elemento ya que la SkipList no acepta repeticiones.
            // Si el elemento está presente y el nodo no esta completamente vinculado, entonces esperamos hasta que lo esté (no se necesita inserción).
            // Si el elemento está presente y el nodo está marcado, significa que algún otro subproceso está en proceso de eliminar ese nodo, 
            // por lo que el subproceso que realiza la operación de insertar lo intenta después.
            // Y si el elemento no está presente en la SkipList simplemente se continua con el método de inserción.

            if (found) {
                SkipListNode<ElemType>* SkipListNode_found = succs[current_level];
                if (!SkipListNode_found->marked) {
                    while (!SkipListNode_found->fully_linked) {}
                    contador_insertar++;    // Contar la cantidad de veces que se realiza la inserción.
                    return false;           // Retorna falso si no se insertó el elemento.
                }
                continue;
            }


            int highestLocked = -1; /* Esta variable nos ayuda a evitar el interbloqueo (bloquear un nodo que ya fue anteriormente bloqueado),
                                       esto puede ocurrir cuando tenemos el mismo predecesor en diferentes niveles. */

            SkipListNode<ElemType>* pred;
            SkipListNode<ElemType>* succ;
            SkipListNode<ElemType>* aux = nullptr;
            bool valid = true;  /* Esta variable se usa para asegurarnos de que tanto el predecesor como el sucesor no estén marcados
                                   y que el siguiente del predecesor sea el sucesor en cada nivel. */

            for (int level = 0; valid && (level <= top_level); level++) {
                pred = preds[level];
                succ = succs[level];

                if (pred != aux) {
                    pred->lock();   // Bloquear todos los predecesores del nodo a insertar.
                    highestLocked = level;
                    aux = pred;
                }

                valid = !(pred->marked) && !(succ->marked) && pred->next[level] == succ;
            }

            if (!valid) {   /* En caso de que no se cumplan las condiciones anteriores, se libera el bloqueo de los predecesores
                               y se intenta realizar la operación de insertar después. */

                SkipListNode<ElemType>* tmp = nullptr;
                for (int level = 0; level <= highestLocked; level++)
                {
                    if (tmp == nullptr || tmp != preds[level]) {
                        preds[level]->unlock();
                    }
                    tmp = preds[level];
                }
                continue;
            }

            // Si se cumplen las condiciones anteriores, entonces  se garantiza que la operación de inserción tendrá éxito.

            SkipListNode<ElemType>* new_SkipListNode = new SkipListNode<ElemType>(top_level, value);    // Se crea un nuevo nodo.
            for (int level = 0; level <= top_level; level++) {
                new_SkipListNode->next[level] = succs[level];   // Se vincula el nuevo nodo con sus sucesores.
                preds[level]->next[level] = new_SkipListNode;   // Se vincula los predecesores con el nuevo nodo.
            }
            new_SkipListNode->fully_linked = true;  // Una vez que se haya terminado el proceso de vinculación, el nodo se marca como totalmente vinculado.

            SkipListNode<ElemType>* tmp = nullptr;
            for (int level = 0; level <= highestLocked; level++)    // Luego de realizar la inserción se libera los bloqueos de los predecesores en cada nivel.
            {
                if (tmp == nullptr || tmp != preds[level]) {
                    preds[level]->unlock();
                }
                tmp = preds[level];
            }

            contador_insertar++;    // Contar la cantidad de veces que se realiza la inserción.
            return true;            // Retorna verdadero si se insertó el elemento.
        }
    }

    /*
       La operación search simplemente llama a FindNode y devuelve verdadero si se encontró el nodo con ese elemento,
       si ese nodo esta completamente vinculado y si ese nodo no está marcado.
       Esta función no es tan eficiente debido a que si encuentra el elemento sigue bajando hasta llegar al primer nivel,
       lo cual tarda mas tiempo.
     */

    template <typename ElemType>
    bool SkipList<ElemType>::search(ElemType value) {

        vector<SkipListNode<ElemType>*> preds;
        vector<SkipListNode<ElemType>*> succs;
        int current_level;
        bool found = FindNode(value, preds, succs, current_level);  // Retorna verdadero si existe el elemento a buscar, caso contrario retorna falso.

        return(found && succs[current_level]->fully_linked && !succs[current_level]->marked);
    }

    /*
       Está es una función auxiliar que nos ayuda a comprobar que el nodo está completamente vinculado, no está marcado y
       se encontró en su nivel superior.
    */

    template <typename ElemType>
    bool SkipList<ElemType>::readyToDelete(SkipListNode<ElemType>* candidate, int lFound) {

        return(candidate->fully_linked && candidate->top_level == lFound && !candidate->marked);
    }


    template <typename ElemType>
    bool SkipList<ElemType>::remove(ElemType value) {

        SkipListNode<ElemType>* nodeToDelete = nullptr; // Puntero al nodo a eliminar.
        bool is_marked = false; /* Esta variable nos ayuda a saber si es que se está intentando realizar la eliminación nuevamente para evitar bloquear
                                   de nuevo los predecesores del nodo a eliminar. */
        int top_level = -1;

        vector<SkipListNode<ElemType>*> preds;
        vector<SkipListNode<ElemType>*> succs;
        int current_level = 0;

        /* Este bucle ayuda a intentar la eliminación nuevamente en caso de que se cambien los predecesores y sucesores. */
        while (true) {

            bool found = FindNode(value, preds, succs, current_level);  // Encontrar los predecesores y sucesores del elemento a eliminar.

            // Si el elemento existe entonces apuntamos a ese nodo para que sea eliminado más adelante.
            if (found) {
                nodeToDelete = succs[current_level];
            }

            /* Si el nodo cumple con los requisitos para ser eliminado (readyToDelete) entonces se bloquea el nodo( línea 313) y se verfica
               que todavía no esté marcado. Si es así, se marca el nodo (línea 320) */
            if (is_marked || (found && readyToDelete(succs[current_level], current_level))) {

                if (!is_marked) {
                    top_level = nodeToDelete->top_level;
                    nodeToDelete->lock();
                    if (nodeToDelete->marked) { // Si el nodo a eliminar ya está marcado significa que otro thread ya está intentando eliminar este elemento.
                        nodeToDelete->unlock(); // Por lo tanto se desbloquea el nodo.
                        contador_eliminar++;    // Contar la cantidad de veces que se realiza la eliminación.
                        return false;           // Retorna falso si no se eliminó el elemento.
                    }
                    nodeToDelete->marked = true;
                    is_marked = true;
                }

                int highestLocked = -1; /* Esta variable nos ayuda a evitar el interbloqueo (bloquear un nodo que ya fue anteriormente bloqueado),
                                           esto puede ocurrir cuando tenemos el mismo predecesor en diferentes niveles. */

                SkipListNode<ElemType>* pred;
                SkipListNode<ElemType>* tmp = nullptr;

                bool valid = true;      /* Esta variable se usa para asegurarnos de que tanto el predecesor como el sucesor no estén marcados
                                           y que el siguiente del predecesor sea el nodo a eliminar. */

                for (int level = 0; valid && (level <= top_level); level++) {
                    pred = preds[level];

                    if (tmp != preds[level]) {
                        pred->lock();   // Bloquear todos los predecesores del nodo a eliminar.
                    }
                    tmp = preds[level];
                    highestLocked = level;
                    valid = !pred->marked && pred->next[level] == nodeToDelete;

                }

                if (!valid) {    /* En caso de que no se cumplan las condiciones anteriores, se libera el bloqueo de los predecesores
                                   y se intenta realizar la operación de eliminar después. */

                    SkipListNode<ElemType>* tmp = nullptr;
                    for (int level = 0; level <= highestLocked; level++)
                    {
                        if (tmp == nullptr || tmp != preds[level]) {
                            preds[level]->unlock();
                        }
                        tmp = preds[level];
                    }
                    continue;
                }

                // Si se cumplen las condiciones anteriores, entonces  se garantiza que la operación de inserción tendrá éxito.

                for (int level = top_level; level >= 0; level--) {
                    preds[level]->next[level] = nodeToDelete->next[level];  // Se vincula los predecesores a los sucesores del nodo a eliminar.
                }

                nodeToDelete->unlock(); // Una vez que termina el proceso de vinculación se desbloquea el nodo.
                tmp = nullptr;
                delete nodeToDelete;    // Se elimina el nodo de la SkipList.

                for (int level = 0; level <= highestLocked; level++) {  // Luego de realizar la eliminación se libera los bloqueos de los predecesores en cada nivel.
                    if (tmp == nullptr || tmp != preds[level]) {
                        preds[level]->unlock();
                    }
                    tmp = preds[level];
                }
                contador_eliminar++;    // Contar la cantidad de veces que se realiza la eliminación.
                return true;            // Retorna verdadero si se insertó el elemento.

            }
            else {
                contador_eliminar++;    // Contar la cantidad de veces que se realiza la eliminación.
                return false;           // Retorna falso si no se eliminó el elemento.
            }
        }
    }

    /*
       La operación de optimized_search empieza desde el nivel más alto de la SkipList, apenas encuentre el elemento a buscar se
       sale del bucle y luego verifica si el nodo que contiene el elemento esta completamente vinculado y no está marcado.
       Si cumple con todo esto retorna verdadero, caso contrario retorna falso.
       Está función a diferencia del anterior Search, no necesita irse hasta el primer nivel de la SkipList, por lo tanto
       es mucho mas eficiente y demora menos tiempo que la anterior.
    */
    template <typename ElemType>
    bool SkipList<ElemType>::optimized_search(ElemType value) {

        SkipListNode<ElemType>* p = head;
        SkipListNode<ElemType>* current_node = nullptr;
        bool found = false;

        for (int i = max_level; i >= 0; i--) {
            current_node = p->next[i];
            while (current_node && current_node->value < value) {
                p = current_node;
                current_node = p->next[i];
            }
            if (current_node && current_node->value == value) {
                found = true; // Si se encuentra el elemento a buscar se sale del bucle debido a que no es necesario seguir buscando.
                break;
            }
        }
        contador_buscar++;  // Contar la cantidad de veces que se realiza la busqueda.
        return(found && current_node->fully_linked && !current_node->marked);
    }

    template <typename ElemType>
    void SkipList<ElemType>::print() {    // Imprimir los elementos por nivel de manera ascendente de la SkipList.
        SkipListNode<ElemType>* p = head;
        cout << "-----------------------------------------------" << endl << endl;
        for (int i = max_level; i >= 0; i--) {
            cout << "Nivel " << i << ": ";
            SkipListNode<ElemType>* q = p->next[i];
            while (q != tail) {
                cout << q->value << " ";
                q = q->next[i];
            }
            cout << "-> NULL ";
            cout << endl << endl;
        }
        cout << "-----------------------------------------------" << endl;
    }


    template <typename ElemType>
    SkipList<ElemType>::~SkipList() {   // Destructor.

        cout << endl;
        //cout << "Entro al destructor" << endl;
        SkipListNode<ElemType>* tmp;
        SkipListNode<ElemType>* p = head->next[0];
        while (p != tail) {
            tmp = p;
            p = p->next[0];
            delete tmp;
        }
        delete head;
        delete tail;
    }
}   // FIN NAMESPACE CONCURRENT

