/**
 * author: ZeitHaum
 * Project: gTest-SimpleSSD
 * **/
#pragma once
#include <stdint.h>
#include <string>
#include <fstream>
// include third-party files.
#include "ftl/page_mapping.hh"
#include "dram/simple.hh"
#include "gtest/gtest.h"

using namespace SimpleSSD::FTL;

//declareation of micro defines.
//define region
#define clear_ptr(ptr) if(ptr){ \
  delete ptr; \
  ptr = nullptr;\
} 
#define is_ratio(x) ((x) >= 0 && (x) <= 1)
#define get_percent(x, y) ((double)(x * 100) / (double)(y))

//declaration of neccessary structures.
struct ConfigInfo{
  uint64_t nTotalLogicalPages;
  uint64_t nPagesToWarmup;
  uint64_t nPagesToInvalidate;
};

struct StatAnalyzerOut{
  double r_c;
  double f_c;
  double r_f;
};

enum class DiskInitPolicy{
  ALL_ZERO, BYTE_RANDOM
};

enum class DiskWritePolicy{
  ZERO, BYTE_RANDOM, CUSTOM, 
};

enum class TestConfig{
    SIMPLE, ACTUAL
};

// global variables' defination
extern const std::string actual_config_file;
extern const std::string simple_config_file; 
extern const std::string img_file; 
extern uint64_t ioUnitInPage; 
extern uint64_t pageCount; 
extern uint64_t ioUnitSize; 
extern uint64_t all_pages;
extern TestConfig test_cfg; 
extern std::ofstream utStatFile;
extern uint64_t printstat_ftlcnt;
extern const uint64_t wrpage_split_factor;
