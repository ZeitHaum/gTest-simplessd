#include "unit_test.hh"

int main(int argc, char **argv) {
    // 初始化 Google Test
    ::testing::InitGoogleTest(&argc, argv);

    std::string s;
    // 解析自定义参数
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--testconfig" && i + 1 < argc) {
            s = argv[i + 1];
            if(s == "simple"){
              test_cfg = TestConfig::SIMPLE;
            }
            else if(s == "actual"){
              test_cfg = TestConfig::ACTUAL;
            }
            else{
              assert(0 && "No such test config options");
            }
            break;
        }
    }

    // 运行测试
    int test_ret =  RUN_ALL_TESTS();
    //do some clear jobs.
    utStatFile.close();
    return test_ret;
}