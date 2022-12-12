#include <string>
#include <iostream>
#include "common/exception.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/header_page.h"

namespace bustub {
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {}

/*
 * 函数功能：
 *  判断B+树是否为空
 * 建议：
 *  B+树初始化后root_page_id_为INVALID_PAGE_ID
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::IsEmpty() const {
  //之前的page类型均为结点，现在该类型表示整个B+树
  return root_page_id_ == INVALID_PAGE_ID;
}
/*****************************************************************************
 * 查找
 *****************************************************************************/
/*
 * 函数功能：
 *  在B+树中，查找key值对应的记录
 *  如果存在则返回true，并将记录push到result中
 *  如果不存在则返回false
 * 建议：
 *  1.通过调用FindLeafPage(key)函数寻找key值所在叶结点
 *  2.当前叶子结点中不存在该key值，返回false
 *  3.当叶子结点不再使用后，需要及时unpin释放，避免缓冲区内存泄露
 *  4.注意利用
 *   B_PLUS_TREE_LEAF_PAGE_TYPE *leafPage = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE*>
 *                                         (page->GetData());
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result) {
  //latch锁，本次实验不考虑
  std::lock_guard<std::mutex> lock(latch_);
  Page *page = FindLeafPage(key, false);
  if(nullptr == page) {
    return false;
  }
  B_PLUS_TREE_LEAF_PAGE_TYPE *leafPage = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());
  ValueType value;
  if (leafPage->Lookup(key, &value, comparator_)) {
    result->push_back(value);
    buffer_pool_manager_->UnpinPage(leafPage->GetPageId(), false);
    return true;
  }
  buffer_pool_manager_->UnpinPage(leafPage->GetPageId(), false);
  return false;
  }

/*
 * 函数功能：
 *  在B+树中，查找key值应在的叶结点
 *  当leftMost为true时，直接返回最左叶结点（用于遍历输出索引记录）
 * 建议：
 *  1.当B+树空时直接返回空指针
 *  2.通过buffer_pool_manager_->FetchPage(page_id)从磁盘中读入特定Page
 *  3.当特定page不再需要时，用buffer_pool_manager_->UnpinPage(page_id, is_dirty)解锁page，不再占用缓冲区
 *  3.不断向下搜索，直至目标叶子结点
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key, bool leftMost) {
  if(IsEmpty()) {
    return nullptr;
  }
  Page *page = buffer_pool_manager_->FetchPage(root_page_id_);
  BPlusTreePage *pointer = reinterpret_cast<BPlusTreePage *>(page->GetData());
  page_id_t next, cur = root_page_id_;
  while(!pointer->IsLeafPage()) {
    B_PLUS_TREE_INTERNAL_PAGE *internalPage = static_cast<B_PLUS_TREE_INTERNAL_PAGE *>(pointer);
    if(leftMost) {
      next = internalPage->ValueAt(0);
    }
    else {
      next = internalPage->Lookup(key, comparator_);
    }
    buffer_pool_manager_->UnpinPage(cur, false);
    cur = next;
    page = buffer_pool_manager_->FetchPage(cur);
    pointer = reinterpret_cast<BPlusTreePage *>(page->GetData());
  }
  return page;
}

/*****************************************************************************
 * 插入
 *****************************************************************************/
/*
 * 函数功能：
 *  向B+树中插入key以及对应的value，成功返回true，失败则返回false
 * 建议：
 *  1.判断B+树是否为空，为空时调用StartNewTree函数处理
 *  2.若非空，则调用InsertIntoLeaf函数正常插入记录
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value) {
  std::lock_guard<std::mutex> lock(latch_);
  if (IsEmpty()) {
    StartNewTree(key, value);
    return true;
  }
  return InsertIntoLeaf(key, value);
  
  }
/*
 * 函数功能：
 *  当B+树为空时插入记录时，该函数负责生成一个新B+树并初始化相关配置
 * 建议：
 *  1.根结点此时应为叶结点
 *  2.调用UpdateRootPageId(rootPageID)更新header_page文件中的根结点记录 //UpdateRootPageId无需实现
 *  3.及时unpin不再使用的page，避免缓冲区内存泄漏，注意是否为脏页
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  page_id_t page_Id;
  Page *rootPage = buffer_pool_manager_->NewPage(&page_Id);
  B_PLUS_TREE_LEAF_PAGE_TYPE *root = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(rootPage->GetData());
  root->Init(page_Id, INVALID_PAGE_ID, leaf_max_size_);

  root_page_id_ = page_Id;
  UpdateRootPageId(page_Id);
  root->Insert(key, value, comparator_);
  buffer_pool_manager_->UnpinPage(page_Id, true);
  
}

/*
 * 函数功能：
 *  当B+树根结点发生变化时，调用该函数在header_page中对根结点ID进行更新
 *  header_pagez中存放元数据，当B+树根节点发生变化时一定要修改header_page
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
    HeaderPage *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
    if (insert_record != 0) {
        // create a new record<index_name + root_page_id> in header_page
        header_page->InsertRecord(index_name_, root_page_id_);
    } else {
        // update root_page_id in header_page
        header_page->UpdateRecord(index_name_, root_page_id_);
    }
    buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/*
 * 函数功能：
 *  在正确的叶结点插入key以及对应的value，成功返回true，失败返回false
 * 建议：
 *  1.找到key值应在的叶结点
 *  2.key值若已存在，直接返回false(为了便于实现，当前仅支持unique key)
 *  3.插入后需判断结点元素是否超过max_size,并调用Split()和InsertIntoParent()进行后续处理
 *  4.注意unpinPage，避免缓冲区内存泄露
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value) {
  Page *page = FindLeafPage(key);
  if(nullptr == page) {
    return false;
  }
  B_PLUS_TREE_LEAF_PAGE_TYPE *bpage = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());
  ValueType v;
  if(bpage->Lookup(key, &v, comparator_)) {
    buffer_pool_manager_->UnpinPage(bpage->GetPageId(), false);
    return false;
  }
  bpage->Insert(key, value, comparator_);
  //每次插入之后进行判断，是否已经超过最大值，选择直接进行分裂
  //由于MAXSIZE后还留有一个空白结点，这样可以保证插入结点而不会导致数据越界
  //这样就可以先插入，再分裂结点，避免了需要先分裂腾出空间，再插入其中一个的复杂操作
  if(bpage->GetSize() > bpage->GetMaxSize()) {
    B_PLUS_TREE_LEAF_PAGE_TYPE *newLeafPage = Split(bpage);
    InsertIntoParent(bpage, newLeafPage->KeyAt(0), newLeafPage);
  }
  buffer_pool_manager_->UnpinPage(bpage->GetPageId(), false);
  return true;
}
/*
 * 函数功能：
 *  分裂输入的结点(叶结点or内部结点)，返回新生成的结点
 * 建议：
 *  1.生成相同类型的新结点并初始化
 *  2.调用MoveHalfTo()进行结点的分裂
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREE_TYPE::Split(N *node) {
  page_id_t new_page_id;
  Page* page = buffer_pool_manager_->NewPage(&new_page_id);
  N *new_node = reinterpret_cast<N *>(page->GetData());
  if(node->IsLeafPage()) {
    new_node->Init(new_page_id, node->GetParentPageId(), leaf_max_size_);
  }
  else {
    new_node->Init(new_page_id, node->GetParentPageId(), internal_max_size_);
  }
  node->MoveHalfTo(new_node, buffer_pool_manager_);
  return new_node;
}


/*
 * 函数功能：
 *  当生成新结点后，在其父结点中插入正确的索引
 *  @param   old_node      分裂过的原结点
 *  @param   key      新索引的key值
 *  @param   new_node      分裂后生成的新结点
 * 建议：
 *  1.若为根结点分裂，即不存在父点时，应构造新根结点，并调用PopulateNewRoot快速添加对原根结点及分裂结点的索引
 *  2.调用UpdateRootPageId更新rootPageID
 *  3.若为内部结点分裂，在父节点中插入新元素后，若超过max_size,调用Split()和InsertIntoParent()进行
 *  4.更新ParentPageId
 *  5.注意unpinPage，避免缓冲区内存泄露
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node) {
  page_id_t parentId = old_node->GetParentPageId();
  if (parentId == INVALID_PAGE_ID) {
    //此时说明是根结点，无父结点存在，需重新生成根结点
    Page *new_root_page = buffer_pool_manager_->NewPage(&root_page_id_);
    B_PLUS_TREE_INTERNAL_PAGE *new_root = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(new_root_page->GetData());
    new_root->Init(root_page_id_, INVALID_PAGE_ID, internal_max_size_);
    new_root->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(root_page_id_);
    new_node->SetParentPageId(root_page_id_);
    UpdateRootPageId();
    buffer_pool_manager_->UnpinPage(new_root->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
  } else {
    Page *parent_page = buffer_pool_manager_->FetchPage(parentId);
    B_PLUS_TREE_INTERNAL_PAGE *parent_node = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(parent_page->GetData());
    new_node->SetParentPageId(parentId);
    parent_node->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
    if(parent_node->GetSize() > parent_node->GetMaxSize()) {
      B_PLUS_TREE_INTERNAL_PAGE *newLeafPage = Split(parent_node);
      InsertIntoParent(parent_node, newLeafPage->KeyAt(0), newLeafPage);
    }
    buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
  }
}

/*
 * 函数功能：
 *  返回指向B+树首个记录的迭代器
 * 建议：
 *  1.优先实现index_iterator.cpp文件
 *  2.调用FindLeafPage函数找到最左叶子结点，迭代器指向B+树首个记录
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin() {
  KeyType _;
  Page *page = FindLeafPage(_, true);
  B_PLUS_TREE_LEAF_PAGE_TYPE *start_leaf = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());
  return INDEXITERATOR_TYPE(start_leaf, 0, buffer_pool_manager_);
}

/*
 * 函数功能：
 *  返回指向B+树特定key值记录的迭代器
 * 建议：
 *  1.优先实现index_iterator.cpp文件
 *  2.调用FindLeafPage函数找到key值所在叶子结点，迭代器指向该记录(或最近记录)
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin(const KeyType &key) {
  Page *page = FindLeafPage(key, false);
  B_PLUS_TREE_LEAF_PAGE_TYPE *start_leaf = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());
  if (page == nullptr) {
    return INDEXITERATOR_TYPE(start_leaf, 0, buffer_pool_manager_);
  }
  int idx = start_leaf->KeyIndex(key, comparator_);
  return INDEXITERATOR_TYPE(start_leaf, idx, buffer_pool_manager_);
}

/*
 * 函数功能：
 *  返回IsEnd状态的B+树记录迭代器
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::End() { return INDEXITERATOR_TYPE(nullptr, 0, buffer_pool_manager_); }

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
