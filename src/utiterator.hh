#pragma once 

#include "def.hh"
#include <vector>

struct UTTestInfo{
  SimpleSSD::CompressType comptype;
  DiskWritePolicy dwpolicy;
  DiskInitPolicy dipolicy;
  uint32_t write_pages;
  std::string getTestName();
};

class UTTestIterator{
private:
  SimpleSSD::CompressType all_comptype[2];
  DiskWritePolicy all_dwpolicy[2];
  DiskInitPolicy all_dipolicy[2];
  uint64_t iter_num;
public:
  std::vector<uint32_t> all_writepages;
  void init(uint32_t all_pages);
  UTTestInfo getnextTestInfo();
  bool is_end();
};

