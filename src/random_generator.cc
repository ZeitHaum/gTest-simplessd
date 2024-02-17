#include "random_generator.hh"

std::random_device rd;  // 随机数种子
std::mt19937 gen(rd()); // 使用 Mersenne Twister 引擎
const uint32_t RANDOM_SEED = 42; // 使用统一的RANDOM_SEED给rand函数。

// 生成 [l, r] 范围内的数据
template <typename T>
T getRandomInt(T l, T r){
    assert(std::is_integral<T>::value);
    std::uniform_int_distribution<T> dis(l, r); 
    return dis(gen);
}

template <typename T>
T getRandomFloating(T l, T r){
    assert(std::__is_floating<T>::__value);
    std::uniform_real_distribution<T> dis(l, r); 
    return dis(gen);
}

void getRandomBitset(SimpleSSD::Bitset& ret){
    for(uint32_t i = 0; i<ret.size(); ++i){
        if(getRandomFloating<float>(0.0f, 1.0f) >= 0.5f){
            ret.set(i);
        }
    }
}

template <typename T>
void getRamdomVector(std::vector<T>& ret, T l, T r){
    for(uint32_t i = 0; i<ret.size(); ++i){
        ret[i] = (T)(getRandomInt(l, r));
    }
}

uint8_t getRandomByte(){
    return rand() % 256;
}

