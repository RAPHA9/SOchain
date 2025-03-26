int main(int argc, char *argv[]) { 
    //init data structures 
    struct info_container* info = allocate_dynamic_memory(sizeof(struct info_container)); 
    struct buffers* buffs = allocate_dynamic_memory(sizeof(struct buffers)); 
    //execute main code 
    main_args(argc, argv, info); 
    create_dynamic_memory_structs(info, buffs); 
    create_shared_memory_structs(info, buffs); 
    create_processes(info, buffs); 
    user_interaction(info, buffs); 
    //release memory before terminating 
    destroy_shared_memory_structs(info, buffs); 
    destroy_dynamic_memory_structs(info, buffs); 
    return 0; 
} 