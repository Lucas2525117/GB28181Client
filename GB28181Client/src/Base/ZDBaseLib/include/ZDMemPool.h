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
		// �޿��нڵ㣬�������ڴ��
		if (nullptr == m_freeNodeHeader)
		{
			MemBlock* newBlock = new MemBlock;
			newBlock->pNext = nullptr;

			// �����ڴ��ĵ�һ���ڵ�Ϊ���нڵ�������׽ڵ�
			m_freeNodeHeader = &newBlock->data[0];	 
			// ���ڴ��������ڵ㴮����
			for (int i = 1; i < NumofObjects; ++i)
			{
				newBlock->data[i - 1].pNext = &newBlock->data[i];
			}
			newBlock->data[NumofObjects - 1].pNext = nullptr;

			// �״������ڴ��
			if (nullptr == m_memBlockHeader)
			{
				m_memBlockHeader = newBlock;
			}
			else
			{
				// �����ڴ����뵽�ڴ������
				newBlock->pNext = m_memBlockHeader;
				m_memBlockHeader = newBlock;
			}
		}

		// ���ؿսڵ�������ĵ�һ���ڵ�
		void* freeNode = m_freeNodeHeader;
		m_freeNodeHeader = m_freeNodeHeader->pNext;
		return freeNode;
	}

	void ZD_Free(void* p)
	{
		FreeNode* pNode = (FreeNode*)p;
		pNode->pNext = m_freeNodeHeader;	//���ͷŵĽڵ������нڵ�ͷ��
		m_freeNodeHeader = pNode;
	}

private:
	//���нڵ�ṹ��
	struct FreeNode
	{
		FreeNode* pNext;
		char data[ObjectSize];
	};

	//�ڴ��ṹ��
	struct MemBlock
	{
		MemBlock* pNext;
		FreeNode data[NumofObjects];
	};

	FreeNode* m_freeNodeHeader = nullptr;
	MemBlock* m_memBlockHeader = nullptr;
};

#endif
