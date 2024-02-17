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
    p_pmap->cd_info.offset = 0;
    p_pmap->cd_info.pDisk = createTestDisk(SimpleSSD::CompressType::LZ4, DiskInitPolicy::BYTE_RANDOM, DiskWritePolicy::BYTE_RANDOM, cfg_info->nTotalLogicalPages);
}

void GCChildObject::run(){
}

void GCChildObject::check(GCSharedObject* pgc_sharedobj){

}

void cSigTermHandler(int signum){

}

void pSigUsr1Handler(int signum){

}

void runGCConsistencyTest(){
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
            std::cerr << "Failed to set signal handler." << std::endl;
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
        int wait_time = rand() % 401 + 100; // wait_time 100 - 500ms
        std::this_thread::sleep_for(std::chrono::microseconds(wait_time));
        kill(pid, SIGTERM);
        //wait subprocess finished.
        waitpid(pid, nullptr, 0);
        // Parent process Finished.
        return ;
    }
}