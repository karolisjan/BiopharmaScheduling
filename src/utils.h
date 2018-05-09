#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdlib>
#include <algorithm>
#include <vector>

namespace utils
{
    inline int random_int(int min, int max)
    {
        return rand() % (max - min) + min;
    }

    inline double random()
    {
        return rand() / (RAND_MAX + 1.0);
    }
    
    template<typename T>
    inline void shuffle(std::vector<T>& v)
    {
        size_t i, j;
        
        for (i = 0; i < v.size(); ++i) {
            j = random_int(i, v.size());
            std::swap(v[j], v[i]);
        }
    }
}

#endif