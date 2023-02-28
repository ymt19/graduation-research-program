#include "batch_list.h"
#include <assert.h>
#include <stdio.h>

int main() {
    Batch *batch1, *batch2, *batch0;
    batch1 = batch_create(1);
    batch2 = batch_create(2);

    batch_list_t *batch_list;
    batch_list = create_batch_list();
    add_node_batch_list(batch_list, batch1);
    add_node_batch_list(batch_list, batch2);

    batch0 = delete_node_batch_list(batch_list);
    assert(batch0 == batch1);
    batch0 = delete_node_batch_list(batch_list);
    assert(batch0 == batch2);
    assert(is_empty_batch_list(batch_list) == 1);
    batch0 = delete_node_batch_list(batch_list);
    assert(batch0 == NULL);
}