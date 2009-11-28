#include "Packets/Compression.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



Compression::Compression(void)
{
	
}

Compression::~Compression(void)
{
}

void Compression::create_tree()
///This function is to create a huffman tree used by the Compression and Decompression functions.
{
	int m1,m2,l,r;
	for(int i=256;i<256+n-1;i++)
	{
		m1=m2=32767;
		l=r=0;
		///according to the weight of the node, construct a huffman tree
		for(int k=0;k<=i-1;k++)
			if(m_data[k].weight!=0&&m_data[k].parent==-1)
				if(m_data[k].weight<m1)
				{
					m2=m1;
					r=l;
					m1=m_data[k].weight;
					l=k;				
				}
				else if(m_data[k].weight<m2)
				{
					m2=m_data[k].weight;
					r=k;
				}
				m_data[l].parent=i;
				m_data[r].parent=i;
				m_data[i].weight=m_data[l].weight+m_data[r].weight;
				m_data[i].left=l;
				m_data[i].right=r;
	}
}

void Compression::coding_1()
///Function performs coding of each node in the huffman tree to be used by the Compression and the Decompression functions.
{
	int k,c,i,f;
	for(i=0;i<256;i++)
		if(m_data[i].weight!=0)
		{//from the leaf node the caculate the code
			k=10;
			c=i;
			f=m_data[i].parent;
			//decide the coding according to the depth of the node
			while(f!=-1)
			{
				if(m_data[f].left==c)
					m_data[i].code[--k]='0';
				else 
					m_data[i].code[--k]='1';
				c=f;
				f=m_data[f].parent;
			}
			m_data[i].start=k;
		}
}

int Compression::Decompress(char* input, int inputsize, char* output, int outputsize) 
///The function which performs the decompression of data.
{
	int tmp=*input-48;
	if(tmp<0) tmp+=256;
	int m=0;
	int i,t,j=0;
	n=*input-48;
	if(n<0) n+=256;
	char* ptr=new char[1024];
	if(n==1)
	//only contain 1 character
	{
		j=*(int*)(input+2);
		for(i=0;i<j;i++)
			*(ptr+i)=*(input+1);
		*(ptr+j)='\0';
		
	}
	else
	//using the huffman tree
	{
		char *tree=new char[tmp*2+10];
		int length=*(int*)(input+2+tmp*2);
		char *result=new char[length*8+10];
		memcpy(tree,input,1+tmp*2);
		*(tree+tmp*2+1)='\0';
		//convert input from bit to char
		length=bittochar((input+1+tmp*2), result);
	
		t=254+n;
		m=length;
	 	m_data=new HuffmanNode[256+n];
		//initialize
		for(i=0;i<=255;i++)
		{
			m_data[i].weight=m_data[i].start=0;
			m_data[i].parent=-1;
			m_data[i].left=m_data[i].right=-1;
		}
		m=length;
		//decoding the huffman tree information to rebuild the tree
		for( i=0;i<n;i++)
		{
			int index=*(tree+i*2+1);
			if(index<0) index+=256;
			m_data[index].weight=*(tree+i*2+2)-48;
			if(m_data[index].weight<0)
				m_data[index].weight+=256;
		}
		for(i=0;i<n-1;i++)
		{
			m_data[256+i].weight=m_data[256+i].start=0;
			m_data[256+i].parent=-1;
			m_data[256+i].left=m_data[256+i].right=-1;
		}
		
		//rebuild the tree
		create_tree();
		coding_1();
	   
		//decode the output by huffman tree
		for(int i=0;i<m;i++)	
			if(m_data[t].left!=-1)
			{
				if(*(result+i)=='0')
					t=m_data[t].left;
				else if(*(result+i)=='1')
					t=m_data[t].right;
			}
			else
			{
				*(ptr+j++)=(char)t;
				t=254+n;
				i--;
			}	
		*(ptr+j++)=(char)t;
		*(ptr+j)='\0';
		delete m_data;
		delete result;
		delete tree;
	}
	//if output buffer is not big enough, returns the required size
	if(j>outputsize)
	{
		 //outputsize=j+10;
		 j=-1;
	}
	else
		memcpy(output,ptr,j+1);
	delete ptr;
	
	return j;
}

int Compression::Compress(char* input, int length, char* output, int outputsize) 
///The function which performs the compression of data.
{
	char ch,c[10],*tree;
	int i=0,j=0,k,l,g,nlength=6;
	int m;
	n=0;
	//initialize
	m_data=(HuffmanNode*)malloc(256*sizeof(HuffmanNode));	
	for(i=0;i<=255;i++)
	{
		m_data[i].weight=m_data[i].start=0;
		m_data[i].parent=-1;
		m_data[i].left=m_data[i].right=-1;
	}
	m=length;
	//caculate the frequency of each character
	for( i=0;i<m;i++)
	{
		ch=*(input+i);
		int hh=ch;
		if (hh<0) hh+=256;
		++m_data[hh].weight==1?n++:n;
	}
	m_data=(HuffmanNode*)realloc(m_data,(n-1+256)*sizeof(HuffmanNode));
	
	//initialize the tree node
	for(i=0;i<n-1;i++)
	{
		m_data[256+i].weight=m_data[256+i].start=0;
		m_data[256+i].parent=-1;
		m_data[256+i].left=m_data[256+i].right=-1;
	}
	
	//tree is used to save the information of huffman tree
	tree=new char[n*2+length+10];
	*(tree+j++)=n+48;
	if(n==1)
	//no need to use the huffman if there's only one kind of character
	{
		*(tree+j++)=*input;
		memcpy((tree+j),&length,sizeof(int));
		j+=4;
		*(tree+j++)='\0';
		memcpy(output,tree,nlength+1);
	}
	else
	{
		//save the value of character and its frequency
		for(i=0;i<=255;i++)
		{
			if(m_data[i].weight!=0)
			{
				*(tree+j++)=i;
				if(m_data[i].weight+48>255)
					*(tree+j++)=255;
				else
					*(tree+j++)=m_data[i].weight+48;
			}
			
		}
		//*(tree+j++)='\0';
		//build the huffman tree
		create_tree();
		coding_1();
		char *result=new char[m*8+10];
		strcpy(result,"");
		int total=0;
		//save the coding result into result
		for(i=0;i<m;i++)
		{
			l=0;
			ch=*(input+i);
			int index=ch;
			if(index<0) index+=256;
			for( k=m_data[index].start;k<10;k++)
				c[l++]=m_data[index].code[k];
			c[l]='\0';
			strcat(result,c);
			total+=strlen(c);
		}
		//convert it from char into bit
		nlength=chartobit(result, output,total);
		if((nlength+n*2+10)>outputsize)
		{
			//outputsize=nlength+n*2+10;
			nlength=-1;
		}
		else
		{
			memcpy(tree+j,output,nlength+1);
			nlength+=(n*2+1);
			memcpy(output,tree,nlength+1);
		}
		delete result;
	}
	free(m_data);
	delete tree;
	return nlength;
	
} 
int Compression::chartobit(char *input,char *output, int length)
///The function converts char to bits used by the Compression function.
{
	//use length to indicate the length of the output
	int i=0,plength=0,tmp;
	char* p=new char[length+10];
	*(p+plength++)=length%8;
	tmp=length/8+5;
	if(length%8!=0)	tmp++;
	memcpy((p+plength),&tmp,sizeof(int));
	plength+=4;
	//convert char to bit (1 char to 8 bits)
	while (length>8)
	{
		tmp=(*(input+i++)-48)*128;
		tmp+=(*(input+i++)-48)*64;
		tmp+=(*(input+i++)-48)*32;
		tmp+=(*(input+i++)-48)*16;
		tmp+=(*(input+i++)-48)*8;
		tmp+=(*(input+i++)-48)*4;
		tmp+=(*(input+i++)-48)*2;
		tmp+=(*(input+i++)-48);
		*(p+plength++)=tmp;
		length-=8;
	}
	//convert the last char to j bits
	*(p+plength)=0;
	for(int j=length;j>0;j--)
	{
		*(p+plength)+=(*(input+i++)-48)*poww(j-1);
	}
	plength++;
	*(p+plength)='\0';
	memcpy(output,p,plength+1);
	delete p;
	return plength;

}
int Compression::bittochar(char *input, char *output)
///The function converts bits to char used by the Compression function.
{
	int i=5,length;
	length=*(int*)(input+1);
	//use length to indicate the length of the output
	int ll=length-5,plength=0,tmp,j;
	char* p=new char[length*8+10];
	while(ll>1)
	{
		//convert bits to char (8 bits to 1 char)
		tmp=*(input+i);
		if(tmp<0)
			tmp+=256;
		*(p+plength++)=tmp/128+48;
		if(tmp>=128)	tmp-=128;
		*(p+plength++)=tmp/64+48;
		if(tmp>=64)	tmp-=64;
		*(p+plength++)=tmp/32+48;
		if(tmp>=32)	tmp-=32;
		*(p+plength++)=tmp/16+48;
		if(tmp>=16)	tmp-=16;
		*(p+plength++)=tmp/8+48;
		if(tmp>=8)	tmp-=8;
		*(p+plength++)=tmp/4+48;
		if(tmp>=4)	tmp-=4;
		*(p+plength++)=tmp/2+48;
		if(tmp>=2)	tmp-=2;
		*(p+plength++)=tmp+48;
		ll--; i++;
	}
	
	//this indicates the last char are made of 8 bits
	if(*input==0)
		*input=8;
	tmp=*(input+i);
	if(tmp<0)
		tmp+=256;
	//convert the last j bits to 1 char
	for (j=*input;j>0;j--)
	{
		*(p+plength++)=tmp/poww(j-1)+48;
		if(tmp>=poww(j-1))	tmp-=poww(j-1);
	}
	*(p+plength)='\0';
	memcpy(output,p,plength+1);
	delete p;
	return plength;
}
int Compression::poww(int n)
{
	int result=1;
	for(int i=0;i<n;i++)
		result*=2;
	return result;

}