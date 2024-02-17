#pragma once
#include "def.hh"
#include "utils.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <thread>
#include <mutex>
#include <csignal>
#include "assert.h"

class GCSharedObject{
    //No complex object.
    bool disk_content_ckres;
    bool block_stat_ckres;
    bool table_meta_ckres;
    GCSharedObject();
};

class GCChildObject{
public:
    //Use for child process
    SimpleSSD::ConfigReader* conf;
    FTL* p_ftl;
    PageMapping* p_pmap;
    SimpleSSD::DRAM::AbstractDRAM * p_dram;
    ConfigInfo* cfg_info;
    GCChildObject();
    void init();
    void run();
    void check(GCSharedObject* pgc_sharedobj);
};

void cSigTermHandler(int signum);
void pSigUsr1Handler(int signum);
void runGCConsistencyTest();

extern GCChildObject* pgc_childobj;
extern GCSharedObject* pgc_sharedobj;