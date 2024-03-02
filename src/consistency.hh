// #pragma once
// #include "def.hh"
// #include "utils.hh"
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/mman.h>
// #include <thread>
// #include <mutex>
// #include <csignal>
// #include "assert.h"

// struct TableConsistencyMeta
// {
//     uint64_t size;
//     bool all_valid;// if all mapping is valid block.
//     bool disklen_consist;// if all length is match disk.
// };


// class GCSharedObject{
//     //No complex object, check stats, table, disk.
//     BlockStat block_stat;
//     TableConsistencyMeta table_meta;
//     ConfigInfo cfg_info;
//     GCSharedObject();
// };

// class GCChildObject{
// public:
//     //Use for child process
//     SimpleSSD::ConfigReader* conf;
//     FTL* p_ftl;
//     PageMapping* p_pmap;
//     SimpleSSD::DRAM::AbstractDRAM * p_dram;
//     ConfigInfo* cfg_info;
//     GCChildObject();
//     void init();
//     void run();
// };

// void cSigTermHandler(int signum);
// void pSigUsr1Handler(int signum);
// void runGCConsistencyTest();
// void check(GCSharedObject* pgc_sharedobj);
// void writeShareObject(GCSharedObject* pgc_sharedobj, GCChildObject* pgc_childobj);

// extern GCChildObject* pgc_childobj;
// extern GCSharedObject* pgc_sharedobj;