#pragma once

///This class compresses the data to be sent resulting in better utilization of the available maximum packet size.
class Compression
{
	///Structure used for Compression and the Decompression functions.
	struct HuffmanNode
	{
		int parent;
		int weight;	
		char code[10];
		int left,right;
		int start;
	};
	HuffmanNode *m_data;

	int n;
public:
	///Constructor
	Compression(void);
	~Compression(void);

	///The function which performs the compression of data.
	int Compress(char* input, int inputsize, char* output, int outputsize);

	///The function which performs the decompression of data.
	int Decompress(char* input, int inputsize, char* output, int outputsize); 

private:
	///Function performs coding of each node in the huffman tree to be used by the Compression and the Decompression functions.
	void coding_1();

	///This function is to create a huffman tree used by the Compression and Decompression functions.
	void create_tree();

	///The function converts char to bits used by the Compression function.
	int chartobit(char *input, char *output, int length);

	///The function converts bits to char used by the Decompression function.
	int bittochar(char *input, char *output);

	///Returns an int value which is the power of the function parameter value.
	int poww(int n);

	///Destructor
	
};
