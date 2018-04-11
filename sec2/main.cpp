#include<stdio.h>
#include<string.h>
/*第一次实验代码
*void overflow(const char* input) {
*	char buf[8];
*	printf("Virtual address of 'buf'=0x%p\n", buf);
*	strcpy(buf, input);
*}
*
*void fun() {
*	printf("Function 'fun' has been called without an explicitly invocation.\n");
*	printf("Buffer Overflow attack succeeded!\n");
*	//something
*}
*
*int main(int grac, char* argv) {
*	printf("Virtual address of 'overflow'=0x%p\n", overflow);
*	printf("Vitrual address of 'fun' = 0x%p\n", fun);
*	char input[]{'A','A','A','A','A','A','A','A'};
*	overflow(input);
*	return 0;
*}
*/
#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<cstdlib>
#include"des.h"

using namespace std;

const int num = 88;

void help(const string &str)
{
	string header("DES Manual copied from github.com/Wenyi-M/des, modified by github.com:zwd1997");
	string command("Command: des option srcfile tarfile keyword");
	string options("Option :");
	string e("-e: encryption the srcfile with keyword, the result is stored in tarfile");
	string d("-d: decryption the srcfile with keyword, the result is stored in tarfile");
	string help("-h: show this manual");

	for (size_t i = 0; i < num - 1; ++i)
	{
		cout << '*';
	}
	cout << endl;

	for (size_t i = 0; i < num; ++i)
	{
		if (i == 0 || i == num - 1)
			cout << '*';
		else if (i == num / 2 - 3)
		{
			cout << header;
			i += header.size();
		}
		else cout << ' ';
	}
	cout << endl;
	for (size_t i = 0; i < num; ++i)
	{
		if (i == 0 || i == num - 1)
			cout << '*';
		else if (i == 3)
		{
			cout << command;
			i += command.size();
		}
		else cout << ' ';
	}
	cout << endl;
	for (size_t i = 0; i < num; ++i)
	{
		if (i == 0 || i == num - 1)
			cout << '*';
		else if (i == 3)
		{
			cout << options;
			i += options.size();
		}
		else cout << ' ';
	}
	cout << endl;

	for (size_t i = 0; i < num; ++i)
	{
		if (i == 0 || i == num - 1)
			cout << '*';
		else if (i == 3)
		{
			cout << e;
			i += e.size();
		}
		else cout << ' ';
	}
	cout << endl;

	for (size_t i = 0; i < num; ++i)
	{
		if (i == 0 || i == num - 1)
			cout << '*';
		else if (i == 3)
		{
			cout << d;
			i += d.size();
		}
		else cout << ' ';
	}
	cout << endl;

	for (size_t i = 0; i < num; ++i)
	{
		if (i == 0 || i == num - 1)
			cout << '*';
		else if (i == 3)
		{
			cout << help;
			i += help.size();
		}
		else cout << ' ';
	}
	cout << endl;

	for (size_t i = 0; i < num - 1; ++i)
		cout << '*';
	cout << endl;

	cout << str << endl;
	system("pause");
	exit(-1);//退出

}

void StrFromBlock(char *str, const Block &block)
{
	memset(str, 0, 8);
	for (size_t i = 0; i < block.size(); ++i)
	{
		if (true == block[i])
			*((unsigned char *)(str)+i / 8) |= (1 << (7 - i % 8));
	}
}

void BlockFromStr(Block & block, const char * str)
{
	for (size_t i = 0; i < block.size(); ++i)
	{
		if (0 != (*((unsigned char *)(str)+i / 8) & (1 << (7 - i % 8))))
			block[i] = true;
		else 	block[i] = false;
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2 || argv[1][0] != '-')
	{
		help("Command Args Error");
		//exit(-1);
	}
	Method method;
	switch (argv[1][1])
	{
	case 'e':
		method = e;
		break;
	case 'd':
		method = d;
		break;
	case 'h':
		help("");
		break;
	default:
		help("Command Args Errors");
		//exit(-1);
		break;
	}
	if (argc < 5 || strlen(argv[4]) < 8)
		help("Command Args Error");

	ifstream srcFile(argv[2]);
	ofstream tarFile(argv[3]);
	if (!srcFile || !tarFile)help("File Open Error");

	Block block, bkey;
	BlockFromStr(bkey, argv[4]);
	char buffer[8];
	while (1) {
		memset(buffer, 0, 8);
		srcFile.read(buffer, 8);
		if (srcFile.eof())break;
		BlockFromStr(block, buffer);
		des(block, bkey, method);
		StrFromBlock(buffer, block);
		tarFile.write(buffer, 8);
	}
	srcFile.close();
	tarFile.close();
	//system("pause");
	return 0;
}