#include <iostream>
#include <fstream>
#include <cstring>
#include "klee/Internal/ADT/KTest.h"

int main(int argc, char** argv) {
    KTest* input = kTest_fromFile(argv[1]);

    for (int i=0; i<input->numObjects; i++) {
        KTestObject *o = &input->objects[i];
        if (strstr(argv[1], "sqlite") != NULL) {
            if (!strcmp(o->name, "stdin")) {
                std::ofstream f;
                f.open(argv[2], std::ios::binary | std::ios::out);
                // std::cout << o->numBytes << std::endl;
                // for (int j=0; j<o->numBytes; j++) {
                //     f.write((char *)&(o->bytes[j]), sizeof(o->bytes[j]));
                //     std::cout << (int)(o->bytes[j]) << std::endl;
                // }
                f.write((char *)&(o->bytes)[0], o->numBytes);
            }
        } else {
            if (!strcmp(o->name, "A-data")) {
                std::ofstream f;
                f.open(argv[2], std::ios::binary | std::ios::out);
                f.write((char *)&(o->bytes)[0], o->numBytes);
            }
        }
    }

    return 0;
}