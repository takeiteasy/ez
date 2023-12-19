#define EZCONTAINER_IMPLEMENTATION
#include "ezcontainer.h"

int main(int argc, const char* argv[]) {
    const char** test_files = malloc(3 * sizeof(const char*));
    test_files[0] = "tests/res/test.png";
    test_files[1] = "tests/res/test2.png";
    test_files[2] = "tests/res/test3.png";
    ezContainerWrite("build/test.cf", 3, test_files);
    free(test_files);
    
    ezContainer *tree = ezContainerRead("build/test.bcf");
    ezContainerEntry *entry = ezContainerFind(tree, "tests/res/test2.png");
    unsigned char *data = ezContainerEntryRaw(tree, entry);
    FILE *fh = fopen("build/out.png", "w");
    fwrite(data, entry->fileSize * sizeof(unsigned char), 1, fh);
    fclose(fh);
    free(data);
    ezContainerFree(tree);
    return 0;
}
