#pragma once

#include "../utils/configuration.h"
#include "../tx/transaction.h"

int sequencer_add_write_tx_to_batch(Tx *tx);
void *master_sequencer_startup(Config *config);