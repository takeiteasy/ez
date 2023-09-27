#include "../src/bcf.h"

int main(int argc, const char* argv[]) {
    const char** test_files = malloc(3 * sizeof(const char*));
    test_files[0] = "tests/res/test.png";
    test_files[1] = "tests/res/test2.png";
    test_files[2] = "tests/res/test3.png";
    bcfwrite("build/test.cf", 3, test_files);
    free(test_files);
    
    struct bcf_tree tree;
    bcfread(&tree, "build/test.bcf");
    struct bcf_entry* entry = bcffind(&tree, "tests/res/test2.png");
    unsigned char* data = bcfdata(&tree, entry);
    FILE* fh = fopen("build/out.png", "w");
    fwrite(data, entry->f_sz * sizeof(unsigned char), 1, fh);
    fclose(fh);
    free(data);
    bcffree(&tree);
    return 0;
}
