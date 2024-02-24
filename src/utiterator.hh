#pragma once 

#include "def.hh"
#include <vector>

struct UTTestInfo{
  SimpleSSD::CompressType comptype;
  DiskWritePolicy dwpolicy;
  DiskInitPolicy dipolicy;
  uint32_t write_pages;
  uint8_t* write_data;
  std::string getTestName();
  UTTestInfo();
  UTTestInfo(SimpleSSD::CompressType ctype, DiskWritePolicy dw, DiskInitPolicy di, uint32_t wp, uint8_t* = nullptr);
};

class UTTestIterator{
private:
  SimpleSSD::CompressType all_comptype[2];
  DiskWritePolicy all_dwpolicy[2];
  DiskInitPolicy all_dipolicy[2];
  uint64_t iter_num;
  uint64_t total_count;
  bool enable_di;
public:
  UTTestIterator();
  UTTestIterator(bool en_di);
  std::vector<uint32_t> all_writepages;
  void init(uint32_t all_pages);
  UTTestInfo getnextTestInfo();
  bool is_end();
};

