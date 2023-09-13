//#include "AVLFile.h"
#include "SequentialFile.h"

int main(){
    VariableRecordFile file1("data.bin");

    Record Matricula1{"201910111",1,100,"Ninguna"};
    Record Matricula2{"201910112",2,200,"Hubo descuento"};
    Record Matricula3{"20191011",3,300,"Pagado una parte"};
    Record Matricula4{"201910114",4,400,"Pagado"};
    Record Matricula5{"201910115",5,500,"Falta pago"};

    file1.add(Matricula1);
    file1.add(Matricula2);
    file1.add(Matricula3);
    file1.add(Matricula4);
    file1.add(Matricula5);
////
//    Record matricula = file1.readRecord(1);
//    matricula.showData();
//    cout<<endl;

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
