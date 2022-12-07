#pragma once

#include <utility>
#include <vector>

#include "storage/page/b_plus_tree_page.h"

namespace bustub {

#define B_PLUS_TREE_LEAF_PAGE_TYPE BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>
#define LEAF_PAGE_HEADER_SIZE 28
#define LEAF_PAGE_SIZE ((PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / (sizeof(MappingType))) - 1

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeLeafPage : public BPlusTreePage {
 public:

  void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = LEAF_PAGE_SIZE);
  page_id_t GetNextPageId() const;
  void SetNextPageId(page_id_t next_page_id);
  KeyType KeyAt(int index) const;
  int KeyIndex(const KeyType &key, const KeyComparator &comparator) const;
  const MappingType &GetItem(int index);

  int Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator);
  bool Lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const;
  int RemoveAndDeleteRecord(const KeyType &key, const KeyComparator &comparator);

  void MoveHalfTo(BPlusTreeLeafPage *recipient, __attribute__((unused)) BufferPoolManager *buffer_pool_manager);
  void MoveAllTo(BPlusTreeLeafPage *recipient, BufferPoolManager *buffer_pool_manager);
  void MoveFirstToEndOf(BPlusTreeLeafPage *recipient, BufferPoolManager *buffer_pool_manager);
  void MoveLastToFrontOf(BPlusTreeLeafPage *recipient, BufferPoolManager *buffer_pool_manager);
 private:
  void CopyNFrom(MappingType *items, int size);
  void CopyLastFrom(const MappingType &item);
  void CopyFirstFrom(const MappingType &item,BufferPoolManager *buffer_pool_manager);
  //B+树叶结点依序访问，最后一个叶结点的next_page_id_是INVALID_PAGE_ID
  page_id_t next_page_id_;
  MappingType array_[0];
};
}
