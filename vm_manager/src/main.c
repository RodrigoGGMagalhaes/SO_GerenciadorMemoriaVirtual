#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "tlb.h"
#include "page_table.h"
#include "memory.h"
#include "statistics.h"

int main(void)
{
    FILE *backing = fopen(BACKING_STORE_PATH, "rb");

    if (backing == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", BACKING_STORE_PATH);
        fprintf(stderr, "Execute antes: cd data && python3 generate_data.py\n");
        return 1;
    }

    page_table_init();
    tlb_init();
    memory_init(backing);
    statistics_init();

    int logical_address;

    while (scanf("%d", &logical_address) == 1) {
        count_address();

        /*
         * Extração do número da página e offset[cite: 9, 10, 11].
         */
        int page = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;

        int frame = tlb_lookup(page);

        if (frame != -1) {
            count_tlb_hit();
        } else {
            frame = page_table_lookup(page);

            if (frame == -1) {
                count_page_fault();
                frame = handle_page_fault(page);
            }

            tlb_insert(page, frame);
        }

        /*
         * Atualização do LRU aproximado[cite: 75, 83].
         */
        page_table_set_reference(page);
        page_table_update_aging();

        /*
         * Cálculo do endereço físico e leitura da memória[cite: 11, 26].
         */
        int physical_address = (frame << 8) | offset;
        signed char value = read_memory(frame, offset);

        printf("Logical address: %d Physical address: %d Value: %d\n",
               logical_address,
               physical_address,
               value);
    }

    print_statistics();

    fclose(backing);

    return 0;
}