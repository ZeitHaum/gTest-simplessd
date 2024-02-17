#include "utiterator.hh"

void UTTestIterator::init(uint32_t all_pages){
  all_comptype[0] = SimpleSSD::CompressType::LZ4;
  all_comptype[1] = SimpleSSD::CompressType::LZMA;
  all_dipolicy[0] = DiskInitPolicy::ALL_ZERO;
  all_dipolicy[1] = DiskInitPolicy::BYTE_RANDOM;
  all_dwpolicy[0] = DiskWritePolicy::ZERO;
  all_dwpolicy[1] = DiskWritePolicy::BYTE_RANDOM;
  all_writepages.clear();
  for(uint32_t i = 1; i<= wrpage_split_factor; ++i){
    all_writepages.push_back(all_pages * i / (1.0D * wrpage_split_factor));
  }
  iter_num = 0;
}

UTTestInfo UTTestIterator::getnextTestInfo(){
  UTTestInfo ret;
  uint32_t cp_itnum = iter_num;
  ret.write_pages = all_writepages[iter_num % all_writepages.size()];
  iter_num /= all_writepages.size();
  ret.dwpolicy = all_dwpolicy[iter_num % 2];
  iter_num /= 2;
  ret.dipolicy = all_dipolicy[iter_num % 2];
  iter_num /= 2;
  ret.comptype = all_comptype[iter_num % 2];
  iter_num = cp_itnum + 1;
  return ret;
}

bool UTTestIterator::is_end(){
  return iter_num == 8 * all_writepages.size();
}

std::string UTTestInfo::getTestName(){
  std::string ret = "unit_test_";
  if(comptype == SimpleSSD::CompressType::LZ4){
    ret += "lz4_";
  }
  else if(comptype == SimpleSSD::CompressType::LZMA){
    ret += "lzma_";
  }
  else {
    assert(false && "Not support.");
  }
  if(dipolicy == DiskInitPolicy::ALL_ZERO){
    ret += "Izero_";
  }
  else if(dipolicy == DiskInitPolicy::BYTE_RANDOM){
    ret += "Irandom_";
  }
  else{
    assert(false && "Not support.");
  }
  if(dwpolicy == DiskWritePolicy::ZERO){
    ret += "Wzero_";
  }
  else if(dwpolicy == DiskWritePolicy::BYTE_RANDOM){
    ret += "Wrandom_";
  }
  else{
    assert(false && "Not support.");
  }
  ret += "P" + std::to_string(write_pages);
  return ret;
}
