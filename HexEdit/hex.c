/*
 * @Description:
 * @Author: yuqingli
 * @Contact: yuqingli05@outlook.com
 * @Date: 2021-05-29 20:47:03
 * @LastEditTime: 2022-05-30 15:00:20
 * @LastEditors: yuqingli
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "utils_ptrlist.h"

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

struct HEX_BLOCK_NODE
{
	uint32_t startAddress; //开始地址
	uint32_t len;		   //固件长度
	uint8_t *buf;		   //固件buf
};

static int _nibble_value(char nibble)
{
	nibble = tolower(nibble);
	if ('a' <= nibble && nibble <= 'f')
		return nibble - 'a' + 10;
	else if ('0' <= nibble && nibble <= '9')
		return nibble - '0';
	return -1;
}
#define BYTE2HEXNS(bytes, len) _byte2hexStr(bytes, len, NULL, 0, 0)
static const char *_byte2hexStr(const unsigned char *byte, int len, char *user_buf, int outsize, int bSpace)
{
	static char outbuf[4096];
	char *chout = user_buf ? user_buf : outbuf;
	int size = user_buf ? outsize : 4096;
	int nibble;
	int unit = bSpace ? 3 : 2;
	if (len == 0 || size <= unit)
	{
		*(chout) = '\0';
	}
	else
	{
		for (; len-- && size > unit; byte++)
		{
			nibble = ((*byte & 0xf0) >> 4) & 0x0f;
			*(chout++) = (nibble >= 10 ? (nibble - 10) + 'A' : nibble + '0');
			nibble = (*byte) & 0x0f;
			*(chout++) = (nibble >= 10 ? (nibble - 10) + 'A' : nibble + '0');
			if (bSpace)
				*(chout++) = ' ';
			size -= unit;
		}
		if (bSpace)
			*(chout - 1) = '\0';
		else
			*(chout) = '\0';
	}
	return user_buf ? user_buf : outbuf;
}
// hex字符串 转 字节数组
static int _hexStr2byte(const char *strhex, unsigned char *byte, int size)
{
	const char *ptr = strhex;
	int len = 0;
	unsigned char value;
	// skip leading space
	while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	while (*ptr && len < size)
	{
		if (!isxdigit(*ptr))
			return -1;
		// first hexidecimal character
		value = _nibble_value(*(ptr++));
		// second hexidecimal character
		if (*ptr == ' ' || *ptr == '\t') // space is encounted, single character value
			byte[len++] = value;
		else if (isxdigit(*ptr))
		{
			byte[len++] = (value << 4) + _nibble_value(*(ptr++));
		}
		else
			return -1;
		// skip spaces between character
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
	}
	return len;
}
static uint8_t CheckSum(uint8_t *buf, int len) // buf为数组，len为数组长度
{
	uint8_t ret = 0;
	for (int i = 0; i < len; i++)
	{
		ret += *(buf++);
	}
	return (0 - ret) & 0xFF;
}
//判断两个块是否重叠
static bool block_isOverlap(uint32_t startAddress1, uint32_t len1, uint32_t startAddress2, uint32_t len2)
{
	return (startAddress1 < startAddress2 + len2 && startAddress1 + len1 > startAddress2);
}
//判断两个hex 链表内部 块是否有重和区域
static bool hex_isOverlap(PtrList* hexA_list, PtrList* hexB_list)
{
	POSITION nodeA = hexA_list->head;
	POSITION nodeB = hexB_list->head;
	while (nodeA != NULL && nodeB != NULL)
	{
		struct HEX_BLOCK_NODE* blockA = PtrNode_get(nodeA);
		struct HEX_BLOCK_NODE* blockB = PtrNode_get(nodeB);
		if (block_isOverlap(blockA->startAddress, blockA->len, blockB->startAddress, blockB->len))
		{
			return true;
		}
		else if (blockB->startAddress >= blockA->startAddress + blockA->len)
		{
			nodeA = nodeA->next;
		}
		else
		{
			nodeB = nodeB->next;
		}
	}
	return false;
}
//删除hex链表，释放所有分配的内存
bool hex_delete(PtrList *hex_list)
{
	//删除固件区域
	for (POSITION node = hex_list->head; node != NULL; node = node->next)
	{
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);
		free(block->buf);
	}
	//删除链表和指针
	PtrList_delete_all(hex_list);
	return true;
}
//删除一个块
bool hex_remove(PtrList *hex_list, uint32_t startAddress, uint32_t len)
{
	if (len <= 0)
		return true;

	POSITION node = hex_list->head;
	while (node != NULL)
	{
		POSITION node_next = node->next;
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);
		if (block && block_isOverlap(startAddress, len, block->startAddress, block->len)) //和本块相交
		{
			uint32_t _startAddress; //删除块地址
			uint32_t _len;			//删除块长度

			if (startAddress > block->startAddress)
				_startAddress = startAddress;
			else
				_startAddress = block->startAddress;

			if (startAddress + len < block->startAddress + block->len)
				_len = startAddress + len - _startAddress;
			else
				_len = block->startAddress + block->len - _startAddress;

			if (block->len == _len && block->startAddress == _startAddress)
			{
				//全部删除
				free(block->buf);
				PtrList_delete(hex_list, node);
			}
			else if (block->startAddress != _startAddress && block->startAddress + block->len != _startAddress + _len)
			{
				//删除的是中间值，需要拆分成两个块
				//把后面块拆出来单独保存，然后把本块缩小
				uint8_t *buf_temp = malloc((block->startAddress + block->len) - (_startAddress + _len));
				if (buf_temp == NULL)
				{
					return false;
				}
				struct HEX_BLOCK_NODE *block_temp = malloc(sizeof(struct HEX_BLOCK_NODE));
				if (block_temp == NULL)
				{
					free(buf_temp);
					return false;
				}
				block_temp->buf = buf_temp;
				block_temp->startAddress = _startAddress + _len;
				block_temp->len = (block->startAddress + block->len) - (_startAddress + _len);
				memcpy(block_temp->buf, &block->buf[block_temp->startAddress - block->startAddress], block_temp->len);
				PtrList_insert_after(hex_list, node, block_temp);

				block->len = _startAddress - block->startAddress;
				buf_temp = realloc(block->buf, block->len);
				if (buf_temp)
				{
					block->buf = buf_temp;
				}
			}
			else if (block->startAddress == _startAddress && block->startAddress + block->len != _startAddress + _len)
			{
				//删除的是前面段
				memmove(block->buf, &block->buf[_len], block->len - _len);
				block->len -= _len;
				block->startAddress += _len;

				uint8_t *buf_temp = realloc(block->buf, block->len);
				if (buf_temp)
					block->buf = buf_temp;
			}
			else if (block->startAddress != _startAddress && block->startAddress + block->len == _startAddress + _len)
			{
				//删除尾巴，直接缩小
				block->len = _startAddress - block->startAddress;
				uint8_t *block_temp = realloc(block->buf, block->len);
				if (block_temp)
				{
					block->buf = block_temp;
				}
			}
		}
		node = node_next;
	}

	return true;
}
//向地址添加一个buf
bool hex_addEx(PtrList *hex_list, uint32_t startAddress, uint32_t len, uint8_t *buf, bool isCover)
{
	if (len <= 0)
		return true;

	//搜索重复地址，并把原先的重复地址删除，保证下面添加操作没有重复地址
	if (isCover)
	{
		hex_remove(hex_list, startAddress, len);
	}

	for (POSITION node = hex_list->head; node != NULL; node = node->next)
	{
		POSITION node_next = node->next;
		struct HEX_BLOCK_NODE *next_block = NULL;
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);

		if (node_next)
			next_block = PtrNode_get(node_next);

		if (!block)
		{
		}
		else if ((block && block_isOverlap(startAddress, len, block->startAddress, block->len)) || //和本块相交
				 (next_block && block_isOverlap(startAddress, len, next_block->startAddress, next_block->len)))
		{
			//地址重合 不能继续添加
			return false;
		}
		else if (block->startAddress + block->len == startAddress) //和本块尾相接
		{
			uint8_t *buf_temp = realloc(block->buf, (size_t)block->len + (size_t)len);
			if (buf_temp == NULL)
			{
				return false;
			}
			block->buf = buf_temp;
			memcpy(&block->buf[block->len], buf, len);
			block->len += len;

			//检测和下一个块是否头相接
			if (node_next && next_block)
			{
				//收尾相接开始合并
				if (block->startAddress + block->len ==
					next_block->startAddress)
				{
					uint8_t *buf_temp = realloc(block->buf, (size_t)block->len + (size_t)next_block->len);
					if (buf_temp == NULL)
					{
						return false;
					}
					block->buf = buf_temp;
					memcpy(&block->buf[block->len], next_block->buf, next_block->len);
					block->len += next_block->len;
					free(next_block->buf);
					PtrList_delete(hex_list, node_next);
				}
			}
			return true;
		}
		else if (block->startAddress == startAddress + len) //和本块头相接
		{
			uint8_t *buf_temp = realloc(block->buf, (size_t)block->len + (size_t)len);
			if (buf_temp == NULL)
			{
				return false;
			}
			block->buf = buf_temp;
			memmove(&block->buf[len], &block->buf[0], block->len); //移动固件位置，把前面len字节留给新加的固件
			memcpy(block->buf, buf, len);
			block->len += len;
			block->startAddress = block->startAddress - len;

			//这里和尾部相加不一样，不用检查前一个是否相连，因为和上一个尾部相连 一定是在上一个尾部添加的
			return true;
		}
		else if (block->startAddress >= startAddress + len)
		{
			//添加一个新的
			uint8_t *buf_temp = malloc(len);
			if (buf_temp == NULL)
			{
				return false;
			}
			struct HEX_BLOCK_NODE *block_temp = malloc(sizeof(struct HEX_BLOCK_NODE));
			if (block_temp == NULL)
			{
				free(buf_temp);
				return false;
			}
			block_temp->buf = buf_temp;
			memcpy(block_temp->buf, buf, len);
			block_temp->startAddress = startAddress;
			block_temp->len = len;
			PtrList_insert_before(hex_list, node, block_temp);
			return true;
		}
	}

	//搜索到结束都没添加，直接加到尾部
	{
		uint8_t *buf_temp = malloc(len);
		if (buf_temp == NULL)
		{
			return false;
		}
		struct HEX_BLOCK_NODE *block_temp = malloc(sizeof(struct HEX_BLOCK_NODE));
		if (block_temp == NULL)
		{
			free(buf_temp);
			return false;
		}
		block_temp->buf = buf_temp;
		memcpy(block_temp->buf, buf, len);
		block_temp->startAddress = startAddress;
		block_temp->len = len;
		PtrList_append(hex_list, block_temp);
	}
	return true;
}
//重合会返回错误
bool hex_add(PtrList *hex_list, uint32_t startAddress, uint32_t len, uint8_t *buf)
{
	return hex_addEx(hex_list, startAddress, len, buf, false);
}

//将hexB_list 里面的节点 保存的数据全部复制到 hexA_list里面一份
//如果 没检查到重合区域，并且hex_add 失败，hexA_list里面的内容可能会被改写。但是正常正确的hexA_list和hexB_list
//不应该会出现这个情况
bool hex_mergeEx(PtrList *hexA_list, PtrList *hexB_list, bool isCover)
{
	//有重合区直接返回
	if (!isCover && hex_isOverlap(hexA_list, hexB_list))
		return false;

	for (POSITION node = hexB_list->head; node != NULL; node = node->next)
	{
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);
		if (!hex_addEx(hexA_list, block->startAddress, block->len, block->buf, isCover))
		{
			return false;
		}
	}
	return true;
}
bool hex_merge(PtrList *hexA_list, PtrList *hexB_list)
{
	return hex_mergeEx(hexA_list, hexB_list, false);
}
//获取flash 总大小，不包括未设置区域
uint32_t hex_getLen(PtrList *hex_list)
{
	uint32_t len = 0;
	for (POSITION node = hex_list->head; node != NULL; node = node->next)
	{
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);
		len += block->len;
	}
	return len;
}
//获取开始地址
uint32_t hex_getStartAddress(PtrList *hex_list)
{
	POSITION node = hex_list->head;
	if (node)
	{
		struct HEX_BLOCK_NODE* block = PtrNode_get(node);
		return block->startAddress;
	}
	else
	{
		return 0;
	}
}
//获取结束地址
uint32_t hex_getEndAddress(PtrList *hex_list)
{
	POSITION node = hex_list->rear;
	if (node)
	{
		struct HEX_BLOCK_NODE* block = PtrNode_get(node);
		return block->startAddress + block->len;
	}
	else
	{
		return 0;
	}
}
// 读入一个hex文件到链表
// isCover 是true的时候会覆盖重合区域
// return 0:成功 正值:解析数据错误的行 -1:文件打开失败 -2:hex_list 和文件有重合区域
int hex_readFile(PtrList *hex_list, char *FilePath, bool isCover)
{
	PtrList hex_temp_list = PTRLIST_INITIALIZER;
	char s_hex[1024];
	uint8_t byte_buf[512];

	int line = 0;
	FILE *fp;

	uint32_t OffsetAddress = 0;	 // 04:扩展线性地址
	uint32_t _OffsetAddress = 0; // 02:扩展段地址

	if ((fp = fopen(FilePath, "r")) == NULL)
	{
		return -1;
	}
	while (fgets(s_hex, sizeof(s_hex), fp) != NULL)
	{
		line++;
		int sLen = strlen(s_hex);
		//清楚后面的换行等不可打印字符串
		for (int i = sLen - 1; i > 0; i--)
		{
			if (isprint(s_hex[i]))
				break;
			else
				s_hex[i] = '\0';
		}
		if (s_hex[0] != ':') //第一个字符必须是：
			goto ERROR_LINE;

		int byteLen = _hexStr2byte(&s_hex[1], byte_buf, sizeof(byte_buf));
		if (byteLen <= 0)
			goto ERROR_LINE;

		if (CheckSum(byte_buf, byteLen - 1) != byte_buf[byteLen - 1])
			goto ERROR_LINE;

		if (byteLen != byte_buf[0] + 5)
			goto ERROR_LINE;

		int hex_len = byte_buf[0];
		uint32_t hex_address = OffsetAddress + _OffsetAddress + ((byte_buf[1] << 8) | byte_buf[2]);
		int cmd = byte_buf[3];

		switch (cmd)
		{
		case 0x00:
			if (!hex_addEx(&hex_temp_list, hex_address, hex_len, &byte_buf[4], isCover))
				goto ERROR_LINE;
			break;
		case 0x01:
			goto SUCCSEE;
			break;
		case 0x02:
			if (hex_len != 2)
				goto ERROR_LINE;
			_OffsetAddress = ((byte_buf[4] << 8) | byte_buf[5]) << 4;
			break;
		case 0x04:
			if (hex_len != 2)
				goto ERROR_LINE;
			OffsetAddress = ((byte_buf[4] << 8) | byte_buf[5]) << 16;
			break;
		default:
			break;
		}

		// printf("%4d %s", line,s_hex);
	}

SUCCSEE:
	fclose(fp);

	if (hex_merge(&hex_temp_list, hex_list)) //这里大多数 hex_list是空的，所以把 hex_list合并到  hex_temp_list
	{
		hex_delete(hex_list);
		*hex_list = hex_temp_list;
		return 0;
	}
	else
	{
		hex_delete(&hex_temp_list);
		return -2;
	}

ERROR_LINE: //解析行错误
	hex_delete(&hex_temp_list);
	fclose(fp);
	return line;
}
// 把一个链表保存为一个hex文件
// isCover 为true的时候 如果文件存在将覆盖
int hex_writeFile(PtrList *hex_list, char *FilePath, bool isCover)
{
	if (!isCover) // 如果存在返回失败
	{
		FILE *_fp;
		_fp = fopen(FilePath, "r");
		if (_fp != NULL)
		{
			fclose(_fp);
			return -1;
		}
	}

	FILE *fp;
	fp = fopen(FilePath, "w");
	if (fp == NULL)
		return -1;

	uint32_t OffsetAddress = -1; // 04:扩展线性地址

	for (POSITION node = hex_list->head; node != NULL; node = node->next)
	{
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);
		uint8_t *hexBuf = block->buf;
		uint32_t hexBufLen = block->len;
		uint32_t hex_address = block->startAddress;
		while (hexBufLen > 0)
		{
			if (hex_address >> 16 != OffsetAddress || OffsetAddress == -1)
			{
				OffsetAddress = hex_address >> 16;
				uint8_t buf[7] = {0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00};
				buf[4] = (uint8_t)(OffsetAddress >> 8);
				buf[5] = (uint8_t)OffsetAddress;
				buf[6] = CheckSum(buf, 6);
				fprintf(fp, ":%s\n", BYTE2HEXNS(buf, 7));
			}
			if (hexBufLen >= 16)
			{
				uint8_t buf[32];
				buf[0] = 16;
				buf[1] = (uint8_t)(hex_address >> 8);
				buf[2] = (uint8_t)hex_address;
				buf[3] = 0x00;
				memcpy(&buf[4], hexBuf, 16);
				buf[20] = CheckSum(buf, 16 + 4);
				fprintf(fp, ":%s\n", BYTE2HEXNS(buf, 16 + 5));

				hex_address += 16;
				hexBuf += 16;
				hexBufLen -= 16;
			}
			else
			{
				uint8_t buf[32];
				buf[0] = hexBufLen;
				buf[1] = (uint8_t)(hex_address >> 8);
				buf[2] = (uint8_t)hex_address;
				buf[3] = 0x00;
				memcpy(&buf[4], hexBuf, hexBufLen);
				buf[hexBufLen + 4] = CheckSum(buf, hexBufLen + 4);
				fprintf(fp, ":%s\n", BYTE2HEXNS(buf, hexBufLen + 5));

				hex_address += hexBufLen;
				hexBuf += hexBufLen;
				hexBufLen -= hexBufLen;
			}
		}
	}
	fprintf(fp, ":%s\n", "00000001FF"); // hex结尾
	fclose(fp);
	return 0;
}
// 读入一个二进制文件文件到链表
// startAddress 二进制文件不带有地址，需要手动设置地址。如果等于 UINT32_MAX 直接 拼接到后面
// isCover 是true的时候会覆盖重合区域
// return 0:成功 -1:文件打开失败 -2:hex_list 和文件有重合区域
int bin_readFile(PtrList *hex_list, char *FilePath, uint32_t startAddress, bool isCover)
{
	FILE *fp;

	if ((fp = fopen(FilePath, "rb")) == NULL)
	{
		return -1;
	}

	fseek(fp, 0, SEEK_END); //定位文件指针到文件尾。
	long size = ftell(fp);	//获取文件指针偏移量，即文件大小。
	fseek(fp, 0, SEEK_SET); //定位文件指针到文件头。

	uint8_t *byte_buf = malloc(size);
	if (!byte_buf)
	{
		fclose(fp);
		return -2;
	}
	size = fread(byte_buf, 1, size, fp);

	fclose(fp);

	if (startAddress == UINT32_MAX)
	{
		startAddress = hex_getEndAddress(hex_list);
	}
	int r = hex_addEx(hex_list, startAddress, size, byte_buf, isCover);
	free(byte_buf);
	return r ? 0 : -2;
}
// 把一个链表保存为一个二进制文件
// freeValue 二进制必须是连续的，这个字节是不连续位置的充填字节，负值的时候 保存为多个文件
// isCover 为true的时候 如果文件存在将覆盖
// 注:单个不连续地址最多1Mb大小,超过1mb 将会失败
int bin_writeFile(PtrList *hex_list, char *FilePath, int freeValue, bool isCover)
{
	if (!isCover) // 如果存在返回失败
	{
		FILE *_fp;
		_fp = fopen(FilePath, "r");
		if (_fp != NULL)
		{
			fclose(_fp);
			return -1;
		}
	}

	char path_buffer[300];
	char drive[300];
	char dir[300];
	char fname[300];
	char ext[300];

	_splitpath(FilePath, drive, dir, fname, ext);

	FILE *fp = NULL;

	if (freeValue >= 0) //不分段存储
	{
		if ((fp = fopen(FilePath, "wb")) == NULL)
		{
			return -1;
		}
	}

	uint32_t addressEnd_pre = 0; //前一个结束地址

	for (POSITION node = hex_list->head; node != NULL; node = node->next)
	{
		struct HEX_BLOCK_NODE *block = PtrNode_get(node);
		uint8_t *hexBuf = block->buf;
		uint32_t hexBufLen = block->len;
		uint32_t hex_address = block->startAddress;

		if (addressEnd_pre == 0)
		{
			addressEnd_pre = hex_address;
		}

		if (freeValue < 0)
		{
			//分段存储,重新打开文件
			char _fname[300];
			sprintf(_fname, "%s%08X", fname, hex_address);
			_makepath(path_buffer, drive, dir, _fname, ext);
			if (fp)
			{
				fclose(fp);
				fp = NULL;
			}
			if ((fp = fopen(path_buffer, "wb")) == NULL)
			{
				return -1;
			}
		}

		//
		if ((hex_address - addressEnd_pre) > 16 * 1024 * 1024) //空闲太长了,返回失败
		{
			goto _ERROR;
		}

		if (freeValue >= 0 && (hex_address - addressEnd_pre) > 0)
		{
			uint8_t *buf_temp = malloc(hex_address - addressEnd_pre);
			if (!buf_temp)
				goto _ERROR;
			memset(buf_temp, freeValue, hex_address - addressEnd_pre);
			fwrite(buf_temp, 1, hex_address - addressEnd_pre, fp);
			free(buf_temp);
		}
		fwrite(hexBuf, 1, hexBufLen, fp);
		addressEnd_pre = hex_address + hexBufLen;
	}

	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
	return 0;

_ERROR:
	if (fp) //不分段存储
	{
		fclose(fp);
		fp = NULL;
	}
	return -2;
}

#if 0

int main()
{
	PtrList hexFile_list = PTRLIST_INITIALIZER;
	PtrList hexA_list = PTRLIST_INITIALIZER;
	PtrList hex_list = PTRLIST_INITIALIZER;
	uint8_t buf1[64] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	uint8_t buf2[64] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,0x33};
	uint8_t buf3[64] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};



	hex_readFile(&hexFile_list, "C:/Users/yuqingli/Documents/a.hex",false);
	hex_readFile(&hexFile_list, "C:/Users/yuqingli/Documents/b.hex", false);
	hex_writeFile(&hexFile_list, "C:/Users/yuqingli/Documents/c.hex", true);
	hex_delete(&hexFile_list);
	hex_readFile(&hexFile_list, "C:/Users/yuqingli/Documents/rtt/c.hex", false);
	bin_writeFile(&hexFile_list, "C:/Users/yuqingli/Documents/rtt/c_out.bin", -1,true);
	bin_writeFile(&hexFile_list, "C:/Users/yuqingli/Documents/rtt/c_outFF.bin",0xFF, true);



	hex_delete(&hexFile_list);
	hex_delete(&hexA_list);
	hex_delete(&hex_list);
	return 0;
}
#endif
