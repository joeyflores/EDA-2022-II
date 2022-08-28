#include <iostream>
#include <random>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <map>
#include <fstream>
#include <sstream>

using namespace std;


//-------Declaracion de funciones para poder usarlas en la clase "Experimento"--------
template <class T>
T NumerosAleatorios();

template <class T>
void RellenarVectorPuntos(vector<vector<T>>& puntos, int cantidad_datos, int dimensiones);

template <class T,class E>
void DistanciaEuclidiana(vector<vector<T>> puntos, map<E, int>& counter, int cantidad_datos, int dimensiones);

template <class T>
void MostrarPuntos(vector<vector<T>> puntos);
//-------------------------------------------------------------------------------------


//----------------------------------CLASE EXPERIMENTO----------------------------------
class Experimento{
  private:
    string file_name;
    int cantidad_datos;
    int dimensiones;

  public:
    Experimento(string _file_name,int _cantidad_datos,int _dimensiones);   
    void conduct();

};

Experimento::Experimento(string _file_name,int _cantidad_datos,int _dimensiones){
    file_name=_file_name;
    cantidad_datos=_cantidad_datos;
    dimensiones=_dimensiones;
}

void Experimento::conduct(){
  
    vector<vector<int>> puntos;
    map<double, int> counter;
    RellenarVectorPuntos(puntos, cantidad_datos, dimensiones); //Funcion para rellenar los vectores con puntos generados aleatoriamente 
  
    //MostrarPuntos(puntos);  //Funcion para ver los puntos generados
    DistanciaEuclidiana(puntos, counter, cantidad_datos, dimensiones);
  
    ofstream archivo;
    ostringstream o;
    string s;
      
    o << "DISTANCIA   VECES QUE SE REPITE" << endl; // Se usa "map" para tener todas las distancias calculadas en forma ordenada y para calcular las veces que se repiten dichas distancias
    for (auto it : counter) {
        o << setw(5) << it.first << setw(8) << " -> " << setw(8) << it.second << endl;           
    }
  
    s=o.str();
    archivo.open(file_name,ios::out);
    archivo<<s<<endl;
    archivo.close();
}
//-------------------------------------------------------------------------------------


//--------------------------------FUNCIONES--------------------------------------------
template <class T>
T NumerosAleatorios() {  //Funcion para crear un numero aleatorio entre 1 a 1000
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 1000);
    return dis(gen);
}

template <class T>
void RellenarVectorPuntos(vector<vector<T>>& puntos, int cantidad_datos, int dimensiones) {   

    for (int i = 0; i < cantidad_datos; i++){
      vector<T> temp(dimensiones);
      generate(temp.begin(), temp.end(),NumerosAleatorios<T>);  //Funcion "generate" para generar numeros aleatorios
      puntos.push_back(temp);
    }
}

template <class T,class E>
void DistanciaEuclidiana(vector<vector<T>> puntos, map<E, int>& counter, int cantidad_datos, int dimensiones){
  
    cout << "DistanciaEuclidiana" << endl;
    E  suma, raiz;
  
    int i = 0;  //se selecciona el primer punto para compararlo con el resto de puntos
    for (int j = i + 1; j < cantidad_datos; j++){
      suma = 0;
      for (int k = 0; k < dimensiones; k++) {
          suma += pow(puntos[i][k] - puntos[j][k], 2);
      }
      
      raiz = sqrt(suma);
      E round=truncf(raiz*10)/10;  //Funcion para redondear el numero a un decimal  
      counter[round]++;
    }
}

template <class T>
void MostrarPuntos(vector<vector<T>> puntos) {  
  
    for(int i=0;i<puntos.size();i++){ 
      copy(puntos[i].begin(),puntos[i].end(),std::ostream_iterator<int>(std::cout, " , "));  //Funcion "copy"
      cout<<endl;
    }
}
//-------------------------------------------------------------------------------------


int main()
{
     
    vector<int> cantidad_datos={10000,20000,30000,40000,50000};
    vector<int> dimensiones={10,20,30,40,50};
    
    for(int i=0;i<cantidad_datos.size();i++){
      for(int j=0;j<dimensiones.size();j++){   
        string tmp="Prueba_"+to_string(cantidad_datos[i])+"datos_"+to_string(dimensiones[j])+"dimensiones.txt"; //Se usa para ponerle nombre a cada experimento en el archivo txt
        Experimento ex(tmp,cantidad_datos[i],dimensiones[j]);
        ex.conduct();
      }
    }
  
    return 0;
}
