#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char start_sector[4];
    char length_sectors[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;

//desde el byte 13 (incluído)
	// {...} COMPLETAR
    unsigned char sectores_por_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short sectores_en_file_system;
    unsigned char media_type;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short numero_cabezas;
    unsigned char numero_sectores_antes_de_particion[4];
    unsigned char numero_sectores_en_file_system[4];

    unsigned char bios_int;
    unsigned char sin_uso;
    unsigned char extended_boot_signature;

//hasta el byte 38 (incluído)

    char volume_id[4];
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct {
	// {...} COMPLETAR
	//unsigned char first_character;
	unsigned char filename[11];
	unsigned char file_attributes;
	unsigned char reserved;
	unsigned char created_time_in_seconds;
	char created_time_hms[2];
	char created_day[2];
	char accesed_day[2];
	char first_cluster_address_high[2];
	char written_time[2];
	char written_day[2];
	char first_cluster_address_low[2];
	char size_of_file[4];
} __attribute((packed)) Fat12Entry;

void print_file_info(Fat12Entry *entry, FILE * in) {
//	char subbuff[3];
    switch(entry->filename[0]) {
//    switch(entry->first_character) {
    case 0x00:
        return; // unused entry
    case 0xE5:
        printf("Deleted file: [?%.7s.%.3s]\n", &entry->filename[1], &entry->filename[8]);
        return;
    case 0x05: 
        printf("File starting with 0xE5: [%c%.7s.%.3s]\n", 0xE5, &entry->filename[1], &entry->filename[8]);
        break;
    case 0x2E:
//        printf("Directory: falta completar\n"); // COMPLETAR aca creo que va file_attributes (los ultimos dos)
        printf("Directory: [%.7s.%s]\n", &entry->filename[0], &entry->filename[8]);
        break;
    default:
	if (entry->file_attributes == 0x10){ // Si 
		printf("Subdirectory: [%.7s.%.3s]\n", &entry->filename[0], &entry->filename[8]);
		uint16_t  cluster = 0;
		//Calculo el cluster inicial
		for (int i = 1; i >= 0; i--){
			cluster<<=8;
			cluster |=  entry->first_cluster_address_low[i];
		}
		if (cluster > 0){
			Fat12Entry entrada;
			unsigned int punteroActual = (unsigned int) ftell(in);
			fseek(in, 0x4200 + ((cluster - 1) * 0x800), SEEK_SET);			//Salta a la posicion del contenido del archivo, dada por el nro del cluster
			for (int indice = 0; indice < (0x800 / sizeof(entrada)); indice++){
				fread(&entrada, sizeof(entrada), 1, in);
				print_file_info(&entrada, in);
			}
			fseek(in, punteroActual, SEEK_SET);
		}
	}
	else{
		printf("File: [%.7s.%.3s]\n", &entry->filename[0], &entry->filename[8]);
	}
    }
    
}

int main(int argc, char** argv) {
	if (argv[1] != NULL){
		char *aBuscar = "ESDF";
		char *stringRecortado = argv[1] + 1;
		

		if (strstr(aBuscar, stringRecortado) != NULL) {
   			 printf("Contiene! \n");
		}
		
	}
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
    
    fseek(in, 0x1BE, SEEK_SET); // go to partition table start
    fread(pt, sizeof(PartitionTable), 4, in); // read all four entries
    
    for(i=0; i<4; i++) {        
        if(pt[i].partition_type == 1) {
            printf("FAT12 filesystem found from partition %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No FAT12 filesystem found, exiting...\n");
        return -1;
    }
    
    //fseek(in, 512 * (unsigned int) pt[i].start_sector, SEEK_SET);
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs\n\n", 
           (unsigned int) ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
           
    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    printf("Root dir_entries %d \n", (unsigned int) bs.root_dir_entries);
    for(i=0; i<bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry, in);
    }
    
    printf("\nRoot directory read, now at 0x%X\n", (unsigned int) ftell(in));
    fclose(in);
    return 0;
}
