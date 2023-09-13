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
    size_t size;
    string Codigo;
    int Ciclo=0;
    float Mensualidad=0;
    string Observaciones;
    int punt_nextPos = -1;
    bool punt_next_is_In_Data = false;

    Record() = default;

    Record (string Codigo, int Ciclo, float Mensualidad, string Observaciones){
        this->Codigo = std::move(Codigo);
        this->Ciclo = Ciclo;
        this->Mensualidad = Mensualidad;
        this->Observaciones = std::move(Observaciones);
        this->size = size_of();
    }

    static size_t string_with_delimiter_size(const string& str){
        return sizeof(size_t)+str.size();
    }

    size_t size_of(){
        return sizeof(size)+ //tamaño de la longitud de la cadena
                string_with_delimiter_size(Codigo) +
               sizeof(Ciclo) +
               sizeof(Mensualidad) +
               string_with_delimiter_size(Observaciones) +
               sizeof(punt_nextPos) +
               sizeof(punt_next_is_In_Data);
    }

    static void concat_string(char*& buffer, const char* str) {
        size_t len = strlen(str);
        memcpy(buffer, &len, sizeof(len));
        buffer += sizeof(len);
        memcpy(buffer, str, len);
        buffer += len;
    }
    template <typename T>
    static void concat(char*& buffer, const T& value) {
        memcpy(buffer, &value, sizeof(value));
        buffer += sizeof(value);
    }

    char* empaquetar() {
        size_t bufferSize = size_of();
        char* buffer = new char[bufferSize];
        char* current = buffer;

        concat(current, size_of());
        concat_string(current, Codigo.c_str());
        concat(current, Ciclo);
        concat(current, Mensualidad);
        concat_string(current, Observaciones.c_str());
        concat(current, punt_nextPos);
        concat(current, punt_next_is_In_Data);
        return buffer;
    }

    template <typename T>
    void copyFromBuffer(const char*& buffer, T& variable) {
        memcpy(&variable, buffer, sizeof(variable));
        buffer += sizeof(variable);
    }

    void assignFromBuffer(const char*& buffer, string& variable) {
        size_t length;
        copyFromBuffer(buffer, length);
        variable.assign(buffer, length);
        buffer += length;
    }

    void desempaquetar(const char* buffer, int n) {
        const char* current = buffer;
        copyFromBuffer(current, size);
        assignFromBuffer(current, Codigo);
        copyFromBuffer(current, Ciclo);
        copyFromBuffer(current, Mensualidad);
        assignFromBuffer(current, Observaciones);
        copyFromBuffer(current, punt_nextPos);
        copyFromBuffer(current, punt_next_is_In_Data);
    }

    void showData(){
        cout<<"Tamaño: "<<size<<endl;
        cout<<"Codigo: "<<Codigo<<endl;
        cout<<"Ciclo: "<<Ciclo<<endl;
        cout<<"Mensualidad: "<<Mensualidad<<endl;
        cout<<"Observaciones: "<<Observaciones<<endl;
        cout<<"Puntero a siguiente posicion: "<<punt_nextPos<<endl;
        cout<<"next esta en data?: "<<punt_next_is_In_Data<<endl;
    }
};
//class SequentialFile {
//    string filename;
//
//public:
//    explicit SequentialFile(string filename) {
//        this->filename = filename;
//        VariableRecordFile file(filename);
//    }
//
//
//
//
//};
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
        cout<<pos_fisica<<" "<<tam_Reg<<endl;
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

#endif //PROYECTO_1_SEQUENTIALFILE_H
