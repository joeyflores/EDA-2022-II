#include <iostream>
#include <vector>
#include <iterator>
#include <random>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <algorithm>

using namespace std;


namespace Sequential {

	template <typename ElemType>
	struct SkipListNode
	{
		int level;	//Maximo nivel que tendra nuestro nodo 
		ElemType value;
		vector<SkipListNode<ElemType>*> next;

		SkipListNode(int level_, ElemType value_ = ElemType()) {
			value = value_;
			next.resize(level_ + 1);
			for (int i = 0; i < next.size(); i++) {
				next[i] = nullptr;
			}
			level = level_;
		}
	};


	template <typename ElemType>
	class SkipList {

	private:
		int max_level;	//Maximo nivel que tiene la SkipList
		int current_level;		//Nivel actual que tiene la SkipList
		int size_;		//Cantidad de elementos que tiene la SkipList
		SkipListNode<ElemType>* head;

	public:
		SkipList(int max_level_);
		int numero_aleatorio();
		int random_level();
		void size_level();
		int size();
		SkipListNode<ElemType>* FindNode(ElemType value, vector<SkipListNode<ElemType>*>& predecessors);
		bool insert(ElemType value);
		bool remove(ElemType value);
		bool search(ElemType value);
		bool optimized_search(ElemType value);	// search optimizado
		void print();

	};


	template <typename ElemType>
	SkipList<ElemType>::SkipList(int max_level_) {	//Constructor de la clase SkipList
		max_level = max_level_;
		current_level = 0;
		size_ = 0;
		head = new SkipListNode<ElemType>(max_level);
	}

	template <typename ElemType>
	int SkipList<ElemType>::numero_aleatorio() {	//Genera un numero aleatorio, 0 o 1
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 1);
		return dis(gen);
	}

	template <typename ElemType>
	int SkipList<ElemType>::random_level() {	//Calcular la cantidad de niveles que tendra el nodo a insertar

		int level = 0;
		while (level < max_level) {
			int aux = numero_aleatorio();
			if (aux == 1) {	//Si el numero aleatorio es 1 el nivel del nodo incrementa en 1
				level++;
			}
			else {	//Si el numero aleatorio es 0 entonces se detiene y retorna el nivel actual que tendra nuestro nodo a insertar
				break;
			}
		}
		return level;
	}

	template <typename ElemType>
	void SkipList<ElemType>::size_level() {	//Cantidad de elementos por nivel que tiene la SkipList


		SkipListNode<ElemType>* p = head;
		for (int i = max_level; i >= 0; i--) {
			int size = 0;
			cout << "Cantidad de elementos en el nivel " << i << ": ";
			SkipListNode<ElemType>* q = p->next[i];
			while (q) {
				size++;
				q = q->next[i];
			}
			cout << size << endl;
			cout << endl;
		}
	}
	template <typename ElemType>
	int SkipList<ElemType>::size() {	//Retorna la cantidad de elementos que tiene la SkipList
		return size_;
	}

	template <typename ElemType>
	SkipListNode<ElemType>* SkipList<ElemType>::FindNode(ElemType value, vector<SkipListNode<ElemType>*>& predecessors) {	//Guarda los antecesores para poder insertar o eliminar

		predecessors.resize(max_level + 1);

		for (int i = 0; i < predecessors.size(); i++) {
			predecessors[i] = nullptr;
		}
		SkipListNode<ElemType>* p = head;

		for (int i = current_level; i >= 0; i--) {	//Busca desde el nivel actual de la SkipList para guardar los antecesores
			SkipListNode<ElemType>* current_node = p->next[i];
			while (current_node && current_node->value < value) {
				p = current_node;
				current_node = p->next[i];
			}
			predecessors[i] = p;
		}
		p = p->next[0];

		return p;
	}

	template <typename ElemType>
	bool SkipList<ElemType>::insert(ElemType value) {


		vector<SkipListNode<ElemType>*> predecessors;
		SkipListNode<ElemType>* p = FindNode(value, predecessors);

		if (p && p->value == value) {	//Si el elemento que deseamos insertar ya fue insertado anteriormente, entonces no se inserta de nuevo y retorna FALSE (No acepta Repetidos)
			//cout << "No se inserto el numero: " << p->value << endl;
			return 0;
		}

		int new_level = random_level();	//Nivel que tendra el nodo a insertar

		//Si el nuevo nivel que tendra el nodo a insertar es mayor que el nivel actual de la SkipList,entonces se hace que el antecesor[i] apunte al head

		if (new_level > current_level) {
			for (int i = current_level + 1; i < new_level + 1; i++) {
				predecessors[i] = head;
			}
			current_level = new_level;	//Se actualiza el nuevo nivel
		}

		SkipListNode<ElemType>* new_node = new SkipListNode<ElemType>(new_level, value);
		for (int i = 0; i <= new_level; i++) {	//Se modifican los punteros para insertar correctamente 
			new_node->next[i] = predecessors[i]->next[i];
			predecessors[i]->next[i] = new_node;
		}
		size_++;

		return 1;
	}

	template <typename ElemType>
	bool SkipList<ElemType>::remove(ElemType value) {

		vector<SkipListNode<ElemType>*> predecessors;


		SkipListNode<ElemType>* p = FindNode(value, predecessors);

		if (!(p && p->value == value)) return 0;	//Si el elemento que deseamos eliminar no existe entonces acaba la funcion  y retorna FALSE


		for (int i = 0; i <= current_level; i++) {	//Se empieza desde el primer nivel y se actualiza los punteros para poder eliminar el nodo

			if (predecessors[i]->next[i] != p) {
				break;
			}
			predecessors[i]->next[i] = p->next[i];
		}

		delete p;

		//Se elimina los niveles que no tengan elementos
		while (current_level > 0 && head->next[current_level] == nullptr) {
			current_level--;
		}
		size_--;
		return 1;
	}

	template <typename ElemType>
	bool SkipList<ElemType>::search(ElemType value) {	//Buscar un elemento en la SkipList

		vector<SkipListNode<ElemType>*> predecessors;
		SkipListNode<ElemType>* p = FindNode(value, predecessors);
		if (p && p->value == value) {	//Si encuentra el elemento retorna TRUE
			return true;
		}
		return false;	//Si no encuentra el elemento retorna FALSE
	}

	template <typename ElemType>
	bool SkipList<ElemType>::optimized_search(ElemType value) {	//Buscar un elemento en la SkipList version 2.0

		SkipListNode<ElemType>* p = head;

		for (int i = current_level; i >= 0; i--) {
			SkipListNode<ElemType>* current_node = p->next[i];
			while (current_node && current_node->value < value) {
				p = current_node;
				current_node = p->next[i];
			}
			if (current_node && current_node->value == value) {
				return true;
			}

		}
		return false;
	}

	template <typename ElemType>
	void SkipList<ElemType>::print() {	//Mostrar la SkipList por niveles 

		SkipListNode<ElemType>* p = head;
		//cout << "HEAD: " << p->value;
		cout << "-----------------------------------------------" << endl << endl;
		for (int i = max_level; i >= 0; i--) {
			cout << "Nivel " << i << ": ";
			SkipListNode<ElemType>* q = p->next[i];
			while (q) {
				cout << q->value << " ";
				q = q->next[i];
			}
			cout << "-> NULL ";
			cout << endl << endl;
		}
		cout << "-----------------------------------------------" << endl;
	}
}	// FIN NAMESPACE SEQUENTIAL
