#include <stdio.h>
#include <stdlib.h>

int w = 0; //debug puro *ignora ignora

//page table entries
unsigned int L1[4096];
unsigned int N_ADDR[4096];
unsigned int L2FINE[1024];
unsigned int L2COARSE[256]; 
#define OFFS_SE 20      //máscara para offset de section entry L1
#define COARSE_TE 255   

//cache data
#define I_DADO  4
#define I_BLOCO 8
#define TAG     20
#define MASK1   0xf
#define MASK2   0xff
//cache data

#define SETSCACHE 256


int manageFileL2(int real_addr, int l1_base, int offset_l2){

    const char *filename = "L2.txt";
    FILE *file = fopen(filename, "r");

    if(!file)
    {
        printf("Arquivo %s não foi encontrado\n", filename);
        return -1;
    }

    //definindo os limites dos blocos de L2 (0-255 ___ 256-511 ___ 512-767)
    int min_bloco_l2 = (l1_base * COARSE_TE);
    int max_bloco_l2 = (min_bloco_l2 + (COARSE_TE-1));
    int diff = max_bloco_l2 - min_bloco_l2; //intervalo no qual o bloco se encontra

    int i = 0;
    unsigned int bit;
    while(!feof(file))
    {
        fscanf(file,"0x%x\n", &bit); //ler o arquivo
        L2COARSE[i] = bit;          //pega as linhas que tem em L2
        i++;
    }
    fclose(file);

    int real_adrr_l2 = 0;    
    for (int j = 0; j < i; j++) //FOR PARA ENCONTRAR O BLOCO ENTRE AS TABELAS L2
    {
        L2COARSE[j];
        if(min_bloco_l2 == j){  //conferir o início do bloco com o valor obtido através do offset de L1
            {
                for (int r = min_bloco_l2; r < max_bloco_l2; r++){ //FOR PARA ENCONTRAR A LINHA DENTRO DO BLOCO
                    if(offset_l2 == r){
                        real_adrr_l2 = L2COARSE[r];
                        break;
                    }

                }         

            }
        }
    }


    int real_addr_l2 = (L2COARSE[i]  & 0xfff00000) | real_addr; //máscara para pegar o valor da base em L2, mas manter o offset do endereço original (complete_address)
   
    return 0;
}

int confereBits(int real_address, int address_l1){
    
        int last_bits;
        last_bits = MASK2 & address_l1; //máscara para pegar os últimos bits identificadores do tipo de tabela
        
        printf("O VALOR DE last_bits É: 0x%08x e adress_l1 É: 0x%08x\n", last_bits, address_l1);

        int addr_base = ((address_l1 >> OFFS_SE));       //pega o endereço base do endereço de l1
        int offsetl1 = (address_l1 & 0x000fffff);        //pega o offset do endereço de l1

        if(last_bits == 0x10){
        printf("Section Entry\n");
            //o endereço real encontrado já pode ser colocado na cache
            printf("O endereço físico resultou em: 0x%08x\n", address_l1);

        }else if(last_bits == 0x00){
            printf("FAULT\n");

        }else{

            int offsetl2 = (address_l1 & 0x000ff000);        //pega o offset do endereço de l2
            manageFileL2(real_address, addr_base, offsetl2);

        } 

        last_bits = MASK2;

        return 1;
}

// READ L1 FILE (AQUI QUE TÁ O w)
int manageFileL1(int base_address, int offset, int complete_address){

    const char *filename = "L1.txt";
    FILE *file = fopen(filename, "r");

    if(!file)
    {
        printf("Arquivo %s não foi encontrado\n", filename);
        return -1;
    }

    printf("\n o %dº BASE ADDRESS É: 0x%08x = ""%d""\n", w, base_address, base_address);
    w++; //debug
    int i = 0;
    int flag = 0;

    unsigned int bit;
    while(!feof(file))
    {
        fscanf(file,"0x%x\n", &bit); //ler o arquivo
        L1[i] = bit;

        if(base_address == i){  //conferir linha da página de acordo com o endereço base

            flag = 1;
            int real_address = (L1[i]  & 0xfff00000) | (complete_address & 0x000fffff); //máscara para pegar o valor da base em L1, mas manter o offset do endereço original (complete_address)
            printf("################### \n o real address é 0x%08x\n e complete_address é 0x%08x\n", real_address, complete_address);

            confereBits(real_address, L1[i]);
        }

        i++;
    }

    fclose(file);
    if(flag == 0){
        printf("O endereço 0x%08x não foi encontrado na tabela ;-;", complete_address);
    }
   
    return 0;
}


int manageFileAddress(void){
    const char *filename = "addresses.txt";
    FILE *file = fopen(filename, "r");

    if(!file)
    {
        printf("Arquivo %s não foi encontrado\n", filename);
        return -1;
    }

    int i = 0; // i é a quantidade de linhas de endereço no arquivo
    unsigned int bit;
    while(!feof(file))
    {
        fscanf(file,"0x%x\n", &bit); //ler o arquivo
        N_ADDR[i++] = bit;          //salva as linhas do arquivo em ADDR
        //printf("0x%08x\n", bit); //imprimir os valores do arquivo
    }

    fclose(file);
    
    int base_endereco;
    int offsetl1;
    int q = 0;
    while(i>0)  //enquanto as linhas do arquivo não terminarem
    {   

        base_endereco = ((N_ADDR[q] >> OFFS_SE));   //pega o endereço base do endereço completo
        offsetl1 = (N_ADDR[q] & 0x000fffff);
        //printf("O OFFSET DO VALOR É 0x%08x\n", offsetl1);

        //CASO L1 SECTION ENTRY
        manageFileL1(base_endereco, offsetl1, N_ADDR[q]);    //procurar a linha desse endereço na L1 através da base

        //manageAddressL1(N_ADDR[q++]);
        //printf(" A linha em endereço eh: >>>0x%08x<<<\n\n", N_ADDR[q]); //imprimir os valores do arquivo
        i--;
        q++;

    }

    //int q = 0;
    //printf("0x%08x\n", N_ADDR[q]); //imprimir a linha do endereço do programa
    
    return 0;
}

//set cache function
int setCache(void){
    
    int cache_tag [SETSCACHE];
    int frst_addr [SETSCACHE];
    int lst_addr  [SETSCACHE];
    int cache_miss = 0;
    int cache_hit  = 0;
    char v[SETSCACHE];
    char d[SETSCACHE];

    int n_addr = 0x12345678; //EXEMPLO DE ENDEREÇO (SÃO 32 BITS NÉ, 8 VALORES)

    int tag = (n_addr >> (I_BLOCO + I_DADO));       //tag da linha
    int indice_bloco = (n_addr >> I_DADO) & MASK2;  //índice do bloco
    int indice_dados = n_addr & MASK1;              //índice dos dados

    /* 
    //!!!DEBUG!!!
    printf(" o valor da tag eh: 0x%08x\n", tag);
    printf(" o valor do indice_boco eh: 0x%08x\n", indice_bloco);
    printf(" o valor do indice_dados eh: 0x%08x\n", indice_dados); */
    
    if(cache_tag[indice_bloco] == tag){       //se a tag do endereço for igual a tag na cache -> HIT
        printf("Cache HIT!");
        cache_hit++;            //incrementa a variável de cache hit

    }else{
        cache_tag[indice_bloco] = tag;
        frst_addr[indice_bloco] = n_addr & ~(MASK1); //transforma o primeiro valor em '0'
        lst_addr[indice_bloco] = n_addr | MASK1;     //transforma o primeiro valor em 'f'
        cache_miss++;                                //incrementa a variável de cache miss
        v[indice_bloco] = 1;                         //tag v é válida (contém o dado que está em memória) 
        d[indice_bloco] = 0;

    }

    //falta fazer o caso em que d = 1;
    //set cache function

}

int main(){

    //setCache();
    manageFileAddress();
    printf("batata");

    return 0;
}
