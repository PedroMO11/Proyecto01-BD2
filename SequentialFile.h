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
    size_t punt_nextPosFisica = -1; //4 bits
    bool punt_next_is_In_Data = false; // 1bits

    static size_t string_with_delimiter_size(const string& str){
        return sizeof(size_t)+str.size();
    }

    size_t size_of(){
        return sizeof(size_t)+ //tamaño de la longitud de la cadena
                string_with_delimiter_size(Codigo) +
               sizeof(Ciclo) +
               sizeof(Mensualidad) +
               string_with_delimiter_size(Observaciones) +
               sizeof(punt_nextPosFisica) +
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

        concat(current, bufferSize);
        concat_string(current, Codigo.c_str());
        concat(current, Ciclo);
        concat(current, Mensualidad);
        concat_string(current, Observaciones.c_str());
        concat(current, punt_nextPosFisica);
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
        current += sizeof(size_t);
        assignFromBuffer(current, Codigo);
        copyFromBuffer(current, Ciclo);
        copyFromBuffer(current, Mensualidad);
        assignFromBuffer(current, Observaciones);
        copyFromBuffer(current, punt_nextPosFisica);
        copyFromBuffer(current, punt_next_is_In_Data);
    }

    void showData() const{
        cout<<"Codigo: "<<Codigo<<endl;
        cout<<"Ciclo: "<<Ciclo<<endl;
        cout<<"Mensualidad: "<<Mensualidad<<endl;
        cout<<"Observaciones: "<<Observaciones<<endl;
        if(punt_nextPosFisica == -1) cout<<"posicion fisica de next: "<<(int)punt_nextPosFisica<<endl;
        else cout<<"posicion fisica de next: "<<punt_nextPosFisica<<endl;
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
class SequentialFile {
private:
    string filename;
public:
    explicit SequentialFile(string filename) : filename(std::move(filename)) {}

    bool add(Record record) {
        fstream file(filename, std::ios::binary | ios::out | ios::in | ios::app);
        if (!file.is_open()) {cerr << "No se pudo abrir el archivo "+filename<<endl; return false;}

        fstream aux("aux.bin", std::ios::binary | ios::out | ios::in | std::ios::app);//Me asegura que escribo al final de la data
        if (!aux.is_open()) {cerr << "No se pudo abrir el archivo aux.bin"<<endl; return false;}

        file.seekp(0, ios::end);//Nos movemos al final del archivo, para comprobar si está escrito la metadata
        auto pos_fisica = file.tellp();//obtengo la posición física del archivo
        //Información de la metadata
        size_t n_data = 0;
        size_t n_aux = 0;
        size_t metadata_nextPosFisica;
        bool metadata_next_is_In_Data;
        int indice = 0;

        if (pos_fisica == 0) {//No hay nada escrito en data
            //Escribimos la metadata
            metadata_next_is_In_Data = false;
            metadata_nextPosFisica = sizeof(n_aux);//Apunta al primer record que será de aux.bin//Solo hay metadata n_aux
            file.write(reinterpret_cast<char *>(&n_data), sizeof(n_data));
            file.write(reinterpret_cast<char *>(&metadata_nextPosFisica), sizeof(metadata_nextPosFisica));
            file.write(reinterpret_cast<char *>(&metadata_next_is_In_Data), sizeof(bool));

            //Escrimos la data en aux.bin
            n_aux = 1;

            aux.write(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
            auto buffer_temp = record.empaquetar();
            aux.write(buffer_temp, record.size_of());//escribimos el record en el archivo
            delete[] buffer_temp;//liberamos la memoria del buffer
            aux.seekg(0, ios::end);
            file.seekg(0, ios::end);

        }else if(pos_fisica == sizeof(size_t)*2 + sizeof(bool)){//Está escrito solo la metadata
            fstream file_out(filename, std::ios::binary | ios::out | ios::in );//abro el archivo sin append
            file_out.seekp(0, ios::beg);//Nos movemos al inicio del archivo
            file_out.read(reinterpret_cast<char *>(&n_data), sizeof(n_data));

            if(n_data == 0){
                Record temp;
                size_t temp_tamReg;

                //Obtengo el record de aux.bin
                aux.seekg(sizeof(size_t), ios::beg);//Me coloco después de la metadata en aux
                aux.read(reinterpret_cast<char *>(&temp_tamReg), sizeof(temp_tamReg));
                aux.seekg(sizeof(size_t), ios::beg);

                char *buffer = new char[temp_tamReg];//Creamos un buffer del tamaño del registro que vamos a leer.
                aux.read(buffer, temp_tamReg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.
                temp.desempaquetar(buffer, temp_tamReg);//Desempaquetamos el buffer, asignamos los valores al record.
                delete[] buffer;//Liberamos la memoria del buffer

                ofstream aux_out("aux.bin", ios::binary | ios::out | ios::in);//Me asegura que escribo al final de la data
                aux_out.seekp(0, ios::beg);
                n_aux = 0;
                aux_out.write(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
                for(int i=0;i<temp_tamReg;i++) aux_out.put(0);//Borramos el registro de aux.bin
                aux_out.close();

//                aux.seekg(0, ios::end);
//                file.seekg(0, ios::end);
//                cout<<file.tellg()<<" "<<aux.tellg(); //todo bien hasta aqui
                if(record.Codigo < temp.Codigo) {
                    metadata_nextPosFisica = sizeof(n_data)+sizeof(metadata_nextPosFisica)+sizeof(metadata_next_is_In_Data);
                    metadata_next_is_In_Data = true;
                    file_out.write(reinterpret_cast<char *>(&metadata_nextPosFisica), sizeof(metadata_nextPosFisica));
                    file_out.write(reinterpret_cast<char *>(&metadata_next_is_In_Data), sizeof(bool));

                    //Escribimos el record en data.bin
                    record.punt_nextPosFisica=((size_t)file_out.tellg() + record.size_of());
                    record.punt_next_is_In_Data = true;
                    auto buffer_record = record.empaquetar();
                    file_out.write(buffer_record, record.size_of());//escribimos el record en el archivo
                    delete[] buffer_record;//liberamos la memoria del buffer
                    aux.seekg(0, ios::end);
                    file_out.seekg(0, ios::end);
                    aux.seekg(0, ios::end);
                    file_out.seekg(0, ios::end);

                    //Escribimos el temp en data.bin
                    auto buffer_temp = temp.empaquetar();
                    file_out.write(buffer_temp, temp.size_of());//escribimos el record en el archivo
                    delete[] buffer_temp;//liberamos la memoria del buffer
                    file_out.seekg(0,ios::end);
                }
            }
            file_out.close();
        }
        aux.close();//cerramos el archivo
        file.close();
        return true;
    }

//    vector<Record> load(){
//        vector<Record> alumnos;
//        ifstream fm("cabecera.bin", ios::binary);
//
//        //Calcular la cantidad de registros con el archivo de cabecera;
//        fm.seekg(0,ios::end);//Nos movemos al final del archivo
//        int cant_reg = fm.tellg()/(sizeof(int)*2);//Calculamos la cantidad de registros
//
//        for(int i=0;i<cant_reg;i++){
//            Record alumno = readRecord(i);
//            alumnos.push_back(alumno);
//        }
//
//        fm.close();
//        return alumnos;
//    }

//    Record readRecord(int pos){
//        //Leyendo metadata
//        ifstream fm("cabecera.bin", ios::binary);//abro el archivo, que será nuestra metadata, en donde almacenaré la posición y el tamaño de cada registro.
//        if(!fm.is_open()) throw ("No se pudo abrir el archivo");
//        fm.seekg(pos*sizeof(int)*2);//Cada registro tiene 2 enteros, pos y tamaño. Nos movemos a la posición del registro que queremos leer. NOLINT(*-narrowing-conversions)
//
//        //Obtenemos las características del record[pos]
//        long pos_fisica;
//        int tam_reg;
//        fm.read(reinterpret_cast<char*>(&pos_fisica), sizeof(long));//Leemos la posición física del registro que queremos leer.
//        fm.read(reinterpret_cast<char*>(&tam_reg), sizeof(int));//Leemos el tamaño del registro que queremos leer.
////        cout<<pos_fisica<<" "<<tam_reg<<endl;
//        fm.close();//Es todo lo que quería de la metadata
//
//        //Procedemos a trabajar con el archivo de registros
//        ifstream file(filename, ios::binary);//abro el archivo
//        if(!file.is_open()) throw ("No se pudo abrir el archivo");
//        file.seekg(pos_fisica);//Nos movemos a la posición del registro que queremos leer.
////        cout<<file.tellg()<<endl;
//        //Ubicado el cursor en el registro pos:
//        char* buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
//        file.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.
//        file.close();//Cerramos el archivo
//
//        Record record;
//        record.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
//        delete [] buffer;//Liberamos la memoria del buffer
//        return record;
//    }

    void show_all_data(){
        ifstream fm(filename, ios::binary);
        ifstream aux("aux.bin", ios::binary);
        if(!fm.is_open()) throw ("No se pudo abrir el archivo");
        size_t temp_punt_next=0;
        bool temp_punt_is_in_data;
        size_t tam_reg;
        size_t n_aux;
        size_t n_data;
        Record temp;

        fm.read(reinterpret_cast<char*>(&n_data), sizeof(n_data));
        fm.read(reinterpret_cast<char*>(&temp_punt_next), sizeof(temp_punt_next));
        fm.read(reinterpret_cast<char*>(&temp_punt_is_in_data), sizeof(temp_punt_is_in_data));
        aux.read(reinterpret_cast<char*>(&n_aux), sizeof(n_aux));

//        fm.seekg(0,ios::end);//Queremos la metadata posición fisica del siguiente registro
//        cout<<fm.tellg()<<endl;

        while(temp_punt_next != -1){
            if(temp_punt_is_in_data) {
                fm.seekg(temp_punt_next, ios::beg);//Queremos la metadata posición fisica del siguiente registro
                fm.read(reinterpret_cast<char *>(&tam_reg), sizeof(tam_reg));//Leemos el tamaño del record
                fm.seekg(-((int)sizeof(tam_reg)),ios::cur);
                char *buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
                fm.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.

                //Actualizamos la metadata: flag_punt_next y flag_punt_is_in_data
                fm.seekg(-9,ios::cur);
                fm.read(reinterpret_cast<char*>(&temp_punt_next), sizeof(temp_punt_next));//Leemos la posición física del registro que queremos leer.
                fm.read(reinterpret_cast<char*>(&temp_punt_is_in_data), sizeof(temp_punt_is_in_data));

                temp.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
                delete[] buffer;//Liberamos la memoria del buffer
                temp.showData();
            }else{
                if(n_aux == 0){
                    cout<<"No hay registros en aux.bin"<<endl;
                    break;
                }
                aux.seekg(8, ios::beg);//Me coloco después d vcbxe la metadata
                aux.read(reinterpret_cast<char *>(&tam_reg), sizeof(tam_reg));//Leemos el tamaño del record

                char *buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
                aux.seekg(-((int)sizeof(tam_reg)),ios::cur);

                aux.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg
                temp.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
                //Actualizamos la metadata: flag_punt_next y flag_punt_is_in_data
                temp_punt_next = temp.punt_nextPosFisica;
                temp_punt_is_in_data = temp.punt_next_is_In_Data;

                delete[] buffer;//Liberamos la memoria del buffer
                temp.showData();
            }
            cout<<endl;
            cout<<"\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\"<<endl;
            cout<<endl;
        }
        aux.close();
        fm.close();
    }

};

#endif //PROYECTO_1_SEQUENTIALFILE_H
