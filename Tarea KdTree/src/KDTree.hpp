// Copyright

#ifndef SRC_KDTREE_HPP_
#define SRC_KDTREE_HPP_

#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <limits>
#include <unordered_map>
#include "Point.hpp"

using namespace std;


//-------------BOUNDED PRIORITY QUEUE------------//
template <typename ElemType>
class BoundedPQueue {
    multimap<double, ElemType> mapita;
    int tam_max;
public:
    BoundedPQueue(int tam_max_);
    bool empty() const;    
    int size() const;
    int max_size() const;
    void push(double distance, ElemType &value);
    void pop();
    ElemType front();
    void print();
    double mejor_distancia() const;
    

};

template <typename ElemType>
BoundedPQueue<ElemType>::BoundedPQueue(int tam_max_) {
    tam_max = tam_max_;
}

template <typename ElemType>
bool BoundedPQueue<ElemType>::empty() const {
    return mapita.empty();
}

template <typename ElemType>
int BoundedPQueue<ElemType>::size() const {
    return mapita.size();
}

template <typename ElemType>
int BoundedPQueue<ElemType>::max_size() const {
    return tam_max;
}

template <typename ElemType>
void BoundedPQueue<ElemType>::push(double distance,  ElemType &value) {
    
    mapita.insert(make_pair(distance, value));

    if (size() > max_size()) {
        typename multimap<double, ElemType>::iterator it = mapita.end();
        it--; 
        mapita.erase(it);
    }
}

template <typename ElemType>
void BoundedPQueue<ElemType>::pop() {

    mapita.erase(mapita.begin());
}

template <typename ElemType>
ElemType BoundedPQueue<ElemType>::front() {

    ElemType value = mapita.begin()->second;
    return value;
}

template <typename ElemType>
double BoundedPQueue<ElemType>::mejor_distancia() const {
    return empty() ? std::numeric_limits<double>::infinity() : mapita.rbegin()->first;
}


template <typename ElemType>
void BoundedPQueue<ElemType>::print() {
    cout << "Distancia -> Valor" << endl;
    for (auto i : mapita) {
        cout << i.first << " -> " << i.second << endl;
    }
}
//-----------------------------------------------//


//------------------KDTREE NODE------------------//
template <size_t N, typename ElemType>
struct KDTreeNode {

    Point<N> point;
    KDTreeNode* nodes[2];
    ElemType value;
    KDTreeNode(const Point<N>& _point,const ElemType& _value = ElemType()) {

        nodes[0] = nodes[1] = 0;
        point = _point;
        value = _value;

    }
};
//-----------------------------------------------//


//------------------ARBOL KDTREE-----------------//
template <size_t N, typename ElemType>
class KDTree {
public:
    typedef std::pair<Point<N>, ElemType> value_type;

    KDTree();
    ~KDTree();

    KDTree(const KDTree& rhs);
    KDTree& operator=(const KDTree& rhs);

    size_t dimension() const;
    size_t size() const;
    bool empty() const;

    bool contains(const Point<N>& pt) const;

    void insert(const Point<N>& pt, const ElemType& value= ElemType());

    ElemType& operator[](const Point<N>& pt);

    ElemType& at(const Point<N>& pt);
    const ElemType& at(const Point<N>& pt) const;

    ElemType knn_value(const Point<N>& key, size_t k) const;

    std::vector<ElemType> knn_query(const Point<N>& key, size_t k) const;

    //--------------------------FUNCIONES CREADAS POR MI--------------------------//
    bool find(const Point<N>& pt, KDTreeNode<N, ElemType>**& it) const; 
    void print(KDTreeNode<N, ElemType>* tmp, int conta = 0); 
    void DestructorRecursivo(KDTreeNode<N, ElemType>* nodo);
    void DestructorIterativo(KDTreeNode<N, ElemType>* nodo);
    KDTreeNode<N, ElemType>* CopiarArbol(KDTreeNode<N, ElemType>* root_);   
    void find_neighbors(KDTreeNode<N, ElemType>* current_node, BoundedPQueue<ElemType>& nearest_neighbors_candidates, int depth, const Point<N> key) const;
    mutable KDTreeNode<N, ElemType>* root;
    //----------------------------------------------------------------------------//
private:
    size_t dimension_;
    size_t size_;
};


template <size_t N, typename ElemType>
KDTree<N, ElemType>::KDTree() { //Constructor del árbol KDTree.
    dimension_ = N;
    root = 0;
    size_ = 0;
}

template <size_t N, typename ElemType>
KDTree<N, ElemType>::~KDTree() { //Destructor del árbol KDTree.

    //DestructorRecursivo(root); 
    //cout << endl;
    DestructorIterativo(root);
    
}

template <size_t N, typename ElemType>
KDTree<N, ElemType>::KDTree(const KDTree& rhs) {    //Copia el contenido de otro KdTree en este.

    root = CopiarArbol(rhs.root);
    size_ = rhs.size_;
    dimension_ = rhs.dimension_;
}

template <size_t N, typename ElemType>
KDTree<N, ElemType>& KDTree<N, ElemType>::operator=(const KDTree& rhs) { 
    if (this != &rhs) { 
        //DestructorRecursivo(root);
        DestructorIterativo(root);
        root = CopiarArbol(rhs.root);
        size_ = rhs.size_;
    }
    return *this;
}

template <size_t N, typename ElemType>
size_t KDTree<N, ElemType>::dimension() const { //Devuelve la cantidad de dimensiones.
    return dimension_;
}

template <size_t N, typename ElemType>
size_t KDTree<N, ElemType>::size() const {  //Devuelve el número de elementos que tiene el árbol KDTree.
    return size_;
}

template <size_t N, typename ElemType>
bool KDTree<N, ElemType>::empty() const {   //Indica si el árbol KDTree esta vacio o no.
    if (size_ == 0) {
        return 1;
    }
    return 0;
}

template <size_t N, typename ElemType>
bool KDTree<N, ElemType>::contains(const Point<N>& pt) const { //Indica si el punto especificado está contenido en el KDTree.
    
    KDTreeNode<N, ElemType>** p;
    if (find(pt, p)) {
        return 1;
    }
    return 0;
}

template <size_t N, typename ElemType>
void KDTree<N, ElemType>::insert(const Point<N>& pt, const ElemType& value) { //Funcion para insertar elementos en el KDTree. 

    KDTreeNode<N, ElemType>** p;
    
    if (find(pt, p)) {  //Si el punto ya está en el árbol, entonces se actualiza su valor.
        (*p)->value = value;    
    }
    else {  //De lo contraro se crea un nuevo nodo.
        (*p) = new KDTreeNode<N, ElemType>(pt,value);
        size_++;
    }
    

}

template <size_t N, typename ElemType>
ElemType& KDTree<N, ElemType>::operator[](const Point<N>& pt) { // Devuelve una referencia al valor asociado con el punto pt en el KDTree.
    KDTreeNode<N, ElemType>** p;
    if (find(pt, p)) {
        return (*p)->value;
    }
    else {  // Si el punto no existe, se agrega al KDTree usando el valor predeterminado de ElemType como su clave.
        size_++;
        (*p) = new KDTreeNode<N, ElemType>(pt);
        return (*p)->value;
       }
}

template <size_t N, typename ElemType>
ElemType& KDTree<N, ElemType>::at(const Point<N>& pt) { //Devuelve una referencia a la clave asociada al punto pt. 
    KDTreeNode<N, ElemType>**p;
    if (find(pt, p)) {
        return (*p)->value;
    }
    else {
        throw std::out_of_range("Fuera de rango");  // Si el punto no está en el árbol, esta función genera una excepción fuera de rango.
    }
}

template <size_t N, typename ElemType>
const ElemType& KDTree<N, ElemType>::at(const Point<N>& pt) const {
    KDTreeNode<N,ElemType>** p;
    if (find(pt, p)) {
        return (*p)->value;
    }
    else {
        throw std::out_of_range("Fuera de rango");
    }
}

template <size_t N, typename ElemType>
ElemType KDTree<N, ElemType>::knn_value(const Point<N>& key, size_t k) const {
    
    
   // if (empty()) return ElemType();

   // if (k > size_) {
   //     k = size_;
   // }

   // ElemType new_element;
   // 
   // vector<pair<double, ElemType>> distancia_elem_type; // distancia y el tipo de dato
   // map<ElemType, int> mapita;  //tipo de dato y las veces que se repite ese dato

   // KDTreeNode<N, ElemType>* tmp = root;
   // stack<KDTreeNode<N, ElemType>*> stack_;

   // while (tmp || !stack_.empty()) {

   //     while (tmp) {
   //         distancia_elem_type.push_back(make_pair(distance(tmp->point, key), tmp->value));
   //         stack_.push(tmp);
   //         tmp = tmp->nodes[0];
   //     }
   //     tmp = stack_.top();
   //     stack_.pop();
   //     tmp = tmp->nodes[1];

   // }
   // /*cout << endl;
   // cout << "DISTANCIAS SIN ORDENAR: " << endl;
   // for (auto i : distancia_elem_type) {
   //     cout << i.first << " -> " << i.second << endl;
   // }*/

   // sort(distancia_elem_type.begin(), distancia_elem_type.end());

   // /*cout << "DISTANCIAS ORDENADAS: " << endl;
   // for (auto i : distancia_elem_type) {
   //     cout << i.first << " -> " << i.second << endl;
   // }*/

   // for (int i = 0; i < k; i++) {

   //     mapita[distancia_elem_type[i].second]++;
   // }
   ///* cout << "MAPITA" << endl;
   // for (auto i : mapita) {
   //     cout << i.first << " -> " << i.second << endl;
   // }
   // cout << endl;*/

   // //new_element = sort_(mapita);
   // int con = -1;
   // 
   // for (auto i : mapita) {
   //     if (i.second > con) {
   //         new_element = i.first;
   //         con = i.second;
   //     }
   // }

   // return new_element;


    BoundedPQueue<ElemType>nearest_neighbors_candidates(k); 
    if (empty()) return ElemType(); 

    int depth = 0;
    //double inf = std::numeric_limits<double>::infinity();
    find_neighbors(root,nearest_neighbors_candidates,depth,key);
    ElemType new_element;
    unordered_map<ElemType, int> contador;
   
    
    while (!nearest_neighbors_candidates.empty()) {
        ElemType aux=nearest_neighbors_candidates.front();
        nearest_neighbors_candidates.pop();
        contador[aux]++;
    }
      
    int con = -1;
    for (auto i : contador) {
        if (i.second > con) {
            new_element = i.first;
            con = i.second;
        }
    }
    return new_element;
}

template <size_t N, typename ElemType>
std::vector<ElemType> KDTree<N, ElemType>::knn_query(const Point<N>& key,size_t k) const {
    
    vector<ElemType> values;

    BoundedPQueue<ElemType>nearest_neighbors_candidates(k);
    if (empty()) return ElemType();

    int depth = 0;
    find_neighbors(root, nearest_neighbors_candidates, depth, key);

    while (!nearest_neighbors_candidates.empty()) {
        ElemType aux = nearest_neighbors_candidates.front();
        nearest_neighbors_candidates.pop();
        values.push_back(aux);
    }
    return values;
}


template <size_t N, typename ElemType>
bool KDTree<N, ElemType>::find(const Point<N>& pt, KDTreeNode<N, ElemType>**& it) const { //Funcion para buscar si existe un punto y donde se debe insertar.
    it = &root;
    int axis = 0;

    while ((*it) && (*it)->point != pt) {
        (pt[axis % dimension_] < (*it)->point[axis % dimension_]) ? it = &((*it)->nodes[0]) : it = &((*it)->nodes[1]);
        axis++;
    }
    return *it != 0;
}

template <size_t N, typename ElemType>
void KDTree<N, ElemType>::print(KDTreeNode<N, ElemType>* tmp, int conta)    //Imprimir los valores del árbol KDTree.
{
    if (!tmp) {
        return;
    }

    print(tmp->nodes[1], conta + 4);
    for (int i = 0; i < conta; i++) {
        cout << " ";
    }
    cout << tmp->value << endl;
    print(tmp->nodes[0], conta + 4);
}



template <size_t N, typename ElemType>
void KDTree<N, ElemType>::DestructorRecursivo(KDTreeNode<N, ElemType>* nodo) {  //Destructor recursivo del árbol KDTree.
    if (nodo == 0) return;
    
    DestructorRecursivo(nodo->nodes[0]);
    //cout<<"ELIMINANDO EL: " << nodo->value << endl;
    DestructorRecursivo(nodo->nodes[1]);
    
    delete nodo;
}

template <size_t N, typename ElemType>
void KDTree<N, ElemType>::DestructorIterativo(KDTreeNode<N, ElemType>* nodo) {  //Destructor recursivo del árbol KDTree.
    if (nodo == NULL) {
        return;
    }
    //cout << "Destructor invocado" << endl;
    queue<KDTreeNode<N, ElemType>*> cola;
    KDTreeNode<N, ElemType>* temp;
    cola.push(nodo);

    while (!cola.empty()) {

        temp = cola.front();
        cola.pop();

        if (temp->nodes[0]) {
            cola.push(temp->nodes[0]);
        }

        if (temp->nodes[1]) {
            cola.push(temp->nodes[1]);
        }
        //cout << temp->value << "  es  borrado" << endl;
        delete temp;
    }
    nodo = NULL;
}

template <std::size_t N, typename ElemType>
KDTreeNode<N, ElemType>* KDTree<N, ElemType>::CopiarArbol(KDTreeNode<N, ElemType>* current_node) {
    if (current_node == 0) return 0;
    KDTreeNode<N, ElemType>* root_copy = new KDTreeNode<N, ElemType>(*current_node);
    root_copy->nodes[0] = CopiarArbol(current_node->nodes[0]);
    root_copy->nodes[1] = CopiarArbol(current_node->nodes[1]);
    return root_copy;
}

template <size_t N, typename ElemType>
void KDTree<N, ElemType>::find_neighbors(KDTreeNode<N, ElemType>* current_node, BoundedPQueue<ElemType>& nearest_neighbors_candidates, int depth, const Point<N> key) const {

    if (!current_node) {
        return;
    }

    nearest_neighbors_candidates.push(distance(current_node->point, key),current_node->value);

    int axis = depth % dimension_;
    bool left;

    if (key[axis] < current_node->point[axis]) {
        left = true;
        find_neighbors(current_node->nodes[0], nearest_neighbors_candidates,++depth,key);
    }
    else {
        left = false;
        find_neighbors(current_node->nodes[1], nearest_neighbors_candidates,++depth,key);
        
    }
    if (nearest_neighbors_candidates.size() <nearest_neighbors_candidates.max_size() || fabs(key[axis] - current_node->point[axis]) < nearest_neighbors_candidates.mejor_distancia()) {
        if (left) {
            find_neighbors(current_node->nodes[1], nearest_neighbors_candidates, ++depth, key);
        }
        else {
            find_neighbors(current_node->nodes[0], nearest_neighbors_candidates, ++depth, key);
        }
    }  

}

#endif  // SRC_KDTREE_HPP_

