//
// Created by isaac on 7/09/2023.
//

#ifndef PROYECTO_1_SEQUENTIALFILE_H
#define PROYECTO_1_SEQUENTIALFILE_H

#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

struct Record{
    string Codigo;
    int Ciclo=0;
    float Mensualidad=0;
    string Observaciones;

    static size_t string_with_delimiter_size(const string& str){
        return sizeof(size_t)+str.size();
    }

    size_t size_of(){
        return string_with_delimiter_size(Codigo) +
               sizeof(Ciclo) +
               sizeof(Mensualidad) +
               string_with_delimiter_size(Observaciones);
    }

    static void concat(char*& buffer, const char* str) {
        size_t len = strlen(str);
        memcpy(buffer, &len, sizeof(len));
        buffer += sizeof(len);
        memcpy(buffer, str, len);
        buffer += len;
    }

    static void concat(char*& buffer, const float& value) {
        memcpy(buffer, &value, sizeof(value));
        buffer += sizeof(value);
    }

    static void concat(char*& buffer, const int& value) {
        memcpy(buffer, &value, sizeof(value));
        buffer += sizeof(value);
    }

    char* empaquetar() {
        size_t bufferSize = size_of();
        char* buffer = new char[bufferSize];
        char* current = buffer;

        concat(current, Codigo.c_str());
        concat(current, Ciclo);
        concat(current, Mensualidad);
        concat(current, Observaciones.c_str());

        return buffer;
    }

    void desempaquetar(const char* buffer, int n) {
        const char* current = buffer;

        size_t codigoLength;
        memcpy(&codigoLength, current, sizeof(codigoLength));
        current += sizeof(codigoLength);

        Codigo.assign(current, codigoLength);
        current += codigoLength;

        memcpy(&Ciclo, current, sizeof(Ciclo));
        current += sizeof(Ciclo);

        memcpy(&Mensualidad, current, sizeof(Mensualidad));
        current += sizeof(Mensualidad);

        size_t observacionesLength;
        memcpy(&observacionesLength, current, sizeof(observacionesLength));
        current += sizeof(observacionesLength);

        Observaciones.assign(current, observacionesLength);
    }

    void showData(){
        cout<<"Codigo: "<<Codigo<<endl;
        cout<<"Ciclo: "<<Ciclo<<endl;
        cout<<"Mensualidad: "<<Mensualidad<<endl;
        cout<<"Observciones: "<<Observaciones<<endl;
    }
};

class VariableRecordFile{
private:
    string filename;
public:
    explicit VariableRecordFile(string filename) : filename(std::move(filename)) {}

    void add(Record matricula) {
        ofstream file(filename, ios::app | ios::binary );//abro el archivo en app para que el puntero se posicione al final del archivo
        if(!file.is_open()) throw ("No se pudo abrir el archivo");
        file.seekp(0,ios::end);
        long pos_fisica = file.tellp();//obtengo la posición física del archivo

        auto buffer_temp = matricula.empaquetar();
        file.write(buffer_temp, matricula.size_of());//escribimos el alumno en el archivo
        delete [] buffer_temp;//liberamos la memoria del buffer
        file.close();//cerramos el archivo

        //LLENANDO METADATA:
        ofstream fm("cabecera.bin",ios::app | ios::binary );//abro el archivo, que será nuestra metadata, en donde almacenaré la posición y el tamaño de cada registro.
        size_t tam_Reg = matricula.size_of();//obtenemos el tamaño del registro que acabamos de escribir.
//        cout<<pos_fisica<<" "<<tam_Reg<<endl;
        fm.write(reinterpret_cast<char*>(&pos_fisica), sizeof(long));//escribimos la posición física del registro que acabamos de escribir.
        fm.write(reinterpret_cast<char*>(&tam_Reg), sizeof(int));//escribimos el tamaño del registro que acabamos de escribir.
        fm.close();//cerramos el archivo
    }

    vector<Record> load(){
        vector<Record> alumnos;
        ifstream fm("cabecera.bin", ios::binary);

        //Calcular la cantidad de registros con el archivo de cabecera;
        fm.seekg(0,ios::end);//Nos movemos al final del archivo
        int cant_reg = fm.tellg()/(sizeof(int)*2);//Calculamos la cantidad de registros

        for(int i=0;i<cant_reg;i++){
            Record alumno = readRecord(i);
            alumnos.push_back(alumno);
        }

        fm.close();
        return alumnos;
    }

    Record readRecord(int pos){
        //Leyendo metadata
        ifstream fm("cabecera.bin", ios::binary);//abro el archivo, que será nuestra metadata, en donde almacenaré la posición y el tamaño de cada registro.
        if(!fm.is_open()) throw ("No se pudo abrir el archivo");
        fm.seekg(pos*sizeof(int)*2);//Cada registro tiene 2 enteros, pos y tamaño. Nos movemos a la posición del registro que queremos leer. NOLINT(*-narrowing-conversions)

        //Obtenemos las características del record[pos]
        long pos_fisica;
        int tam_reg;
        fm.read(reinterpret_cast<char*>(&pos_fisica), sizeof(long));//Leemos la posición física del registro que queremos leer.
        fm.read(reinterpret_cast<char*>(&tam_reg), sizeof(int));//Leemos el tamaño del registro que queremos leer.
//        cout<<pos_fisica<<" "<<tam_reg<<endl;
        fm.close();//Es todo lo que quería de la metadata

        //Procedemos a trabajar con el archivo de registros
        ifstream file(filename, ios::binary);//abro el archivo
        if(!file.is_open()) throw ("No se pudo abrir el archivo");
        file.seekg(pos_fisica);//Nos movemos a la posición del registro que queremos leer.
//        cout<<file.tellg()<<endl;
        //Ubicado el cursor en el registro pos:
        char* buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
        file.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.
        file.close();//Cerramos el archivo

        Record record;
        record.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
        delete [] buffer;//Liberamos la memoria del buffer
        return record;
    }

};

int main(){
    VariableRecordFile file1("data.bin");

    Record Matricula1{"201910111",1,100,"Ninguna"};
    Record Matricula2{"201910112",2,200,"Hubo descuento"};
    Record Matricula3{"20191011",3,300,"Pagado una parte"};
    Record Matricula4{"201910114",4,400,"Pagado"};
    Record Matricula5{"201910115",5,500,"Falta pago"};

    file1.add(Matricula1);
//    file1.add(Matricula2);
//    file1.add(Matricula3);
//    file1.add(Matricula4);
//    file1.add(Matricula5);
//
    Record matricula = file1.readRecord(2);
    matricula.showData();
    cout<<endl;

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


#endif //PROYECTO_1_SEQUENTIALFILE_H
