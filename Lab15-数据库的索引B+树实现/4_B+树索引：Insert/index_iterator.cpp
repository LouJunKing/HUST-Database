#include <cassert>
#include "storage/index/index_iterator.h"

namespace bustub {

/*
* 迭代器的作用：
*    方便程序访问每个叶结点中每个键值对元素，其成员变量index指向当前访问
* 的叶结点内的键值对元素的索引号（键值对索引按照键值依序存储）,成员变量leaf_是
* 一个指针，指向当前访问的叶结点。eg.
*    leaf_->[<0, r0>, <1, r1>, <2, r2>, <3, r3>, <4, r4>] 
*                    index         
*/

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(B_PLUS_TREE_LEAF_PAGE_TYPE *leaf, int index, BufferPoolManager *bufferPoolManager)
    : index_(index), leaf_(leaf), bufferPoolManager_(bufferPoolManager) {}
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator() = default;
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator(){
	if (leaf_ != nullptr) {
    bufferPoolManager_->UnpinPage(leaf_->GetPageId(),false);
  }
}
/*
 * 函数功能：
 *  判断迭代器是否已经将叶结点全部迭代
 */
INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::IsEnd() { 
	//有可能处于末尾，也有可能是空
  return (nullptr == leaf_ || index_ >= leaf_->GetSize());

}
/*
 * 函数功能：
 *  返回迭代器在当前页的中指向的具体键值对
 */
INDEX_TEMPLATE_ARGUMENTS
const MappingType &INDEXITERATOR_TYPE::operator*() { 
  return leaf_->GetItem(index_);

}

/*
 * 函数功能：
 *  迭代器移动至下一个记录(实现迭代器顺序遍历B+树记录的功能)，返回当前迭代器
 * 建议：
 *  1.利用NextPageId属性实现跨结点遍历
 *  2.当记录遍历至(NextPageId == INVALID_PAGE_ID),即IsEnd()返回true
 *  3.当叶子结点不再使用后，需要及时unpin释放，避免缓冲区内存泄露
 *  4.当index+1>leaf_->GetSize()时，要通过GetNextPageId（）跳转到下一页
 *  5.如果下一页是INVALID_PAGE_ID，
        只需要将 leaf_ = nullptr;
                 index_ = 0;
                 return *this;
      如果下一页不是INVALID_PAGE_ID           
         bufferPoolManager_->UnpinPage(leaf_->GetPageId(), false);
         Page *page = bufferPoolManager_->FetchPage(next);
         leaf_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page);
         index_ = 0;                
    6.当index+1<leaf_->GetSize()时，   index++,  return *this;
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE &INDEXITERATOR_TYPE::operator++() {
	//该迭代器是叶子节点的迭代器，B+树的叶子节点之间相互连接，以实现顺序遍历的特性
	//故此迭代器应该可以在不同的结点间移动
  if((index_+1) >= leaf_->GetSize()) {
    page_id_t next_page_id = leaf_->GetNextPageId();
    if(next_page_id == INVALID_PAGE_ID) {
      bufferPoolManager_->UnpinPage(leaf_->GetPageId(), false);
      leaf_ = nullptr;
      index_ = 0;
    }
    else {
      bufferPoolManager_->UnpinPage(leaf_->GetPageId(), false);
      Page *page = bufferPoolManager_->FetchPage(next_page_id);
      leaf_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page);
      index_ = 0;
    }
  }
  else {
    index_++;
  }
  return *this;
}

template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
