/*
 * @Description:
 * @Author: yuqingli
 * @Contact: yuqingli05@outlook.com
 * @Date: 2021-05-29 20:47:03
 * @LastEditTime: 2022-05-28 16:46:54
 * @LastEditors: yuqingli
 */
#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hex.h"
#include "getopt.h"

int main(int argc, char **argv)
{
	int option_index = 0;
	int c;
	static struct option long_options[] = {
		{"add", no_argument, NULL, 200},
		{"del", required_argument, NULL, 201},
		{"set", required_argument, NULL, 202},
		{"cut", required_argument, NULL, 203},
		{"version", no_argument, NULL, 'v'},
		{"help", no_argument, NULL, '?'},
		{0, 0, 0, 0}};

	int cmd = 200; //命令字 add del set
	char cmdStr[1024] = "";

	int inFileNum = 0;
	int inFileType[64];			  //输入文件类型 1:bin 或者 0:hex
	uint32_t inbinaryAddress[64]; //如果是二进制文件，二进制文件的地址
	static char inFilePath[64][512];

	int outFileType = 0;	  //输出文件类型 1:bin 或者 0:hex -1:自适应
	int binaryOutMode = 0xFF; //二进制文件输出模式 <0:分段输出 >充填输出字节
	char outFilePath[1024] = "";

	uint32_t binaryAddress = 0; //后面的二进制文件地址
	int currentFileType = -1;	//当前文件类型 1:bin 或者 0:hex

	while ((c = getopt_long(argc, argv, "f:o:bhx:i:v", long_options, &option_index)) != -1)
	{
		switch (c)
		{
		case 200:
			cmd = c;
			break;
		case 201:
		case 202:
		case 203:
			strcpy(cmdStr, optarg);
			cmd = c;
			break;
		case 'f':
			strcpy(inFilePath[inFileNum], optarg);
			if (currentFileType == -1)
			{
				char ext[32] = "";
				_splitpath(optarg, NULL, NULL, NULL, ext);
				if ((ext[1] == 'b' || ext[1] == 'B') &&
					(ext[2] == 'i' || ext[2] == 'I') &&
					(ext[3] == 'n' || ext[3] == 'N'))
				{
					inFileType[inFileNum] = 1;
				}
				else //默认是 hex
				{
					inFileType[inFileNum] = 0;
				}
			}
			else
			{
				inFileType[inFileNum] = currentFileType;
			}
			inbinaryAddress[inFileNum] = binaryAddress;
			inFileNum++;
			binaryAddress = 0; //恢复默认地址
			break;
		case 'o':
			strcpy(outFilePath, optarg);
			if (currentFileType == -1)
			{
				char ext[32] = "";
				_splitpath(optarg, NULL, NULL, NULL, ext);
				if ((ext[1] == 'b' || ext[1] == 'B') &&
					(ext[2] == 'i' || ext[2] == 'I') &&
					(ext[3] == 'n' || ext[3] == 'N'))
				{
					outFileType = 1;
				}
				else //默认是 hex
				{
					outFileType = 0;
				}
			}
			else
			{
				outFileType = currentFileType;
			}
			break;
		case 'b':
			currentFileType = 1;
			break;
		case 'h':
			currentFileType = 0;
			break;
		case 'x':
			binaryAddress = strtoul(optarg, NULL, 16);
			break;
		case 'i':
			binaryOutMode = atoi(optarg);
			break;
		case 'v':
			printf("version:V0.0.1 %s\n", __DATE__ " "__TIME__);
			return 0;
		case '?':
		default:
			printf("\t--add 执行文件合并操作,把输入的文件合并成一个文件,当没有输入命令时候add是默认命令\n");
			printf("\t--del 对输入文件删除指定地址和长度 地址:长度 eg: --sub 0x00000000:1024 (十六进制:十进制)\n");
			printf("\t--cut 对输入文件裁剪出指定地址和长度 eg: --cut 0x00000000:1024 (十六进制:十进制)\n");
			printf("\t--set 对输入文件指定地址进行充填操作 地址:长度:充填字节 eg: --set 0x00000000:1024:0xFF(十六进制:十进制:十六进制)\n");
			printf("\t-f 后面跟输入的文件名，最多输入64个文件\n");
			printf("\t-o 后面跟输出的文件名\n");
			printf("\t-b 指定后面输入和输出的文件类型为二进制文件,直到再次遇见改变文件类型的参数\n");
			printf("\t-h 指定后面输入和输出的文件类型为hex文件,直到再次遇见改变文件类型的参数\n");
			printf("\t-x 指定后面输入二进制文件的开始地址,默认为0, -x 0x00;只对后面第一个二进制输入文件生效一次\n");
			printf("\t-i 设置输出二进制文件的充填字节,设置-1将分段输出  eg:-i 255(设置充填字节为0xFF,默认0xFF)\n");
			printf("\t--version 输出版本信息\n");
			printf("\t--help    输出帮助信息\n");
			return 0;
		}
	}

	if (cmd == 200) // add
	{
		PtrList hex = PTRLIST_INITIALIZER;
		for (int i = 0; i < inFileNum; i++)
		{
			int r = 0;
			if (inFileType[i] == 1)
			{
				r = bin_readFile(&hex, inFilePath[i], inbinaryAddress[i], false);
				if (r != 0)
				{
					if (r == -1)
						printf("文件%s打开失败\n", inFilePath[i]);
					else if (r == -2)
						printf("文件%s存在重合区域\n", inFilePath[i]);
					else
						printf("文件%s 处理错误%d\n", inFilePath[i], r);
					goto RETURN;
				}
			}
			else if (inFileType[i] == 0)
			{
				r = hex_readFile(&hex, inFilePath[i], false);
				if (r != 0)
				{
					if (r == -1)
						printf("文件<%s>打开失败\n", inFilePath[i]);
					else if (r == -2)
						printf("文件<%s>存在重合区域\n", inFilePath[i]);
					else if (r > 0)
						printf("文件<%s>在行%d错误\n", inFilePath[i], r);
					else
						printf("文件<%s>处理错误%d\n", inFilePath[i], r);
					goto RETURN;
				}
			}
		}
		if (outFileType == 1)
		{
			int r = bin_writeFile(&hex, outFilePath, binaryOutMode, true);
			if (r != 0)
			{
				if (r == -1)
					printf("输出文件<%s>打开失败\n", outFilePath);
				else if (r == -2)
					printf("输出文件<%s>已存在\n", outFilePath);
				else
					printf("输出文件<%s>处理错误%d\n", outFilePath, r);
			}
		}
		else if (outFileType == 0)
		{
			int r = hex_writeFile(&hex, outFilePath, true);
			if (r != 0)
			{
				if (r == -1)
					printf("输出文件<%s>打开失败\n", outFilePath);
				else if (r == -2)
					printf("输出文件<%s>已存在\n", outFilePath);
				else
					printf("输出文件<%s>处理错误%d\n", outFilePath, r);
			}
		}

		hex_delete(&hex);
	}
	else // del set cut
	{
		uint32_t address = 0;
		uint32_t len = 0;
		uint32_t value = 0xFF;
		int i = 0;
		char *temp = strtok(cmdStr, ":");
		while (temp)
		{
			if (i == 0)
			{
				address = strtoul(temp, NULL, 16);
			}
			else if (i == 1)
			{
				len = (uint32_t)_atoi64(temp);
			}
			else if (i == 2)
			{
				value = strtoul(temp, NULL, 16);
			}
			i++;
			temp = strtok(NULL, ":");
		}
		if ((cmd == 201 && i == 2) ||
			cmd == 202 && i == 3 ||
			cmd == 203 && i == 2)
		{
			PtrList hex = PTRLIST_INITIALIZER;

			if (inFileNum == 1)
			{
				int r = 0;

				if (inFileType[0] == 1)
				{
					r = bin_readFile(&hex, inFilePath[0], inbinaryAddress[0], false);
					if (r != 0)
					{
						if (r == -1)
							printf("文件%s打开失败\n", inFilePath[0]);
						else if (r == -2)
							printf("文件%s存在重合区域\n", inFilePath[0]);
						else
							printf("文件%s 处理错误%d\n", inFilePath[0], r);
						goto RETURN;
					}
				}
				else if (inFileType[0] == 0)
				{
					r = hex_readFile(&hex, inFilePath[0], false);
					if (r != 0)
					{
						if (r == -1)
							printf("文件<%s>打开失败\n", inFilePath[0]);
						else if (r == -2)
							printf("文件<%s>存在重合区域\n", inFilePath[0]);
						else if (r > 0)
							printf("文件<%s>在行%d错误\n", inFilePath[0], r);
						else
							printf("文件<%s>处理错误%d\n", inFilePath[0], r);
						goto RETURN;
					}
				}

				bool _rb;
				uint8_t *_temp_buf = NULL;

				if (cmd == 201)
				{
					_rb = hex_remove(&hex, address, len);
				}
				else if (cmd == 202)
				{
					_temp_buf = (uint8_t *)malloc(len);
					memset(_temp_buf, value, len);
					_rb = hex_addEx(&hex, address, len, _temp_buf, true);
				}
				else if (cmd == 203) //不用的裁剪掉
				{
					_rb = hex_remove(&hex, 0, address);
					if (_rb)
						_rb = hex_remove(&hex, address + len, UINT32_MAX - (address + len));
				}

				if (_rb)
				{
					if (outFileType == 1)
					{
						int r = bin_writeFile(&hex, outFilePath, binaryOutMode, true);
						if (r != 0)
						{
							if (r == -1)
								printf("输出文件<%s>打开失败\n", outFilePath);
							else if (r == -2)
								printf("输出文件<%s>已存在\n", outFilePath);
							else
								printf("输出文件<%s>处理错误%d\n", outFilePath, r);
						}
					}
					else if (outFileType == 0)
					{
						int r = hex_writeFile(&hex, outFilePath, true);
						if (r != 0)
						{
							if (r == -1)
								printf("输出文件<%s>打开失败\n", outFilePath);
							else if (r == -2)
								printf("输出文件<%s>已存在\n", outFilePath);
							else
								printf("输出文件<%s>处理错误%d\n", outFilePath, r);
						}
					}
				}
				else
				{
					printf("处理\n");
				}

				if (_temp_buf)
				{
					free(_temp_buf);
				}
			}
			else
			{
				printf("只能处理一个文件 当前输入%d个文件\n", inFileNum);
			}

			hex_delete(&hex);
		}
		else
		{
			printf("参数错误\n");
		}
	}
RETURN:
	return 0;
}
