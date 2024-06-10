include_directories(${CacheSim_SOURCE_DIR}/include)
add_executable(adv_cache main.cpp)
target_sources(adv_cache PUBLIC debug_utilities.cpp L1cache L2cache Victimcache)
