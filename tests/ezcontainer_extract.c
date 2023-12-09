#define EZCONTAINER_IMPLEMENTATION
#include "ezcontainer.h"

int main(int argc, const char* argv[]) {
    assert(argc == 2);
    ezContainer *tree = ezContainerRead(argv[1]);

    ezContainerTreeEntry *e = NULL;
    for (int i = 0; i < tree->sizeOfEntries; ++i) {
        e = &tree->entries[i];
        char buf[255];
        sprintf(buf, ".%s", strrchr(e->filePath, '/'));

        unsigned char* data = ezContainerEntryRaw(tree, &e->entry);
        FILE* tmp = fopen(buf, "w");
        fwrite(data, e->entry.fileSize * sizeof(unsigned char), 1, tmp);
        fclose(tmp);
        free(data);
    }
    ezContainerFree(tree);
    return 0;
}
