#include "def.hh"

// global variable assignment;
const std::string actual_config_file = "./simplessd/config/samsung_983dct_1.92tb.cfg";
const std::string simple_config_file = "./unit_test.cfg";
const std::string img_file = "./nvme.img";
uint64_t ioUnitInPage = 16;
uint64_t pageCount = 768;
uint64_t ioUnitSize = 4096;
uint64_t all_pages = 0;
TestConfig test_cfg = TestConfig::SIMPLE;
std::ofstream utStatFile("utstat.txt"); // output ut stats.
uint64_t printstat_ftlcnt = 0;
const uint64_t wrpage_split_factor = 1;
