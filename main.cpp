//#include "AVLFile.h"
#include "SequentialFile.h"

int main(){
    SequentialFile file1("data.bin");

    Record Matricula1{"201910110",1,100,"Ninguna"};
    Record Matricula2{"201910112",2,200,"Hubo descuento"};
    Record Matricula3{"201910119",3,300,"Pagado una parte"};
    Record Matricula4{"201910113",4,400,"Pagado"};
    Record Matricula5{"201910114",5,500,"Falta pago"};

    file1.add(Matricula4);//201910113
    file1.add(Matricula2);//201910112
    file1.add(Matricula5);//201910114
    file1.add(Matricula3);
    // file1.add(Matricula4);
    // file1.add(Matricula1);
    // file1.add(Matricula3);

    // file1.add(Matricula1);
    // file1.add(Matricula5);
    // file1.add(Matricula4);
    // file1.add(Matricula2);
    // file1.add(Matricula3);
//
   fstream aux("aux.bin", ios::binary | ios::in | ios::out | ios::app);
   aux.seekg(0, ios::end);
   cout<<"main_aux_n: "<<aux.tellg()<<endl;
   aux.close();

//
//    aux.read((char*)&temp, sizeof(size_t));
//    cout<<aux.tellg()<<endl;
//    cout<<temp<<endl;
//    aux.close();

//    Record matricula = file1.readRecord(1);
//    matricula.showData();
    fstream cabecera("data.bin", ios::binary | ios::in | ios::out | ios::app);
    cabecera.seekg(4, ios::beg);
    size_t temp;
    cabecera.read((char*)&temp, sizeof(size_t));
    
    cout<<endl<<endl;
    cout<<"Impresion del archivo aux.bin: "<<endl;
    cout<<"--------------------------------"<<endl<<endl;
    cout<<"Puntero de cabecera: "<<temp<<endl<<endl;
    file1.Imprimir_aux_bin();

//    vector<Record> v_matricula = file1.load();
//    for(auto & i : v_matricula) {
//        i.showData();
//        cout<<endl;
//    }

// NO HACER CASO A ESTO, codigos de prueba unitarias
//    Record Copia_datos;
//    char* buffer = Record3.empaquetar();
//
//
//    // Mostrar los datos desempaquetados
//    std::cout << "Codigo: " << Copia_datos.Codigo << std::endl;
//    std::cout << "Ciclo: " << Copia_datos.Ciclo << std::endl;
//    std::cout << "Mensualidad: " << Copia_datos.Mensualidad << std::endl;
//    std::cout << "Observaciones: " << Copia_datos.Observaciones << std::endl;


//    delete[] buffer; // Liberar la memoria del buffer

//    ifstream file("cabecera.bin", ios::binary);
//    char c;
//    while (file.get(c)) {
//        cout << c;
//    }
//    file.close();
    return 0;
}
