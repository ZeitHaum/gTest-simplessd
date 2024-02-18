#include "consistency.hh"

GCChildObject* pgc_childobj = nullptr;
GCSharedObject* pgc_sharedobj = nullptr;

GCSharedObject::GCSharedObject(){
    //all set to false;
    memset(this, 0, sizeof(GCSharedObject));
}

GCChildObject::GCChildObject(){
    conf = nullptr;
    p_ftl = nullptr;
    p_pmap = nullptr;
    p_dram = nullptr;
    cfg_info = nullptr;
}

void GCChildObject::init(){
    remakeFTL(conf, p_ftl, p_pmap, p_dram, cfg_info);
    p_pmap->resetStatValues();
    //Initialize Disk.
    p_pmap->cd_info = new SimpleSSD::CompressedDiskInfo();
    p_pmap->cd_info->offset = 0;
    p_pmap->cd_info->pDisk = createTestDisk(SimpleSSD::CompressType::LZ4, DiskInitPolicy::ALL_ZERO, cfg_info->nTotalLogicalPages);
}

void GCChildObject::run(){
    while(true){
        //Always Do GC.
        std::vector<uint32_t> list;
        uint64_t beginAt = 0;
        p_pmap->selectVictimBlock(list, beginAt);
        p_pmap->doGarbageCollection(list, beginAt);
        p_pmap->stat.gcCount++;
        p_pmap->stat.reclaimedBlocks += list.size();
    }
}

void check(GCSharedObject* pgc_sharedobj){
    const BlockStat& block_stat = pgc_sharedobj->block_stat;
    const TableConsistencyMeta& table_mata = pgc_sharedobj->table_meta;
    const ConfigInfo cfg_info = pgc_sharedobj->cfg_info;
    //Check Stats.
    if(block_stat.compressUnitCount == 0){
        std::cout<< "warn: GC not Trigged." << "\n";
    }
    assert(block_stat.totalDataLength == ioUnitInPage * ioUnitSize * cfg_info.nPagesToWarmup);
    assert(block_stat.totalUnitCount == ioUnitInPage * cfg_info.nPagesToWarmup);
    //Check Table
    assert(table_mata.size == cfg_info.nTotalLogicalPages);
    assert(table_mata.all_valid);
    assert(table_mata.disklen_consist);
}

void writeShareObject(GCSharedObject* pgc_sharedobj, GCChildObject* pgc_childobj){
    pgc_sharedobj->block_stat = pgc_childobj->p_pmap->calculateBlockStat();
    //calculate table_meta
    pgc_sharedobj->table_meta.size = pgc_childobj->p_pmap->table.size();
    //Iterate table
    auto getTableMata = [&](bool& all_valid, bool& disklen_consist){
        all_valid = true;
        disklen_consist = true;
        PageMapping* p_pmap = pgc_childobj->p_pmap;
        for(auto iter : p_pmap->table){
            for(uint32_t idx = 0; idx < iter.second.size(); ++idx){
                auto& mapping = iter.second.at(idx);
                auto b = p_pmap->blocks.find(mapping.paddr.blockIndex);
                if(b == p_pmap->blocks.end()){
                    all_valid = false;
                    return;
                }
                if(!b->second.isvalid(mapping.paddr.pageIndex, idx, mapping.paddr.compressunitIndex)){
                    all_valid = false;
                    return;
                }
                uint64_t didx = iter.first*ioUnitInPage + idx;
                if(mapping.length != ((SimpleSSD::CompressedDisk*)(p_pmap->cd_info->pDisk))->getCompressedLength(didx)){
                    disklen_consist = false;
                    return;
                }
            }
        }
    };
    getTableMata(pgc_sharedobj->table_meta.all_valid, pgc_sharedobj->table_meta.disklen_consist);
    pgc_sharedobj->cfg_info.nPagesToInvalidate = pgc_childobj->cfg_info->nPagesToInvalidate;
    pgc_sharedobj->cfg_info.nPagesToWarmup = pgc_childobj->cfg_info->nPagesToWarmup;
    pgc_sharedobj->cfg_info.nTotalLogicalPages = pgc_childobj->cfg_info->nTotalLogicalPages;
}

void cSigTermHandler(int signum){
    writeShareObject(pgc_sharedobj, pgc_childobj);
    clear_ptr(pgc_childobj->p_pmap->cd_info);   
    exit(EXIT_SUCCESS);
}

void pSigUsr1Handler(int signum){
    //Do Nothing.
}

void runGCConsistencyTest(){
    signal(SIGUSR1, pSigUsr1Handler);
    pgc_sharedobj = (GCSharedObject*) (mmap(NULL, sizeof(GCSharedObject), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(pgc_sharedobj != MAP_FAILED && "Failed to Map Shared Memory.");
    new (pgc_sharedobj) GCSharedObject();
    pid_t pid = fork();
    if(pid == -1){
        //Failed to create subprocess
        std::cerr << "Failed to fork process." << std::endl;
        return ;
    }
    else if(pid == 0){
        /**
         * Child Process Initialization.
        **/
        pgc_childobj = new GCChildObject();
        pgc_childobj->init();
        struct sigaction sa;
        sa.sa_handler = cSigTermHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGTERM, &sa, nullptr) == -1) {
            std::cout << "Failed to set signal handler." << std::endl;
            return ;
        }
        /**
         * Initialization finished.
        **/
        kill(getppid(), SIGUSR1);
        while(true){
            pgc_childobj->run();      
        }
        exit(EXIT_FAILURE);
    }
    else {
        pause();
        int wait_time = rand() % 101 + 100; // wait_time 100ms - 200ms
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
        kill(pid, SIGTERM);
        //wait subprocess finished.
        waitpid(pid, nullptr, 0);
        // Parent process Finished.
        return ;
    }
}