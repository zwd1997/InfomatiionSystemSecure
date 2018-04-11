#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<cstdlib>
#include<iostream>
#include<bitset>

typedef std::bitset<64> Block;
typedef std::bitset<56> Key;
typedef std::bitset<48> Code;

typedef std::bitset<32> HBlock;
typedef std::bitset<28> HKey;
typedef std::bitset<24> HCode;

typedef enum { e, d } Method;

using namespace std;

const int num = 88;

const static unsigned char ip_table[64] = {
	58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
	62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
	57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
	61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7
};

//拓展置换，32->48
static const unsigned char expa_perm[48] = {
	32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,
	12,13,14,15,16,17,16,17,18,19,20,21,22,23,
	24,25,24,25,26,27,28,29,28,29,30,31,32,1
};

//s盒替代
const static unsigned char sbox[8][64] = {
	{//s1
		14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
		0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
		4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
		15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
	},
	{//s2
		15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
		3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
		0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
		13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
	},
	{//s3
		10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
		13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
		13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
		1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
	},
	{//s4
		7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
		13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
		10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
		3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
	},
	{//s5
		2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
		14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
		4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
		11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
	},
	{//s6
		12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
		10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
		9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
		4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
	},
	{//s7
		4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
		13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
		1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
		6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
	},
	{//s8
		13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
		1,15,13,8,10,3,7,14,12,5,6,11,0,14,9,2,
		7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
		2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
	}
};

//p盒置换
const static unsigned char p_table[32] = {
	16,7,20,21,29,12,28,17,1,15,26,5,18,31,10,
	2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25
};


//p逆置换
const static unsigned char ipr_table[64] = {
	40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,
	38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,
	36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
	34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25
};

//将数据块初始置换为左右两个部分
int ip(const Block &block, HBlock &left, HBlock &right)
{
	for (size_t i = 0; i < right.size(); i++)
	{
		right[i] = block[ip_table[i] - 1];//左半部分赋给右
	}
	for (size_t i = 0; i < left.size(); i++)
	{
		left[i] = block[ip_table[i + left.size()] - 1];//右半部分赋给左
	}
	return 0;
}

//一轮加解密运算，不带交换
int des_turn(HBlock &left, HBlock &right, const Code &subkey)
{
	Code code;//48位数据块
	HBlock pcode;//32位数据块
				 //将右半部分拓展为48位
	for (size_t i = 0; i < code.size(); i++)
	{
		code[i] = right[expa_perm[i] - 1];//拓展置换
	}
	code ^= subkey;//与子密钥异或
				   //s盒替代
	std::bitset<4> col;//s盒的行
	std::bitset<2> row;//s盒的列
	for (size_t i = 0; i < 8; ++i)
	{//8个盒子
		row[0] = code[6 * i];//获取行标
		row[1] = code[6 * i + 5];
		col[0] = code[6 * i + 1];//获取列标
		col[1] = code[6 * i + 2];
		col[2] = code[6 * i + 3];
		col[3] = code[6 * i + 4];
		std::bitset<4>temp(sbox[i][row.to_ulong() * 16 + col.to_ulong()]);
		for (size_t j = 0; j < temp.size(); ++j)
		{
			code[4 * i + j] = temp[j];//将32位暂存于48位中
		}
	}
	for (size_t i = 0; i < pcode.size(); ++i)
	{
		pcode[i] = code[p_table[i] - 1];//p盒置换
	}
	left ^= pcode;
	return 0;
}

//将左右两个部分数据进行末置换形成一个数据块
int rip(const HBlock &left, const HBlock &right, Block &block)
{
	for (size_t i = 0; i < block.size(); ++i)
	{
		if (ipr_table[i] <= 32)
		{
			block[i] = right[ipr_table[i] - 1];//从right部分获取数据
		}
		else
		{
			block[i] = left[ipr_table[i] - 32 - 1];//从left部分获取数据
		}
	}
	return 0;
}

//交换左右两个部分
int exchange(HBlock &left, HBlock &right)
{
	HBlock temp;
	for (size_t i = 0; i < temp.size(); ++i)
	{
		temp[i] = left[i];
	}
	for (size_t i = 0; i < temp.size(); ++i)
	{
		left[i] = right[i];
	}
	for (size_t i = 0; i < temp.size(); ++i)
	{
		right[i] = temp[i];
	}
	return 0;
}

//密钥置换表，将64位置换为56位
const static unsigned char key_table[56] = {
	57,49,41,33,25,17,9,1,
	58,50,42,34,26,18,10,2,
	59,51,43,35,27,19,11,3,
	60,52,44,36,63,55,47,39,
	31,23,15,7,62,54,46,38,
	30,22,14,6,61,53,45,37,
	29,21,13,5,28,20,12,4
};

//每轮移动位数
const static unsigned char bit_shift[16] = {
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

//压缩置换表，将56位密钥压缩为48位
const static unsigned char comp_perm[48] = {
	14,17,11,24,1,5,
	3,28,15,6,21,10,
	23,19,12,4,26,8,
	16,7,27,20,13,2,
	41,52,31,37,47,55,
	30,40,51,45,33,48,
	44,49,39,56,34,53,
	46,42,50,36,29,32
};

//获取bkey产生的第n轮密钥
Code getkey(const unsigned int n, const Block &bkey)
{//n属于【0，15】，bkey为64位密钥
	Code result;//返回值，48位
	Key key;//56位密钥
	unsigned int klen = key.size(), rlen = result.size();//分别为56和48
														 //获取56位密钥
	for (size_t i = 0; i < key.size(); ++i)
	{
		key[i] = bkey[key_table[i] - 1];//密钥置换
	}
	for (size_t i = 0; i <= n; ++i)
	{//循环移位
		for (size_t j = 0; j < bit_shift[i]; ++j)
		{//将密钥循环存在result中
			result[rlen - bit_shift[i] + j] = key[klen - bit_shift[i] + j];
			result[rlen / 2 - bit_shift[i] + j] = key[klen / 2 - bit_shift[i] + j];
		}
		key <<= bit_shift[i];
		for (size_t j = 0; j < bit_shift[i]; ++j)
		{//写回key
			key[klen / 2 + j] = result[rlen - bit_shift[i] + j];
			key[j] = result[rlen / 2 - bit_shift[i] + j];
		}
	}
	//压缩置换
	for (size_t i = 0; i < result.size(); ++i)
	{
		result[i] = key[comp_perm[i] - 1];
	}
	return 0;
}

//加解密运算
int des(Block &block, Block &bkey, const Method method)
{//block为数据块,bkey为64位密钥
	HBlock left, right;
	ip(block, left, right);
	switch (method)
	{
	case e://加密
		for (char i = 0; i < 16; ++i)
		{
			Code key = getkey(i, bkey);
			des_turn(left, right, key);
			if (i != 15)exchange(left, right);
		}
		break;
	case d://解密
		for (char i = 15; i >= 0; --i)
		{
			Code key = getkey(i, bkey);
			des_turn(left, right, key);
			if (i != 0)exchange(left, right);
		}
		break;
	default:
		break;
	}
	rip(left, right, block);
	return 0;
}

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
	exit(-1);//ÍË³ö

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
