#pragma once

class compression
{
	
	typedef struct
	{
		int parent;
		int weight;	
		char code[10];
		int left,right;
		int start;
	}huffnode;
	huffnode *m_data;
	int n;
public:
	compression(void);
	void Coding_1();
	void Creatree();
	int comp(char* input, int length, char* output, int &outputsize);
	int decomp(char* input, char* output, int &outputsize); 
	int chartobit(char *input, char *output, int length);
	int bittochar(char *input, char *output);
	int poww(int n);
	~compression(void);
};
