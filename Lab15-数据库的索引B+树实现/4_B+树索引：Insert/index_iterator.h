#pragma once
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
 public:
  IndexIterator();
  ~IndexIterator();
  IndexIterator(B_PLUS_TREE_LEAF_PAGE_TYPE *leaf, int index, BufferPoolManager *bufferPoolManager);
  bool IsEnd();

  const MappingType &operator*();

  IndexIterator &operator++();


 private:
  int index_;
  B_PLUS_TREE_LEAF_PAGE_TYPE *leaf_;
  BufferPoolManager *bufferPoolManager_;
};

}
