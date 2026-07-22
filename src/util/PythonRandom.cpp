#include "placer/util/PythonRandom.hpp"
#include <stdexcept>

namespace placer
{
namespace { constexpr int N = 624; constexpr int M = 397; constexpr std::uint32_t MATRIX_A = 0x9908b0dfU; constexpr std::uint32_t UPPER_MASK = 0x80000000U; constexpr std::uint32_t LOWER_MASK = 0x7fffffffU; }
PythonRandom::PythonRandom(std::uint64_t seed) { std::vector<std::uint32_t> key{static_cast<std::uint32_t>(seed & 0xffffffffULL)}; const auto high=static_cast<std::uint32_t>(seed>>32U); if(high!=0U) key.push_back(high); initByArray(key); }
void PythonRandom::initGenRand(std::uint32_t seed){ state_[0]=seed; for(index_=1; index_<N; ++index_) state_[index_]=1812433253U*(state_[index_-1]^(state_[index_-1]>>30U))+static_cast<std::uint32_t>(index_); }
void PythonRandom::initByArray(const std::vector<std::uint32_t>& key){ initGenRand(19650218U); int i=1,j=0,k=N>static_cast<int>(key.size())?N:static_cast<int>(key.size()); for(;k;--k){ state_[i]=(state_[i]^((state_[i-1]^(state_[i-1]>>30U))*1664525U))+key[static_cast<std::size_t>(j)]+static_cast<std::uint32_t>(j); if(++i>=N){state_[0]=state_[N-1]; i=1;} if(++j>=static_cast<int>(key.size())) j=0;} for(k=N-1;k;--k){ state_[i]=(state_[i]^((state_[i-1]^(state_[i-1]>>30U))*1566083941U))-static_cast<std::uint32_t>(i); if(++i>=N){state_[0]=state_[N-1]; i=1;} } state_[0]=0x80000000U; index_=N; }
std::uint32_t PythonRandom::getRandBits32(){ static constexpr std::uint32_t mag01[2]={0U,MATRIX_A}; if(index_>=N){ int kk=0; for(;kk<N-M;++kk){auto y=(state_[kk]&UPPER_MASK)|(state_[kk+1]&LOWER_MASK); state_[kk]=state_[kk+M]^(y>>1U)^mag01[y&1U];} for(;kk<N-1;++kk){auto y=(state_[kk]&UPPER_MASK)|(state_[kk+1]&LOWER_MASK); state_[kk]=state_[kk+(M-N)]^(y>>1U)^mag01[y&1U];} auto y=(state_[N-1]&UPPER_MASK)|(state_[0]&LOWER_MASK); state_[N-1]=state_[M-1]^(y>>1U)^mag01[y&1U]; index_=0;} auto y=state_[index_++]; y^=y>>11U; y^=(y<<7U)&0x9d2c5680U; y^=(y<<15U)&0xefc60000U; y^=y>>18U; return y; }
std::uint64_t PythonRandom::getRandBits(int bits){ if(bits<0||bits>64) throw std::runtime_error("invalid getRandBits width"); if(bits==0) return 0; std::uint64_t value=0; int rem=bits; while(rem>=32){ value=(value<<32U)|getRandBits32(); rem-=32;} if(rem>0) value=(value<<static_cast<unsigned>(rem))|(getRandBits32()>>static_cast<unsigned>(32-rem)); return value; }
std::size_t PythonRandom::randBelow(std::size_t n){ if(n==0) throw std::runtime_error("randBelow requires n > 0"); int bits=0; for(std::size_t v=n; v; v>>=1U) ++bits; auto r=getRandBits(bits); while(r>=n) r=getRandBits(bits); return static_cast<std::size_t>(r); }
}
