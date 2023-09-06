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

	void insert(long pos_node, Record record, fstream &dataFile)
	{
		if (this->size() == 0)
		{
			dataFile.write(reinterpret_cast<char *>(&record), sizeof(record));
			return;
		}

		Record recordTemp{};
		dataFile.seekg(pos_node * sizeof(Record), ios::beg);
		dataFile.read(reinterpret_cast<char *>(&recordTemp), sizeof(Record));

		recordTemp.showData();

		if (record.cod < recordTemp.cod)
		{
			if (recordTemp.left == -1)
			{
				recordTemp.left = this->size();
				dataFile.seekp(pos_node * sizeof(Record), ios::beg);
				dataFile.write(reinterpret_cast<char *>(&recordTemp), sizeof(Record));
				dataFile.seekp(0, ios::end);
				dataFile.write(reinterpret_cast<char *>(&record), sizeof(record));
			}
			else
				insert(recordTemp.left, record, dataFile);
		}
		else
		{

			if (recordTemp.right == -1)
			{
				recordTemp.right = this->size();
				dataFile.seekp(pos_node * sizeof(Record), ios::beg);
				dataFile.write(reinterpret_cast<char *>(&recordTemp), sizeof(Record));
				dataFile.seekp(0, ios::end);
				dataFile.write(reinterpret_cast<char *>(&record), sizeof(record));
			}
			else
				insert(recordTemp.right, record, dataFile);
		}
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