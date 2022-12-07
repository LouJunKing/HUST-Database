#pragma once

#include <cassert>
#include <climits>
#include <cstdlib>
#include <string>

#include "buffer/buffer_pool_manager.h"
#include "storage/index/generic_key.h"

namespace bustub {

#define MappingType std::pair<KeyType, ValueType>

#define INDEX_TEMPLATE_ARGUMENTS template <typename KeyType, typename ValueType, typename KeyComparator>

/* 定义索引类型
*  
*
* 
*/
enum class IndexPageType { INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE };

/**
 * 内部结点和叶结点均继承自该结点
 *
 * BPlusTreePage存储在Page类的data中，处于Page的首部
 */
class BPlusTreePage {
 public:
  
  bool IsLeafPage() const;
  bool IsRootPage() const;
  void SetPageType(IndexPageType page_type);

  int GetSize() const;
  void SetSize(int size);
  void IncreaseSize(int amount);

  int GetMaxSize() const;
  void SetMaxSize(int max_size);
  int GetMinSize() const;

  page_id_t GetParentPageId() const;
  void SetParentPageId(page_id_t parent_page_id);

  page_id_t GetPageId() const;
  void SetPageId(page_id_t page_id);

  void SetLSN(lsn_t lsn = INVALID_LSN);

 private:
  //页的类型,取值enum IndexPageType
  IndexPageType page_type_ __attribute__((__unused__));
  //日志序列号，用于并发索引
  lsn_t lsn_ __attribute__((__unused__));
  //一个页里的键值对数量
  int size_ __attribute__((__unused__));
  //一个页里的键值对最大数量
  int max_size_ __attribute__((__unused__));
  //父结点的页ID，根节点是没有父节点，可以定义根节点的parent_page_id_=INVALID_PAGE_ID
  //定义于config.h,static constexpr int INVALID_PAGE_ID = -1;
  page_id_t parent_page_id_ __attribute__((__unused__));
  //本结点的id
  page_id_t page_id_ __attribute__((__unused__));
};

}
