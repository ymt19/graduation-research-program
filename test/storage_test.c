#include "storage.h"
#include <assert.h>

int main(void) {
    int i;
    Storage *storage = new_storage();

    for (i = 0; i < STORAGE_SIZE; i++) {
        write_to_storage(storage, i, i);
    }

    for (i = 0; i < STORAGE_SIZE; i++) {
        int value = read_from_storage(storage, i);
        assert(value == i);
    }

    return 0;
}