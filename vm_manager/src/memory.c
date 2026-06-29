int handle_page_fault(int page)
{
    int frame = find_free_frame();

    // 2. Se não houver quadro livre, selecionar página vítima
    if (frame == -1) {
        int victim_page = select_victim_page();
        
        // Obter o quadro da página vítima para reutilizá-lo
        for (int i = 0; i < NUM_FRAMES; i++) {
            if (frame_to_page[i] == victim_page) {
                frame = i;
                break;
            }
        }

        // 3. Invalidar página vítima na tabela de páginas [cite: 80]
        page_table_invalidate(victim_page);
        
        // 4. Remover página vítima do TLB [cite: 81, 142]
        tlb_invalidate(victim_page);
    }

    // 5. Ler a página correta do BACKING_STORE.bin [cite: 67, 71]
    fseek(backing, page * PAGE_SIZE, SEEK_SET);
    fread(physical_memory[frame], sizeof(signed char), PAGE_SIZE, backing);

    // 6. Atualizar frame_to_page
    frame_to_page[frame] = page;

    // 7. Atualizar tabela de páginas
    page_table_update(page, frame);

    return frame;
}

int select_victim_page(void)
{
    int victim_page = -1;
    int min_aging = 256; // Contador de 8 bits, valor máximo possível é 255

    // 9. Selecionar a página com o menor valor do contador [cite: 79, 91]
    for (int i = 0; i < NUM_FRAMES; i++) {
        int page = frame_to_page[i];
        if (page != -1) {
            int aging_counter = page_table_get_aging(page);
            if (aging_counter < min_aging) {
                min_aging = aging_counter;
                victim_page = page;
            }
        }
    }

    return victim_page;
}

signed char read_memory(int frame, int offset)
{
    // Retorna o byte armazenado na memória física
    return physical_memory[frame][offset];
}