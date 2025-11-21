#include <chrono>
#include <fstream>
#include <unistd.h>
#include <atomic>
#include <new>
#pragma once

using namespace std;

static atomic<long long> allocated_bytes(0);
static thread_local bool track_alloc = false;

void start_tracking() {
    allocated_bytes = 0;
    track_alloc = true;
}

void stop_tracking() {
    track_alloc = false;
}

long long get_tracked_bytes() {
    return allocated_bytes.load();
}

void* operator new(std::size_t sz) {
    if (track_alloc) allocated_bytes += sz;
    void* ptr = std::malloc(sz);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

// 2. new array (ex: new int[100]) - ESSENCIAL para Matriz Densa
void* operator new[](std::size_t sz) {
    if (track_alloc) allocated_bytes += sz;
    void* ptr = std::malloc(sz);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void operator delete(void* ptr) noexcept {
    free(ptr);
}


struct Cronometro {
    chrono::high_resolution_clock::time_point inicio;

    void comecar() {
        inicio = chrono::high_resolution_clock::now();
    }

    long long fim_ns() {
        auto fim = chrono::high_resolution_clock::now();
        return chrono::duration_cast<chrono::nanoseconds>(fim - inicio).count();
    }
};
