/**
 ******************************************************************************
 * @file notfs.c
 * @author leven
 * @brief
 ******************************************************************************
 */

#include "notfs.h"

#define NOTFS_KEY                      (0x12345678U)
#define NOTFS_NOT_VALID_INDEX          (0xFFFFF)  /* 索引占用20个比特位 */
#define NOTFS_SUBREGION_TOTAL_SIZE     (956 * 1024)
#define NOTFS_INODE_ADDR_OFFSET        (4096)     /* 备份索引的地址偏移量（相对于主索引地址） */
#define NOTFS_FILE_ADDR_STEP           (4096 * 2) /* 主文件地址的步进 */
#define NOTFS_FILE_ADDR_OFFSET         (4096)     /* 备份文件的地址偏移量（相对主数据地址） */
#define NOTFS_FILE_MAX_SIZE            (NOTFS_FILE_ADDR_OFFSET - 8) /* 允许的单文件最大容量 */
#define NOTFS_FILE_MAX_COUNT           (256  - 2) /* 允许的文件最大数量 */

struct notfs_inode {
    struct nofs_header header;
    notfs_inode_t *inode;
};
struct notfs_file {
    struct nofs_header header;
    /* void *file */ /* 因为文件指针直接使用 API 参数中传进来的数据指针，所以此处不再单独定义 */
};

/* supper block */
static notfs_super_block_t s_notfs_sb[NOTFS_SUBREGION_MAX] = NOTFS_NORFLASH_SUBREGION_TABLE;
/* inode */
static struct notfs_inode s_notfs_inode[NOTFS_SUBREGION_MAX];
/* data */
static struct notfs_file s_notfs_file[NOTFS_SUBREGION_MAX];

/* 只是为了增强异常的处理 */
static bool s_check_failed[NOTFS_SUBREGION_MAX] = {false, false};
/* 只是为了加快寻址 */
static uint32_t s_notfs_current_index[NOTFS_SUBREGION_MAX] = {0};
/* 只是为了加快寻址 */
static uint32_t s_notfs_first_data_index[NOTFS_SUBREGION_MAX] = {0};

static uint8_t s_init_flag_check[NOTFS_SUBREGION_MAX] = {0}, s_is_init[NOTFS_SUBREGION_MAX] = {0};

notfs_err_e notfs_init(void)
{
    notfs_err_e err = NOTFS_NO_ERR;

    uint8_t tp = 0xFF; /* 芯片初始值为 0xFF */
    uint32_t i, j, k, t;
    uint32_t subregion_start_addr, file_max_count, total_size, isize, crc, tmp;

    for (i = 0; i < NOTFS_SUBREGION_MAX; ++i) {
        if(s_is_init[i] == 0){
            s_notfs_current_index[i] = NOTFS_NOT_VALID_INDEX;
        }
    }

    for (i = 0; i < NOTFS_SUBREGION_MAX; ++i) {
        if(s_is_init[i] == 0){
            subregion_start_addr = s_notfs_sb[i].subregion_start_addr; /* 用于已经先于进行默认配置 */
            file_max_count = s_notfs_sb[i].file_max_count;             /* 用户已经先于进行默认配置 */
            total_size = s_notfs_sb[i].subregion_total_size;           /* 用户已经先于进行默认配置 */

            memset(&(s_notfs_sb[i]), 0x00, sizeof(notfs_super_block_t));
            /* 读出分区的超级块信息 */
            err = notfs_port_read(subregion_start_addr, (uint8_t *)&(s_notfs_sb[i]), sizeof(notfs_super_block_t));
            rt_kprintf("notfs_init(%d, %x, %d, %d, %d)\n", i, s_notfs_sb[i].key, s_check_failed[i], s_init_flag_check[i], err);
            if (NOTFS_NO_ERR != err) {
                break;
            }

            if (((NOTFS_KEY != s_notfs_sb[i].key) || (s_check_failed[i])) && (s_init_flag_check[i] <= 5)) {
                if(s_notfs_sb[i].key == 0xFFFFFFFF){
                    /* 执行分区擦除 */
                    (void)notfs_port_erase(subregion_start_addr, total_size);
                    /* 初始化超级块数据 */
                    s_notfs_sb[i].key = NOTFS_KEY;
                    s_notfs_sb[i].subregion_start_addr = subregion_start_addr;
                    s_notfs_sb[i].subregion_total_size = total_size;
                    s_notfs_sb[i].file_max_size        = NOTFS_FILE_MAX_SIZE;
                    s_notfs_sb[i].file_max_count       = (file_max_count < NOTFS_FILE_MAX_COUNT ? file_max_count : NOTFS_FILE_MAX_COUNT);
                    s_notfs_sb[i].inode_start_addr     = s_notfs_sb[i].subregion_start_addr + 4096 * 1;
                    s_notfs_sb[i].inode_addr_offset    = NOTFS_INODE_ADDR_OFFSET;
                    s_notfs_sb[i].file_start_addr      = s_notfs_sb[i].inode_start_addr + 4096 * 2;
                    s_notfs_sb[i].file_addr_step       = NOTFS_FILE_ADDR_STEP;
                    s_notfs_sb[i].file_addr_offset     = NOTFS_FILE_ADDR_OFFSET;
                    s_notfs_sb[i].crc                  = notfs_crc32(0, (const uint8_t *)&(s_notfs_sb[i]), sizeof(s_notfs_sb[i]));
                    /* 初始化索引区数据 */
                    for (t = 0, crc = 0x00; t < sizeof(notfs_inode_t) * s_notfs_sb[i].file_max_count; t++) {
                        crc = notfs_crc32(crc, &tp, sizeof(tp)); /* 按照 0xFF 计算校验 */
                    }
                    s_notfs_inode[i].header.crc = crc;
                    /* 初始化主备索引区 */
                    (void)notfs_port_write(s_notfs_sb[i].inode_start_addr, (const uint8_t *)&(s_notfs_inode[i].header), sizeof(struct nofs_header));
                    (void)notfs_port_write(s_notfs_sb[i].inode_start_addr + s_notfs_sb[i].inode_addr_offset, (const uint8_t *)&(s_notfs_inode[i].header), sizeof(struct nofs_header));
                    /* 初始化超级块 */

                    err = notfs_port_write(s_notfs_sb[i].subregion_start_addr, (const uint8_t *)&(s_notfs_sb[i]), sizeof(s_notfs_sb[i]));
                    err = NOTFS_NOTINIT_ERR;
                    /* 初始化校验失败标志 */
                    s_check_failed[i] = false;
                }else{
                    if(s_init_flag_check[i] < 0xFF){
                        s_init_flag_check[i]++;
                    }
                    return NOTFS_READ_ERR;
                }
            } else {
                s_is_init[i] = 1;

                memset(&(s_notfs_inode[i]), 0x00, sizeof(struct notfs_inode));
                memset(&(s_notfs_file [i]), 0x00, sizeof(struct notfs_file));

                /* 申请内存前进行安全处理 */
                if (NOTFS_FILE_MAX_COUNT < s_notfs_sb[i].file_max_count) {
                    s_notfs_sb[i].file_max_count = NOTFS_FILE_MAX_COUNT;
                }
                isize = sizeof(notfs_inode_t) * s_notfs_sb[i].file_max_count;

                /* 申请索引块内存 */
                s_notfs_inode[i].inode = (notfs_inode_t *)(malloc(isize));
                if (NULL == s_notfs_inode[i].inode) {
                    err = NOTFS_MALLOC_ERR;
//                    rt_kprintf("NOTFS_MALLOC_ERR(%d)\n", isize);
                    break;
                }
                memset(s_notfs_inode[i].inode, 0x00, isize); /* 对内存块进行清0 */

                /* 从主索引区读数据 */
                (void)notfs_port_read(s_notfs_sb[i].inode_start_addr, (uint8_t *)&(s_notfs_inode[i].header), sizeof(struct nofs_header));
                (void)notfs_port_read(s_notfs_sb[i].inode_start_addr + sizeof(struct nofs_header), (uint8_t *)(s_notfs_inode[i].inode), isize);


//                extern void thaisenW25qxxRead(unsigned char* pBuffer,unsigned int ReadAddr,unsigned short NumByteToRead);
//                thaisenW25qxxRead((uint8_t *)&(s_notfs_inode[i].header), s_notfs_sb[i].inode_start_addr, sizeof(struct nofs_header));
//                thaisenW25qxxRead((uint8_t *)(s_notfs_inode[i].inode), s_notfs_sb[i].inode_start_addr + sizeof(struct nofs_header), isize);

//                printf("thaisenW25qxxRead(%d)\n", isize);
//                for(uint16_t count = 0; count < isize; count++){
//                    if(*((uint8_t*)(s_notfs_inode[i].inode) + count) < 0x10){
//                        printf("0%X ", *((uint8_t*)(s_notfs_inode[i].inode) + count));
//                    }else{
//                        printf("%X ", *((uint8_t*)(s_notfs_inode[i].inode) + count));
//                    }
//                    if(count%16 == 0 && count != 0){
//                        printf("\n");
//                    }
//                }
//                printf("\n----------------------------------------------------\n");

                crc = notfs_crc32(0, (const uint8_t *)(s_notfs_inode[i].inode), isize);
                if (crc != s_notfs_inode[i].header.crc) {
                    /* 从备份索引区读数据 */
//
//
//                    thaisenW25qxxRead((uint8_t *)&(s_notfs_inode[i].header), (s_notfs_sb[i].inode_start_addr + s_notfs_sb[i].inode_addr_offset), sizeof(struct nofs_header));
//                    thaisenW25qxxRead((uint8_t *)(s_notfs_inode[i].inode), (s_notfs_sb[i].inode_start_addr + s_notfs_sb[i].inode_addr_offset + sizeof(struct nofs_header)), isize);
//

                    (void)notfs_port_read(s_notfs_sb[i].inode_start_addr + s_notfs_sb[i].inode_addr_offset,
                            (uint8_t *)&(s_notfs_inode[i].header), sizeof(struct nofs_header));
                    (void)notfs_port_read(s_notfs_sb[i].inode_start_addr + s_notfs_sb[i].inode_addr_offset + sizeof(struct nofs_header),
                            (uint8_t *)(s_notfs_inode[i].inode), isize);
                    crc = notfs_crc32(0, (const uint8_t *)(s_notfs_inode[i].inode), isize);
                    if (crc != s_notfs_inode[i].header.crc) {
                        /* 校验还失败，那就重新格式化分区吧，并且把申请的索引区内存释放掉 */
                        s_check_failed[i] = true;
                        free(s_notfs_inode[i].inode);
                        rt_kprintf("NOTFS_NOTINIT_ERR(%d)\n", isize);
                        err = NOTFS_NOTINIT_ERR; /* 主备都校验失败，就是初始化异常 */
                        break;
                    }
                }

                /* 找出可存储的索引下标 */
                for (j = 0; j < s_notfs_sb[i].file_max_count; j++) {
                    if (NOTFS_NOT_VALID_INDEX == s_notfs_inode[i].inode[j].index) {
                        s_notfs_current_index[i] = (0 == j ? NOTFS_NOT_VALID_INDEX : j - 1); /* 对一块新的分区或一块还从未写满的分区的处理 */
                        break;
                    }
                }
                if (j >= s_notfs_sb[i].file_max_count) {
                    for (tmp = s_notfs_inode[i].inode[0].index, k = 0, j = 1; j < s_notfs_sb[i].file_max_count; j++) {
                        if (s_notfs_inode[i].inode[j].index > tmp) {
                            tmp = s_notfs_inode[i].inode[j].index;
                            k = j;
                        }
                    }
                    s_notfs_current_index[i] = ((0 == ((tmp + 1) % s_notfs_sb[i].file_max_count)) ? NOTFS_NOT_VALID_INDEX : k); /* 对一块分区刚好写满了或者分区至少写满过的处理 */
                }
                rt_kprintf("s_notfs_current_index[%d] = %x\n", i, s_notfs_current_index[i]);
            }
        }
    }

    return err;
}

notfs_err_e notfs_subregion_append_record(enum notfs_subregion subregion, const void *buf, size_t size, uint8_t user_data, uint16_t verify_mask)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }
    if (NULL == s_notfs_inode[subregion].inode) {
        return NOTFS_ILLEGAL_ERR;
    }

    if (NULL == buf) {
        return NOTFS_ILLEGAL_ERR;
    }
    if (size > s_notfs_sb[subregion].file_max_size) {
        return NOTFS_OUTRANGE_ERR;
    }

    notfs_err_e err = NOTFS_NO_ERR;
    uint32_t address, index, isize = sizeof(notfs_inode_t) * s_notfs_sb[subregion].file_max_count;

    /* 获取并计算当前索引信息 */
    index = s_notfs_current_index[subregion];
    if ((NOTFS_NOT_VALID_INDEX == index) || (index >= s_notfs_sb[subregion].file_max_count - 1)) {
        index = 0; /* 对一块新分区或一块刚好写满的分区做索引环绕（也就是重新开始写） */
    } else {
        index += 1;
    }

    /* 初始化 FILE HANDER */
    s_notfs_file[subregion].header.crc = notfs_crc32(0, (const uint8_t *)buf, size);

    /* 存储 FILE 到主文件区 */
    address = s_notfs_sb[subregion].file_start_addr + index * s_notfs_sb[subregion].file_addr_step;
    (void)notfs_port_erase(address, NOTFS_FILE_ADDR_STEP - NOTFS_FILE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_file[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)buf, size);

    /* 存储 FILE 到备份文件区 */
    address = address + s_notfs_sb[subregion].file_addr_offset;
    (void)notfs_port_erase(address, NOTFS_FILE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_file[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)buf, size);

    /* 填充 INODE */
    s_notfs_inode[subregion].inode[index].timestamp = notfs_timestamp();

    if (NOTFS_NOT_VALID_INDEX == s_notfs_inode[subregion].inode[index].index) {
        s_notfs_inode[subregion].inode[index].index = index; /* 对一块新的分区或一块还从未写满的分区的处理 */
    } else {
        s_notfs_inode[subregion].inode[index].index += s_notfs_sb[subregion].file_max_count; /* 对一块分区刚好写满了或者分区至少写满过的处理 */
    }
    s_notfs_inode[subregion].inode[index].user_data = user_data;
    s_notfs_inode[subregion].inode[index].record_verify = verify_mask;

    /* 初始化 INODE HANDER */
    s_notfs_inode[subregion].header.crc = notfs_crc32(0, (const uint8_t *)(s_notfs_inode[subregion].inode), isize);

    /* 存储 INODE 到主索引区 */
    address = s_notfs_sb[subregion].inode_start_addr;

//    printf("0000000000 s_notfs_inode main(%d, %d, %x, %d)\n", isize, subregion, address, s_notfs_inode[subregion].inode[index].index);
//    for(uint16_t count = 0; count < isize; count++){
//        if(*((uint8_t*)(s_notfs_inode[subregion].inode) + count) < 0x10){
//            printf("0%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }else{
//            printf("%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }
//        if(count%16 == 0 && count != 0){
//            printf("\n");
//        }
//    }
//    printf("\n----------------------------------------------------\n");

    (void)notfs_port_erase(address, NOTFS_FILE_ADDR_STEP - NOTFS_INODE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_inode[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)(s_notfs_inode[subregion].inode), isize);
    /* 存储 INODE 到备份索引区 */

//    printf("0000000000 s_notfs_inode backup(%d, %d, %x, %d)\n", isize, subregion, address, s_notfs_inode[subregion].inode[index].index);
//    for(uint16_t count = 0; count < isize; count++){
//        if(*((uint8_t*)(s_notfs_inode[subregion].inode) + count) < 0x10){
//            printf("0%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }else{
//            printf("%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }
//        if(count%16 == 0 && count != 0){
//            printf("\n");
//        }
//    }
//    printf("\n----------------------------------------------------\n");
    address = address + s_notfs_sb[subregion].inode_addr_offset;
    (void)notfs_port_erase(address, NOTFS_INODE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_inode[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)(s_notfs_inode[subregion].inode), isize);

    /* 更新当前索引信息 */
    s_notfs_current_index[subregion] = index;

    return err;
}

notfs_err_e notfs_subregion_updated_designate_index_data(enum notfs_subregion subregion, const void *buf, size_t size, uint8_t user_data, uint16_t verify_mask, int32_t index)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }
    if (NULL == s_notfs_inode[subregion].inode) {
        return NOTFS_ILLEGAL_ERR;
    }

    if (NULL == buf) {
        return NOTFS_ILLEGAL_ERR;
    }
    if (size > s_notfs_sb[subregion].file_max_size) {
        return NOTFS_OUTRANGE_ERR;
    }

    if(0 > index){
        if(NOTFS_SUBREGION_INDEX_CURRENT == index){
            index = s_notfs_current_index[subregion];
        }else if(NOTFS_SUBREGION_INDEX_FIRST == index){
            index = s_notfs_first_data_index[subregion];
        }else{
            return NOTFS_OUTRANGE_ERR;
        }
    }

    if (index >= s_notfs_sb[subregion].file_max_count) {
        return NOTFS_OUTRANGE_ERR;
    }

    if (NOTFS_NOT_VALID_INDEX == s_notfs_inode[subregion].inode[index].index) {
        return NOTFS_OUTRANGE_ERR; /* 这还是一块从来没有存储过数据的区域 */
    }

    notfs_err_e err = NOTFS_NO_ERR;
    uint32_t address, isize = sizeof(notfs_inode_t) * s_notfs_sb[subregion].file_max_count;

    /* 初始化 FILE HANDER */
    s_notfs_file[subregion].header.crc = notfs_crc32(0, (const uint8_t *)buf, size);

    /* 存储 FILE 到主文件区 */
    address = s_notfs_sb[subregion].file_start_addr + index * s_notfs_sb[subregion].file_addr_step;
    (void)notfs_port_erase(address, NOTFS_FILE_ADDR_STEP - NOTFS_FILE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_file[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)buf, size);

    /* 存储 FILE 到备份文件区 */
    address = address + s_notfs_sb[subregion].file_addr_offset;
    (void)notfs_port_erase(address, NOTFS_FILE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_file[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)buf, size);

    /* 填充 INODE */
    s_notfs_inode[subregion].inode[index].timestamp = notfs_timestamp();
    s_notfs_inode[subregion].inode[index].user_data = user_data;
    s_notfs_inode[subregion].inode[index].record_verify |= verify_mask;

    rt_kprintf("111notfs_subregion_updated_designate_index_data(%d, %d, %x, %x)\n", subregion, index,
            s_notfs_inode[subregion].inode[index].record_verify, verify_mask);

    /* 初始化 INODE HANDER */
    s_notfs_inode[subregion].header.crc = notfs_crc32(0, (const uint8_t *)(s_notfs_inode[subregion].inode), isize);

//    printf("1111111111 s_notfs_inode main(%d, %d, %x)\n", isize, subregion, address);
//    for(uint16_t count = 0; count < isize; count++){
//        if(*((uint8_t*)(s_notfs_inode[subregion].inode) + count) < 0x10){
//            printf("0%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }else{
//            printf("%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }
//        if(count%16 == 0 && count != 0){
//            printf("\n");
//        }
//    }
//    printf("\n----------------------------------------------------\n");

    /* 存储 INODE 到主索引区 */
    address = s_notfs_sb[subregion].inode_start_addr;
    (void)notfs_port_erase(address, NOTFS_FILE_ADDR_STEP - NOTFS_INODE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_inode[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)(s_notfs_inode[subregion].inode), isize);



//    printf("11111111 s_notfs_inode backup(%d, %d, %x)\n", isize, subregion, address);
//    for(uint16_t count = 0; count < isize; count++){
//        if(*((uint8_t*)(s_notfs_inode[subregion].inode) + count) < 0x10){
//            printf("0%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }else{
//            printf("%X ", *((uint8_t*)(s_notfs_inode[subregion].inode) + count));
//        }
//        if(count%16 == 0 && count != 0){
//            printf("\n");
//        }
//    }
//    printf("\n----------------------------------------------------\n");

    /* 存储 INODE 到备份索引区 */
    address = address + s_notfs_sb[subregion].inode_addr_offset;
    (void)notfs_port_erase(address, NOTFS_INODE_ADDR_OFFSET);
    (void)notfs_port_write(address, (const uint8_t *)&(s_notfs_inode[subregion].header), sizeof(struct nofs_header));
    (void)notfs_port_write(address + sizeof(struct nofs_header), (const uint8_t *)(s_notfs_inode[subregion].inode), isize);

    return err;
}

notfs_err_e notfs_get_subregion_designate_index_record_data(enum notfs_subregion subregion, uint8_t* buf, size_t size, int32_t index)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }
    if (NULL == s_notfs_inode[subregion].inode) {
        return NOTFS_ILLEGAL_ERR;
    }

    if (NULL == buf) {
        return NOTFS_ILLEGAL_ERR;
    }
    if (size >= s_notfs_sb[subregion].file_max_size) {
        return NOTFS_ILLEGAL_ERR;
    }

    if(0 > index){
        if(NOTFS_SUBREGION_INDEX_CURRENT == index){
            index = s_notfs_current_index[subregion];
        }else if(NOTFS_SUBREGION_INDEX_FIRST == index){
            index = s_notfs_first_data_index[subregion];
        }else{
            return NOTFS_OUTRANGE_ERR;
        }
    }

    if (index >= s_notfs_sb[subregion].file_max_count) {
        return NOTFS_OUTRANGE_ERR;
    }

    if (NOTFS_NOT_VALID_INDEX == s_notfs_inode[subregion].inode[index].index) {
        return NOTFS_OUTRANGE_ERR; /* 这还是一块从来没有存储过数据的区域 */
    }

    notfs_err_e err = NOTFS_NO_ERR;

    uint32_t crc, addr;
    struct nofs_header file_header = {0};

    addr = s_notfs_sb[subregion].file_start_addr + index * s_notfs_sb[subregion].file_addr_step;
    (void)notfs_port_read(addr, (uint8_t *)&(file_header), sizeof(file_header));
    (void)notfs_port_read(addr + sizeof(struct nofs_header), buf, size);

//    printf("\n----------------------------------------------------\n");
//    printf("notfs_record_get_designate_index_record(%d, %d, %d)\n", size, subregion, index);
//    for(uint16_t count = 0; count < size; count++){
//        if(buf[count] < 0x10){
//            printf("0%X ", buf[count]);
//        }else{
//            printf("%X ", buf[count]);
//        }
//        if(count%16 == 0 && count != 0){
//            printf("\n");
//        }
//    }
//    printf("\n----------------------------------------------------\n");

    crc = notfs_crc32(0, buf, size);
    if (crc != file_header.crc) {
        rt_kprintf("file_header.crc(%x, %x)\n", crc, file_header.crc);
        addr = s_notfs_sb[subregion].file_start_addr + index * s_notfs_sb[subregion].file_addr_step + s_notfs_sb[subregion].file_addr_offset;
        (void)notfs_port_read(addr, (uint8_t *)&(file_header), sizeof(file_header));
        (void)notfs_port_read(addr + sizeof(struct nofs_header), buf, size);

        crc = notfs_crc32(0, buf, size);
        if (crc != file_header.crc) {
            err = NOTFS_CHECK_ERR;
        }
    }

    return err;
}

int32_t notfs_get_subregion_record_total_num(enum notfs_subregion subregion)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }

    if (NULL == s_notfs_inode[subregion].inode) {
        return NOTFS_ILLEGAL_ERR;
    }

    if(s_notfs_current_index[subregion] >= s_notfs_sb[subregion].file_max_count){
        return NOTFS_OUTRANGE_ERR;
    }

    uint32_t count = s_notfs_current_index[subregion] + 1;
    if(count < s_notfs_sb[subregion].file_max_count){
        if(s_notfs_inode[subregion].inode[s_notfs_current_index[subregion] + 1].index != NOTFS_NOT_VALID_INDEX){
            count = s_notfs_sb[subregion].file_max_count;
        }
    }

    return count;
}

int32_t notfs_get_subregion_userdata_record_num(enum notfs_subregion subregion, uint8_t user_data)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }
    if (NULL == s_notfs_inode[subregion].inode) {
        return NOTFS_ILLEGAL_ERR;
    }

    int32_t index, count = 0;

    for (index = 0; index < s_notfs_sb[subregion].file_max_count; index++) {
        if (user_data == s_notfs_inode[subregion].inode[index].user_data) {
            count += 1;
        }
    }

    return count;
}

int32_t notfs_get_subregion_current_index(enum notfs_subregion subregion)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }

    if(s_notfs_current_index[subregion] >= s_notfs_sb[subregion].file_max_count){
        return NOTFS_OUTRANGE_ERR;
    }

    return s_notfs_current_index[subregion];
}

int32_t notfs_clear_subregion_record_info(enum notfs_subregion subregion)
{
    notfs_err_e err = NOTFS_NO_ERR;

    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }

    /* 擦除记录信息 */
    if(notfs_port_erase((s_notfs_sb[subregion].subregion_start_addr + 1 * NOTFS_INODE_ADDR_OFFSET),
            (s_notfs_sb[subregion].subregion_total_size - (1 * NOTFS_INODE_ADDR_OFFSET))) < 0){
        err = NOTFS_ERASE_ERR;
        return err;
    }

    memset(s_notfs_inode[subregion].inode, 0xFF, (sizeof(notfs_inode_t) * s_notfs_sb[subregion].file_max_count));

    s_notfs_current_index[subregion] = NOTFS_NOT_VALID_INDEX;
    s_notfs_first_data_index[subregion] = 0;

    return err;
}

int32_t notfs_get_subregion_first_index_userdata(enum notfs_subregion subregion, uint8_t user_data)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }

    if (s_notfs_sb[subregion].file_max_count <= s_notfs_current_index[subregion]) {
        return NOTFS_OUTRANGE_ERR;
    }

    int32_t index = -1, count = 0;

    for (count = s_notfs_current_index[subregion]; count >=0; count--) {
        if (user_data == s_notfs_inode[subregion].inode[count].user_data) {
            index = s_notfs_inode[subregion].inode[count].index %s_notfs_sb[subregion].file_max_count;
            break;
        }
    }

    if(0 > index){
        if((s_notfs_current_index[subregion] + 1) < s_notfs_sb[subregion].file_max_count){
            if(s_notfs_inode[subregion].inode[s_notfs_current_index[subregion] + 1].index != NOTFS_NOT_VALID_INDEX){
                for (count = s_notfs_sb[subregion].file_max_count - 1; count > s_notfs_current_index[subregion]; count--) {
                    if (user_data == s_notfs_inode[subregion].inode[count].user_data) {
                        index = s_notfs_inode[subregion].inode[count].index %s_notfs_sb[subregion].file_max_count;
                        break;
                    }
                }
            }
        }
    }

    return index;
}

int32_t notfs_get_subregion_record_verify_record_num(enum notfs_subregion subregion, uint16_t verify_mask)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }
    if (NULL == s_notfs_inode[subregion].inode) {
        return NOTFS_ILLEGAL_ERR;
    }

    int32_t index, count = 0;

    for (index = 0; index < s_notfs_sb[subregion].file_max_count; index++) {
        if (((s_notfs_inode[subregion].inode[index].record_verify) &verify_mask) == 0x00) {
            count += 1;
        }
    }

    return count;
}

int32_t notfs_get_subregion_first_index_record_verify(enum notfs_subregion subregion, uint16_t verify_mask)
{
    if (NOTFS_SUBREGION_MAX < subregion) {
        return NOTFS_OUTRANGE_ERR;
    }

    if (s_notfs_sb[subregion].file_max_count <= s_notfs_current_index[subregion]) {
        return NOTFS_OUTRANGE_ERR;
    }

    int32_t index = -1, count = 0;

    for (count = s_notfs_current_index[subregion]; count >=0; count--) {
        if (((s_notfs_inode[subregion].inode[count].record_verify) &verify_mask) == 0x00) {
            index = s_notfs_inode[subregion].inode[count].index %s_notfs_sb[subregion].file_max_count;
            break;
        }
    }

    if(0 > index){
        if((s_notfs_current_index[subregion] + 1) < s_notfs_sb[subregion].file_max_count){
            if(s_notfs_inode[subregion].inode[s_notfs_current_index[subregion] + 1].index != NOTFS_NOT_VALID_INDEX){
                for (count = s_notfs_sb[subregion].file_max_count - 1; count > s_notfs_current_index[subregion]; count--) {
                    if (((s_notfs_inode[subregion].inode[count].record_verify) &verify_mask) == 0x00) {
                        index = s_notfs_inode[subregion].inode[count].index %s_notfs_sb[subregion].file_max_count;
                        break;
                    }
                }
            }
        }
    }

    return index;
}

/*****************************(C)COPYRIGHT(c) 2022 Thaisen *****END OF FILE****/
