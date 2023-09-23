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

    void showData(){
        cout<<"Tamanio: "<<size_of()<<endl;
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
            cout<<"Fin: "<<file.tellg()<<" "<<aux.tellg()<<endl;

        }else if(pos_fisica == sizeof(size_t)*2 + sizeof(bool)){//Está escrito solo la metadata
            // fstream file_out(filename, std::ios::binary | ios::out | ios::in );//abro el archivo sin append
            file.seekp(0, ios::beg);//Nos movemos al inicio del archivo
            file.read(reinterpret_cast<char *>(&n_data), sizeof(n_data));

            if(n_data == 0){//En data.bin no hay datos
                //Debo insertarlo en aux.bin, pero debo insertar records hasta llegar a n_aux=2, para luego reinsertarlos ordenadamente en data.bin
                aux.seekp(0, ios::beg);//Nos movemos al inicio del archivo
                aux.read(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
                cout<<n_aux<<" "<<n_data<<endl;

                if(n_aux==2){
                    //función insertar en data de manera ordenada
                }

                insert_heap_record(record);
                

                
                ////////////////////////////////
                // Record temp;
                // size_t temp_tamReg;

                // //Obtengo el record de aux.bin
                // aux.seekg(sizeof(size_t), ios::beg);//Me coloco después de la metadata en aux
                // aux.read(reinterpret_cast<char *>(&temp_tamReg), sizeof(temp_tamReg));
                // aux.seekg(sizeof(size_t), ios::beg);

                // char *buffer = new char[temp_tamReg];//Creamos un buffer del tamaño del registro que vamos a leer.
                // aux.read(buffer, temp_tamReg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.
                // temp.desempaquetar(buffer, temp_tamReg);//Desempaquetamos el buffer, asignamos los valores al record.
                // delete[] buffer;//Liberamos la memoria del buffer

                // ofstream aux_out("aux.bin", ios::binary | ios::out | ios::in);//Me asegura que escribo al final de la data
                // aux_out.seekp(0, ios::beg);
                // n_aux = 0;
                // aux_out.write(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
                // for(int i=0;i<temp_tamReg;i++) aux_out.put(0);//Borramos el registro de aux.bin
                // aux_out.close();

//                aux.seekg(0, ios::end);
//                file.seekg(0, ios::end);
//                cout<<file.tellg()<<" "<<aux.tellg(); //todo bien hasta aqui
                // if(record.Codigo < temp.Codigo) {
                //     metadata_nextPosFisica = sizeof(n_data)+sizeof(metadata_nextPosFisica)+sizeof(metadata_next_is_In_Data);
                //     metadata_next_is_In_Data = true;
                //     file_out.write(reinterpret_cast<char *>(&metadata_nextPosFisica), sizeof(metadata_nextPosFisica));
                //     file_out.write(reinterpret_cast<char *>(&metadata_next_is_In_Data), sizeof(bool));

                //     //Escribimos el record en data.bin
                //     record.punt_nextPosFisica=((size_t)file_out.tellg() + record.size_of());
                //     cout<<file_out.tellg()<<" - "<<record.size_of()<<endl;
                //     record.punt_next_is_In_Data = true;
                //     auto buffer_record = record.empaquetar();
                //     file_out.write(buffer_record, record.size_of());//escribimos el record en el archivo
                //     delete[] buffer_record;//liberamos la memoria del buffer
                //     aux.seekg(0, ios::end);
                //     file_out.seekg(0, ios::end);
                //     aux.seekg(0, ios::end);
                //     file_out.seekg(0, ios::end);

                //     //Escribimos el temp en data.bin
                //     auto buffer_temp = temp.empaquetar();
                //     file_out.write(buffer_temp, temp.size_of());//escribimos el record en el archivo
                //     delete[] buffer_temp;//liberamos la memoria del buffer
                //     file_out.seekg(0,ios::end);
                // }
            }
            // file_out.close();
        }
        aux.close();//cerramos el archivo
        file.close();
        return true;
    }
    void insert_heap_record(Record& record){
        fstream aux_out("aux.bin", std::ios::binary | ios::out | ios::in );
        if (!aux_out.is_open()) {cerr << "No se pudo abrir el archivo aux.dat"<<endl;}
        fstream file_out(filename, std::ios::binary | ios::out | ios::in );
        if (!file_out.is_open()) {cerr << "No se pudo abrir el archivo "+filename<<endl;}
        
        size_t temp_tamReg;
        size_t flag_punt_nextPosFisica;
        bool flag_punt_is_in_data;
        bool always_false=0;

        aux_out.seekg(0, ios::end);
        size_t last_space = aux_out.tellg();

        aux_out.seekg(0, ios::beg);//Me coloco después de la metadata en aux
        size_t n_aux;
        aux_out.read(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
        

        //Parto desde la metadata de data.dat: metadata_nextPosFisica y metadata_next_is_In_Data
        file_out.seekg(sizeof(size_t), ios::beg);//Me coloco después de la metadata en data.dat
        file_out.read(reinterpret_cast<char *>(&flag_punt_nextPosFisica), sizeof(flag_punt_nextPosFisica));
        file_out.read(reinterpret_cast<char *>(&flag_punt_is_in_data), sizeof(flag_punt_is_in_data));
        cout<<"Inicio: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;
        cout<<"Siguiente posicion: "<<flag_punt_nextPosFisica<<" - "<<boolalpha<<flag_punt_is_in_data<<endl<<endl;
        size_t flag_before_PosFisica = 0;
        bool flag_before_is_in_data = true;
        size_t flag_before_tamReg = 0;


        while(flag_punt_nextPosFisica!=-1){
            if(flag_punt_is_in_data) {
                // fm.seekg(temp_punt_next, ios::beg);//Queremos la metadata posición fisica del siguiente registro

                // fm.read(reinterpret_cast<char *>(&tam_reg), sizeof(tam_reg));//Leemos el tamaño del record

                // fm.seekg(-((int)sizeof(tam_reg)),ios::cur);
                // char *buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
                // fm.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.

                // //Actualizamos la metadata: flag_punt_next y flag_punt_is_in_data

                // fm.seekg(-((int)sizeof(size_t)+(int)sizeof(bool)),ios::cur);

                // fm.read(reinterpret_cast<char*>(&temp_punt_next), sizeof(temp_punt_next));//Leemos la posición física del registro que queremos leer.
                // fm.read(reinterpret_cast<char*>(&flag_punt_is_in_data), sizeof(flag_punt_is_in_data));

                // temp.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
                // delete[] buffer;//Liberamos la memoria del buffer
                // temp.showData();
            }else{
                Record temp;
                aux_out.seekg(flag_punt_nextPosFisica, ios::beg);//Voy al siguiente registro
                cout<<"Entro aux_bin: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;
                aux_out.read(reinterpret_cast<char *>(&temp_tamReg), sizeof(temp_tamReg));//Leemos el tamaño del record
            

                char *buffer = new char[temp_tamReg];//Creamos un buffer del tamaño del registro que vamos a leer.
                aux_out.seekg(-((int)sizeof(temp_tamReg)),ios::cur);

                aux_out.read(buffer, temp_tamReg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg
                temp.desempaquetar(buffer, temp_tamReg);//Desempaquetamos el buffer, asignamos los valores al record temp.

                delete[] buffer;//Liberamos la memoria del buffer

                cout<<"tamaño del record a leer: "<<temp_tamReg<<endl;
                cout<<"(RA)Record siguiente: "<<temp.punt_nextPosFisica<<" - "<<boolalpha<<temp.punt_next_is_In_Data<<endl;
                cout<<"(RA)Record anterior: "<<flag_before_PosFisica<<" - "<<boolalpha<<flag_before_is_in_data<<endl;
                
                cout<<"Posicion actual: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;
                
                if(record.Codigo < temp.Codigo) {
                    cout<<"---------------"<<endl;
                    cout<<"Entro(record < temp)"<<endl;

                    //Actualizo el puntero de record para apuntar al temp
                    record.punt_nextPosFisica = (size_t)aux_out.tellg() - temp.size_of();
                    record.punt_next_is_In_Data = false;
                    cout<<"(RI)Puntero siguiente: "<<record.punt_nextPosFisica<<" - "<<boolalpha<<record.punt_next_is_In_Data<<endl;
                    cout<<"Posicion actual: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;

                    //Actualizo el puntero before de temp para apuntar al record
                    if(flag_before_is_in_data) {
                        cout<<"---------------"<<endl;
                        cout<<"before en data:"<<endl;
                        file_out.seekg(flag_before_PosFisica, ios::beg);
                        if(file_out.tellg()!=0){//Nos posicionamos en el puntero de un record
                            file_out.seekg(temp.size_of()-sizeof(temp.punt_next_is_In_Data)-sizeof(temp.punt_nextPosFisica),ios::cur);
                        }else{//Nos posicionamos en el puntero de la metadata
                            file_out.seekg(sizeof(temp_tamReg),ios::cur);
                        }
                        
                        file_out.write(reinterpret_cast<char *>(&last_space), sizeof(size_t));
                        file_out.write(reinterpret_cast<char *>(&always_false), sizeof(temp.punt_next_is_In_Data));
                    }else{
                        cout<<"---------------"<<endl;
                        cout<<"before NO en data:"<<endl;
                        aux_out.seekg(flag_before_PosFisica, ios::beg);
                        cout<<"Pos Actual - before: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;
                        aux_out.seekg(flag_before_tamReg-sizeof(temp.punt_next_is_In_Data)-sizeof(temp.punt_nextPosFisica),ios::cur);
                        cout<<"Pos Actual - before: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;

                        aux_out.write(reinterpret_cast<char *>(&last_space), sizeof(size_t));
                        aux_out.write(reinterpret_cast<char *>(&always_false), sizeof(temp.punt_next_is_In_Data));
                    }//Actualizo cursores, me posiciono en los punteros de los records o metadata

                    cout<<"Posicion actual: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;

                    //Inserto el record en aux.bin
                    cout<<"(RI)Record siguiente: "<<record.punt_nextPosFisica<<" - "<<boolalpha<<record.punt_next_is_In_Data<<endl;
                    cout<<"Tamanio del record a insertar: "<<record.size_of()<<endl;
                    aux_out.seekg(0, ios::end);
                    auto buffer_record = record.empaquetar();
                    aux_out.write(buffer_record, record.size_of());//escribimos el record en el archivo
                    delete[] buffer_record;//liberamos la memoria del buffer
                    
                    //Actualizo la metadata de aux.bin
                    n_aux++;
                    aux_out.seekg(0, ios::beg);
                    aux_out.write(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
                    cout<<"\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\"<<endl;
                    break;

                }else if(record.Codigo == temp.Codigo){
                    cerr<<"No se pudo insertar el record: record repetido"<<endl;
                    break;
                }

                cout<<"ver: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;
                //Actualizamos la metadata: flag_punt_next y flag_punt_is_in_data
                flag_punt_nextPosFisica = temp.punt_nextPosFisica;
                flag_punt_is_in_data = temp.punt_next_is_In_Data;
                flag_before_PosFisica = (size_t)aux_out.tellg() - temp.size_of();
                flag_before_is_in_data = false;//Es false porque estamos en aux.bin
                flag_before_tamReg = temp_tamReg;
                
                cout<<"ver: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;
                //Antes de que termine:
                if(flag_punt_nextPosFisica == -1){
                    cout<<"---------------"<<endl;
                    cout<<"Entro(record > all)"<<endl;
                    //Actualizamos el puntero del temp para apuntar el nuevo record
                    int retroceso = sizeof(temp.punt_next_is_In_Data)+sizeof(temp.punt_nextPosFisica);
                    aux_out.seekg(-retroceso,ios::cur);
                    cout<<"Posicion actual: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;

                    aux_out.write(reinterpret_cast<char *>(&last_space), sizeof(size_t));
                    aux_out.write(reinterpret_cast<char *>(&always_false), sizeof(temp.punt_next_is_In_Data));
                    cout<<"Posicion actual: "<<file_out.tellg()<<" - "<<aux_out.tellg()<<endl;

                    //Escribimos el nuevo record al último
                    aux_out.seekg(0, ios::end);
                    auto buffer_record = record.empaquetar();
                    aux_out.write(buffer_record, record.size_of());//escribimos el record en el archivo
                    delete[] buffer_record;//liberamos la memoria del buffer
                    cout<<"Tamanio del record a insertar: "<<record.size_of()<<endl;
                    //Actualizo la metadata de aux.bin
                    n_aux++;
                    aux_out.seekg(0, ios::beg);
                    aux_out.write(reinterpret_cast<char *>(&n_aux), sizeof(n_aux));
                }
            }
        }
    
        aux_out.close();
        file_out.close();
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
    void Imprimir_aux_bin(){
        ifstream aux("aux.bin", ios::binary);
        if(!aux.is_open()) throw ("No se pudo abrir el archivo");
        size_t n_aux;
        aux.read(reinterpret_cast<char*>(&n_aux), sizeof(n_aux));

        if(n_aux == 0){
            cout<<"No hay registros en aux.bin"<<endl;
            return;
        }
        size_t tam_reg;
        Record temp;
        aux.seekg(sizeof(n_aux), ios::beg);//Me coloco después d vcbxe la metadata
        cout<<"Tamanio de regitros en el archivo: "<<n_aux<<endl<<endl;
        while(n_aux > 0){
            aux.read(reinterpret_cast<char *>(&tam_reg), sizeof(tam_reg));//Leemos el tamaño del record
            char *buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
            aux.seekg(-((int)sizeof(tam_reg)),ios::cur);
            aux.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg
            temp.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
            delete[] buffer;//Liberamos la memoria del buffer
            temp.showData();
            cout<<endl;
            cout<<"\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\"<<endl;
            cout<<endl;
            n_aux--;
        }

        aux.close();
    }
    void show_all_data_ordered_by_pointer(){
        ifstream fm(filename, ios::binary);
        ifstream aux("aux.bin", ios::binary);
        if(!fm.is_open()) throw ("No se pudo abrir el archivo");
        size_t temp_punt_next=0;
        bool flag_punt_is_in_data;
        size_t tam_reg;
        size_t n_aux;
        size_t n_data;
        Record temp;

        fm.read(reinterpret_cast<char*>(&n_data), sizeof(n_data));
        fm.read(reinterpret_cast<char*>(&temp_punt_next), sizeof(temp_punt_next));
        fm.read(reinterpret_cast<char*>(&flag_punt_is_in_data), sizeof(flag_punt_is_in_data));
        aux.read(reinterpret_cast<char*>(&n_aux), sizeof(n_aux));

//        fm.seekg(0,ios::end);//Queremos la metadata posición fisica del siguiente registro
//        cout<<fm.tellg()<<endl;

        while(temp_punt_next != -1){
            if(flag_punt_is_in_data) {
                fm.seekg(temp_punt_next, ios::beg);//Queremos la metadata posición fisica del siguiente registro

                fm.read(reinterpret_cast<char *>(&tam_reg), sizeof(tam_reg));//Leemos el tamaño del record

                fm.seekg(-((int)sizeof(tam_reg)),ios::cur);
                char *buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
                fm.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg.

                //Actualizamos la metadata: flag_punt_next y flag_punt_is_in_data

                fm.seekg(-((int)sizeof(size_t)+(int)sizeof(bool)),ios::cur);

                fm.read(reinterpret_cast<char*>(&temp_punt_next), sizeof(temp_punt_next));//Leemos la posición física del registro que queremos leer.
                fm.read(reinterpret_cast<char*>(&flag_punt_is_in_data), sizeof(flag_punt_is_in_data));

                temp.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
                delete[] buffer;//Liberamos la memoria del buffer
                temp.showData();
            }else{
                if(n_aux == 0){
                    cout<<"No hay registros en aux.bin"<<endl;
                    break;
                }
                aux.seekg(sizeof(n_aux), ios::beg);//Me coloco después d vcbxe la metadata
                aux.read(reinterpret_cast<char *>(&tam_reg), sizeof(tam_reg));//Leemos el tamaño del record

                char *buffer = new char[tam_reg];//Creamos un buffer del tamaño del registro que vamos a leer.
                aux.seekg(-((int)sizeof(tam_reg)),ios::cur);

                aux.read(buffer, tam_reg); //Leemos el registro que queremos leer en el buffer de tamaño tam_reg
                temp.desempaquetar(buffer, tam_reg);//Desempaquetamos el buffer, asignamos los valores al record.
                //Actualizamos la metadata: flag_punt_next y flag_punt_is_in_data
                temp_punt_next = temp.punt_nextPosFisica;
                flag_punt_is_in_data = temp.punt_next_is_In_Data;

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
