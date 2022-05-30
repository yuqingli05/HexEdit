/*
 * @Description:
 * @Author: yuqingli
 * @Contact: yuqingli05@outlook.com
 * @Date: 2021-05-29 20:47:03
 * @LastEditTime: 2022-05-28 15:21:24
 * @LastEditors: yuqingli
 */
#ifndef __HEX_H
#define __HEX_H
#include <stdint.h>
#include <stdbool.h>
#include "utils_ptrlist.h"

struct HEX_BLOCK_NODE
{
	uint32_t startAddress; //开始地址
	uint32_t len;		   //固件长度
	uint8_t *buf;		   //固件buf
};
//判断两个hex 链表内部 块是否又重和区域
extern bool hex_isOverlap(PtrList *hexA_list, PtrList *hexB_list);
//删除hex链表，释放所有分配的内存
extern bool hex_delete(PtrList *hex_list);
//删除一个块
extern bool hex_remove(PtrList *hex_list, uint32_t startAddress, uint32_t len);
//向地址添加一个buf
extern bool hex_addEx(PtrList *hex_list, uint32_t startAddress, uint32_t len, uint8_t *buf, bool isCover);
//重合会返回错误
extern bool hex_add(PtrList *hex_list, uint32_t startAddress, uint32_t len, uint8_t *buf);
//将hexB_list 里面的节点 保存的数据全部复制到 hexA_list里面一份
//如果 没检查到重合区域，并且hex_add 失败，hexA_list里面的内容可能会被改写。但是正常正确的hexA_list和hexB_list
//不应该会出现这个情况
extern bool hex_mergeEx(PtrList *hexA_list, PtrList *hexB_list, bool isCover);
extern bool hex_merge(PtrList *hexA_list, PtrList *hexB_list);
//获取flash 总大小
extern uint32_t hex_getLen(PtrList *hex_list);
//获取开始地址
extern uint32_t hex_getStartAddress(PtrList* hex_list);
//获取结束地址
extern uint32_t hex_getEndAddress(PtrList* hex_list);
// 读入一个hex文件到链表
// isCover 是true的时候会覆盖重合区域
// return 0:成功 正值:解析数据错误的行 -1:文件打开失败 -2:hex_list 和文件有重合区域
extern int hex_readFile(PtrList *hex_list, char *FilePath, bool isCover);
// 把一个链表保存为一个hex文件
// isCover 为true的时候 如果文件存在将覆盖
extern int hex_writeFile(PtrList *hex_list, char *FilePath, bool isCover);
// 读入一个二进制文件文件到链表
// startAddress 二进制文件不带有地址，需要手动设置地址
// isCover 是true的时候会覆盖重合区域
// return 0:成功 -1:文件打开失败 -2:hex_list 和文件有重合区域
extern int bin_readFile(PtrList *hex_list, char *FilePath, uint32_t startAddress, bool isCover);
// 把一个链表保存为一个二进制文件
// freeValue 二进制必须是连续的，这个字节是不连续位置的充填字节，负值的时候 保存为多个文件
// isCover 为true的时候 如果文件存在将覆盖
// 注:单个不连续地址最多1Mb大小,超过1mb 将会失败
extern int bin_writeFile(PtrList *hex_list, char *FilePath, int freeValue, bool isCover);
#endif