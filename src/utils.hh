#pragma once

#include "def.hh"
#include "random_generator.hh"
#include <regex>

//declaration of neccessary help functions.
extern std::string split_line;

void remakeFTL(SimpleSSD::ConfigReader* &conf, FTL* &p_ftl, PageMapping* &p_pmap, SimpleSSD::DRAM::AbstractDRAM* &p_dram, ConfigInfo* &cfg_info);

void printFTLInfo(PageMapping* p_pmap, const BlockStat& block_stat, const std::string &test_name);

bool pyRun(const std::string file_name, const std::string input, std::string& output);

bool parseStatAnalyzerOut(const std::string& output, StatAnalyzerOut& out);

SimpleSSD::Disk* createTestDisk(SimpleSSD::CompressType compress_type, DiskInitPolicy, uint64_t superpage_cnt);

void outputUTStats(const BlockStat& block_stat, PageMapping* p_pmap);
