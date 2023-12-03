#ifndef _ZD_MEMPOOL_H_
#define _ZD_MEMPOOL_H_

#include <stdio.h>

template<int ObjectSize, int NumofObjects = 20>
class ZDMemPool
{
public:
	ZDMemPool() 
		: m_freeNodeHeader(nullptr)
		, m_memBlockHeader(nullptr) 
	{
	}

	~ZDMemPool()
	{
		MemBlock* ptr = nullptr;
		while (m_memBlockHeader)
		{
			ptr = m_memBlockHeader->pNext;
			delete m_memBlockHeader;
			m_memBlockHeader = ptr;
		}
	}

	void* ZD_Malloc()
	{
		// 无空闲节点，申请新内存块
		if (nullptr == m_freeNodeHeader)
		{
			MemBlock* newBlock = new MemBlock;
			newBlock->pNext = nullptr;

			// 设置内存块的第一个节点为空闲节点链表的首节点
			m_freeNodeHeader = &newBlock->data[0];	 
			// 将内存块的其它节点串起来
			for (int i = 1; i < NumofObjects; ++i)
			{
				newBlock->data[i - 1].pNext = &newBlock->data[i];
			}
			newBlock->data[NumofObjects - 1].pNext = nullptr;

			// 首次申请内存块
			if (nullptr == m_memBlockHeader)
			{
				m_memBlockHeader = newBlock;
			}
			else
			{
				// 将新内存块加入到内存块链表
				newBlock->pNext = m_memBlockHeader;
				m_memBlockHeader = newBlock;
			}
		}

		// 返回空节点闲链表的第一个节点
		void* freeNode = m_freeNodeHeader;
		m_freeNodeHeader = m_freeNodeHeader->pNext;
		return freeNode;
	}

	void ZD_Free(void* p)
	{
		FreeNode* pNode = (FreeNode*)p;
		pNode->pNext = m_freeNodeHeader;	//将释放的节点插入空闲节点头部
		m_freeNodeHeader = pNode;
	}

private:
	//空闲节点结构体
	struct FreeNode
	{
		FreeNode* pNext;
		char data[ObjectSize];
	};

	//内存块结构体
	struct MemBlock
	{
		MemBlock* pNext;
		FreeNode data[NumofObjects];
	};

	FreeNode* m_freeNodeHeader = nullptr;
	MemBlock* m_memBlockHeader = nullptr;
};

#endif
