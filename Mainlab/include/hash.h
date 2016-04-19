#ifndef HASH_H
#define HASH_H

#include "heap.h"
#include "list.h"

class Hash
{
public:
	Hash(int _n1, int _n2, int _n3, int _n4, int _n5);
	~Hash(); //��������, ��� ������ hidden destructor

	List* find_list(char* key_word);
protected:
	virtual int key1(char* key_word) { return 0; };
	virtual int key2(char* key_word) { return 0; };
	virtual int key3(char* key_word) { return 0; };
	virtual int key4(char* key_word) { return 0; };
	virtual int key5(char* key_word) { return 0; };
	int combine_keys(int k1, int k2, int k3, int k4, int k5)
	{
		return 	k1 +
			k2*n1 +
			k3*n1*n2 +
			k4*n1*n2*n3 +
			k5*n1*n2*n3*n4;
	}
	// ���������� �� private, ����� ����� ���� ������� ����� ������ �����
	List** table;
	int    n1,n2,n3,n4,n5;
};

struct Article
{
	char* word;
	char* description;
};

class Diction_list : public List
{
public:
	Diction_list() : List(sizeof(Article)){};
	~Diction_list(); // purge all Article data

	void     put(Article* article); // allocate memory for word & description
	Article* find(char* word);
	void     del(Article* article) { del(article->word); };
	void     del(char* word);
};

class Diction : public Hash
{
public: 
	Diction();
	~Diction();
	
	// �������� - ��������
	// 33 �������� �� n1 � n2, ����� ���� ��-�����������
	int key1(char* key_word);
	int key2(char* key_word);

	Article* find (char* word);
	Article* auto_create(char* word);
private:
};

#endif