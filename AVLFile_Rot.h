#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

struct Record
{
	int cod;
	char nombre[12];
	int ciclo;

	long left = -1;
	long right = -1;
    long height = 1;

	void setData()
	{
		cout << "Codigo: ";
		cin >> cod;
		cout << "Nombre: ";
		cin >> nombre;
		cout << "Ciclo: ";
		cin >> ciclo;
	}

	void showData()
	{
		cout << "\nCodigo: " << cod;
		cout << "\nNombre: " << nombre;
		cout << "\nLeft: " << left;
		cout << "\nRight: " << right;
		cout << "\nCiclo : " << ciclo << endl;
	}
};

class AVLFile
{
private:
	string fileName;
	long posRoot;

	bool fileExists()
	{
		ifstream dataFile(this->fileName, ios::binary);
		bool exists = dataFile.good();
		dataFile.close();
		return exists;
	}

	void createFile()
	{
		ofstream dataFile(this->fileName, ios::app | ios::binary);
		dataFile.close();
	}

public:
	AVLFile(string fileName)
	{
		this->posRoot = 0;
		this->fileName = fileName;
		if (!this->fileExists())
			this->createFile();
	}

	Record find(int key)
	{
		ifstream dataFile(this->fileName, ios::binary);
		if (!dataFile.is_open())
			throw runtime_error("Error opening file " + this->fileName);
		return find(this->posRoot, key, dataFile);
	}

	void insert(Record record)
	{
		fstream dataFile(this->fileName, ios::binary | ios::in | ios::out);
		if (!dataFile.is_open())
			throw runtime_error("Error opening file " + this->fileName);
		insert(this->posRoot, record, dataFile);
	}

	vector<Record> inorder()
	{
		vector<Record> result{};
		ifstream dataFile(this->fileName, ios::binary);
		if (!dataFile.is_open())
			throw runtime_error("Error opening file " + this->fileName);
		inorder(this->posRoot, result, dataFile);
		return result;
	}

private:
	Record find(long pos_node, int key, ifstream &dataFile)
	{
		Record record{};
		if (pos_node == -1)
			return record;

		dataFile.seekg(pos_node * sizeof(Record), ios::beg);
		dataFile.read(reinterpret_cast<char *>(&record), sizeof(Record));

		if (record.cod == key)
			return record;

		if (key < record.cod)
			return find(record.left, key, dataFile);
		else
			return find(record.right, key, dataFile);
	}

	void insert(long &pos_node, Record record, fstream &dataFile) {
    if (pos_node == -1) {
        //Si el nodo es nulo, inserta el nuevo nodo aquí.
        dataFile.seekp(0, ios::end);
        dataFile.write(reinterpret_cast<char *>(&record), sizeof(record));
        pos_node = this->size(); //Actualiza la posición del nodo
        return;
    }

	
    Record recordTemp{};

	//Verifica si el nodo se ha movido por rotaciones
    if (pos_node != this->size()) {
        //Si el nodo se ha movido, actualiza pos_node y la lectura de recordTemp
        pos_node = this->size() - 1;
        dataFile.seekg(pos_node * sizeof(Record), ios::beg);
        dataFile.read(reinterpret_cast<char *>(&recordTemp), sizeof(Record));
    }

    dataFile.seekg(pos_node * sizeof(Record), ios::beg);
    dataFile.read(reinterpret_cast<char *>(&recordTemp), sizeof(Record));

    if (record.cod < recordTemp.cod) {
        insert(recordTemp.left, record, dataFile);
    }
    else {
        insert(recordTemp.right, record, dataFile);
    }
	
    //Actualiza la altura del nodo actual
    recordTemp.height = 1 + max(getHeight(recordTemp.left, dataFile), getHeight(recordTemp.right, dataFile));

    //Calcula el balancing factor del nodo
    int balance = getBalanceFactor(recordTemp, dataFile);

    //Realiza rotaciones si es necesario para mantener el equilibrio
    if (balance > 1) {
        if (record.cod < getRecord(recordTemp.left, dataFile).cod) {
            //Rotación a la derecha
            rotateRight(pos_node, dataFile);
        }
        else {
            //Rotación doble a la izquierda-derecha
            rotateLeft(recordTemp.left, dataFile);
            rotateRight(pos_node, dataFile);
        }
    }
    else if (balance < -1) {
        if (record.cod > getRecord(recordTemp.right, dataFile).cod) {
            //Rotación a la izquierda
            rotateLeft(pos_node, dataFile);
        }
        else {
            //Rotación doble a la derecha-izquierda
            rotateRight(recordTemp.right, dataFile);
            rotateLeft(pos_node, dataFile);
        }
    }

    //Cerrar y volver a abrir el archivo en modo escritura
    dataFile.close();
    dataFile.open(this->fileName, ios::binary | ios::in | ios::out);

    //Actualiza el nodo en el archivo
    dataFile.seekp(pos_node * sizeof(Record), ios::beg);
    dataFile.write(reinterpret_cast<char *>(&recordTemp), sizeof(Record));
}

		int getHeight(long pos_node, fstream &dataFile){
    	if (pos_node == -1) return 0;
    	Record record = getRecord(pos_node, dataFile);
    	return record.height;
	}

	Record& getRecord(long pos_node, fstream &dataFile){
    	static Record record; 
    	dataFile.seekg(pos_node * sizeof(Record), ios::beg);
    	dataFile.read(reinterpret_cast<char *>(&record), sizeof(Record));
    	return record;
	}


	int getBalanceFactor(Record record, fstream &dataFile){
    	if (record.left == -1 && record.right == -1)
        	return 0;

    	int leftHeight = getHeight(record.left, dataFile);
    	int rightHeight = getHeight(record.right, dataFile);

    	return leftHeight - rightHeight;
	}

	void rotateRight(long &pos_node, fstream &dataFile){
    	long newRoot = getRecord(pos_node, dataFile).left;
    	long newRootRight = getRecord(newRoot, dataFile).right;

    	getRecord(pos_node, dataFile).left = newRootRight;
    	getRecord(newRoot, dataFile).right = pos_node;

    	//Actualiza alturas
    	getRecord(pos_node, dataFile).height = 1 + max(getHeight(getRecord(pos_node, dataFile).left, dataFile), getHeight(getRecord(pos_node, dataFile).right, dataFile));
    	getRecord(newRoot, dataFile).height = 1 + max(getHeight(getRecord(newRoot, dataFile).left, dataFile), getHeight(getRecord(newRoot, dataFile).right, dataFile));

    	//Actualiza la posición de la raíz
    	pos_node = newRoot;
	}

	void rotateLeft(long &pos_node, fstream &dataFile){
    	long newRoot = getRecord(pos_node, dataFile).right;
    	long newRootLeft = getRecord(newRoot, dataFile).left;

    	getRecord(pos_node, dataFile).right = newRootLeft;
    	getRecord(newRoot, dataFile).left = pos_node;

    	//Actualiza alturas
    	getRecord(pos_node, dataFile).height = 1 + max(getHeight(getRecord(pos_node, dataFile).left, dataFile), getHeight(getRecord(pos_node, dataFile).right, dataFile));
    	getRecord(newRoot, dataFile).height = 1 + max(getHeight(getRecord(newRoot, dataFile).left, dataFile), getHeight(getRecord(newRoot, dataFile).right, dataFile));

    	//Actualiza la posición de la raíz
    	pos_node = newRoot;
	}

	void inorder(long pos_node, vector<Record> &result, ifstream &dataFile)
	{
		if (pos_node == -1)
			return;

		Record record{};
		dataFile.seekg(pos_node * sizeof(Record), ios::beg);
		dataFile.read(reinterpret_cast<char *>(&record), sizeof(record));

		inorder(record.left, result, dataFile);
		result.push_back(record);
		inorder(record.right, result, dataFile);
	}

	int size()
	{
		ifstream dataFile(this->fileName, ios::binary);
		if (!dataFile.is_open())
			throw runtime_error("Error opening file " + this->fileName);
		dataFile.seekg(0, ios::end);
		long total_bytes = dataFile.tellg();
		dataFile.close();
		return total_bytes / sizeof(Record);
	}
};