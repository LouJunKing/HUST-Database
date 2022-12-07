/**
 * 该头文件修改无效，仅供阅读参考，不要修改
 */
#pragma once

#include <queue>
#include <string>
#include <vector>

#include "concurrency/transaction.h"
#include "storage/index/index_iterator.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

#define BPLUSTREE_TYPE BPlusTree<KeyType, ValueType, KeyComparator>

/**
 * B+树类声明
 * (1) key不重复，主键唯一
 * (2) 支持插入和删除功能(该关卡仅实现插入)
 * (4) 请实现B+树index_iterator类，供B+树遍历使用
 */
    INDEX_TEMPLATE_ARGUMENTS
    class BPlusTree {

    public:
        explicit BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                           int leaf_max_size = LEAF_PAGE_SIZE, int internal_max_size = INTERNAL_PAGE_SIZE);

        bool IsEmpty() const;

        bool Insert(const KeyType &key, const ValueType &value);

        bool GetValue(const KeyType &key, std::vector<ValueType> *result);

        //index_iterator
        INDEXITERATOR_TYPE Begin();
        INDEXITERATOR_TYPE Begin(const KeyType &key);
        INDEXITERATOR_TYPE End();

        Page *FindLeafPage(const KeyType &key, bool leftMost = false);

    private:
        void StartNewTree(const KeyType &key, const ValueType &value);

        bool InsertIntoLeaf(const KeyType &key, const ValueType &value);

        void InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node);

        template <typename N>
        N *Split(N *node);

        void UpdateRootPageId(int insert_record = 0);


        // member variable
        //索引名称
        std::string index_name_;
        //根节点的PageID
        page_id_t root_page_id_;
        BufferPoolManager *buffer_pool_manager_;
        KeyComparator comparator_;
        //叶结点的最大数目
        int leaf_max_size_;
        //内部结点的最大数目
        int internal_max_size_;
        std::mutex latch_;
    };

}
