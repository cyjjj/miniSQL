#include <iostream>
#include "Interpreter.h"
#include "BufferManager.h"

BufferManager buffer_manager;

int main(int argc, const char* argv[]) {
    Interpreter query;
    while (1) {
        query.get_query();
        if(!query.interpret()) break;
    }
    return 0;
}