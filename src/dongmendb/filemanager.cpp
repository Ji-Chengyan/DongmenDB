//
// Created by Sam on 2018/1/26.
//

#include <sys/stat.h>
#include <utils/utils.h>
#include <io.h>
#include <iostream>

#include "dongmendb/filemanager.h"

FileManager::FileManager(char *directory, const char *dbName) {
    this->isNew = 0;
    /*如果对应的文件夹不存在*/
    int exists = access(dbName, F_OK);
    if (exists != 0) {
        this->isNew = 1;
        mkdir(dbName);
    }

    this->dbDirectoryName = strdup(dbName);
    this->dbDirectory = fopen(dbName, "rw");

    this->openFiles  = new map<string, FILE*>();
};

int FileManager::file_manager_read(memory_page *memoryPage, DiskBlock *diskBlock) {

    FILE *fp = file_manager_getfile( diskBlock->fileName);
    //memset(memoryPage->contents, 0, sizeof(memoryPage->contents));
    fseek(fp, diskBlock->blkNum * DISK_BOLCK_SIZE, SEEK_SET);
    fread(memoryPage->contents, sizeof(char), sizeof(memoryPage->contents), fp);
};

int FileManager::file_manager_write(memory_page *memoryPage, DiskBlock *diskBlock) {

    FILE *fp = file_manager_getfile( diskBlock->fileName);
    fseek(fp, diskBlock->blkNum * DISK_BOLCK_SIZE, SEEK_SET);
    int size = sizeof(memoryPage->contents);
    fwrite(memoryPage->contents, sizeof(char), size, fp);
};

int FileManager::file_manager_append(MemoryBuffer *memoryBuffer, char *fileName, table_info *tableInfo) {
    int newBlockNum = file_manager_size( fileName);
    DiskBlock *diskBlock = new DiskBlock(fileName, newBlockNum, tableInfo);

    memoryBuffer->block = diskBlock;
    file_manager_write( memoryBuffer->contents, diskBlock);
};

/*获取文件大小*/
int FileManager::file_manager_size(char *fileName) {

    FILE *file = file_manager_getfile( fileName);

    fseek(file,0L,SEEK_END); /* 定位到文件末尾 */
    int flen=ftell(file); /* 得到文件大小 */
    return flen / DISK_BOLCK_SIZE;
}

int FileManager::file_manager_isnew(FileManager *fileManager) {
    return this->isNew;
};

FILE*  FileManager::file_manager_getfile(char *fileName) {
    map<string,FILE*>::iterator it;
    it = this->openFiles->find(fileName);
    if (it == this->openFiles->end()) {
        char *fname = new_id_name();
        strcat(fname, this->dbDirectoryName);
        strcat(fname, "/");
        strcat(fname, fileName);
        FILE *f = fopen(fname, "rwb+");
        if (f == NULL) {
            f = fopen(fname, "wb+");
            if (f == NULL) {
                /*TODO:error*/
                return NULL;
            }
            fclose(f);
            f = fopen(fname, "rwb+");
        }

        this->openFiles->insert(pair<string, FILE*>(fileName, f));
        return f;
    }else{
        return it->second;

    }

};

int FileManager::file_manager_closefile(char *fileName){
    map<string,FILE*>::iterator it;
    it = this->openFiles->find(fileName);
    if (it != this->openFiles->end()) {
        fclose(it->second);
        this->openFiles->erase(it);
    }

    return 1;
}

int FileManager::file_manager_closeallfile(){
    map<string,FILE*>::iterator iter;

    for(iter = this->openFiles->begin(); iter != this->openFiles->end(); iter++){
        fclose(iter->second);
    }
    this->openFiles->clear();

}

DiskBlock::DiskBlock(char *fileName, int blockNum, table_info *tableInfo) {

    this->fileName = fileName;
    this->blkNum = blockNum;
    this->tableInfo = tableInfo;

};

char *DiskBlock::disk_block_get_num_string() {
    char *blockNum = new_id_name();
    itoa(this->blkNum, blockNum, 10);

    char *blockName = new_id_name();
    strcat(blockName, this->tableInfo->tableName);
    strcat(blockName, blockNum);
    return blockName;
};

int memory_page_create(memory_page *memoryPage, FileManager *fileManager) {
    memset(memoryPage->contents, 0, sizeof(memoryPage->contents));
    memoryPage->fileManager = fileManager;
    return 1;
};

int memory_page_read(memory_page *memoryPage, DiskBlock *block) {
    memoryPage->fileManager->file_manager_read( memoryPage, block);
};

int memory_page_write(memory_page *memoryPage, DiskBlock *block) {
    memoryPage->fileManager->file_manager_write(memoryPage, block);
};

int memory_page_append(MemoryBuffer *memoryBuffer, char *fileName, table_info *tableInfo) {
    memoryBuffer->contents->fileManager-> file_manager_append( memoryBuffer, fileName, tableInfo);
};

/**
 * recordlen由整型数据的INTSIZE和字符数据的LEN + INTSIZE构成。
 *
 * @param contents
 * @param tableInfo
 * @return
 */
int memory_page_record_formatter(memory_page *contents, table_info *tableInfo) {
//    memset(contents->contents, 0, DISK_BOLCK_SIZE);
    /*保留一个整型位置保存slot的使用状态,默认RECORD_PAGE_EMPTY*/
    int recsize = tableInfo->recordLen + INT_SIZE;
    for (int pos = 0; pos + recsize <= DISK_BOLCK_SIZE; pos += recsize) {
        /*先写slot的使用状态*/
        memory_page_setint(contents, pos, RECORD_PAGE_EMPTY);
        /*计算当前记录的偏移量, +slot的使用状态的整型值偏移量*/
        int recoffset = pos + INT_SIZE;
        int count = tableInfo->fieldsName.size() - 1;
        for (int i = 0; i <= count; i++) {
            char *fieldName = tableInfo->fieldsName.at( i);

            field_info *fieldInfo = tableInfo->fields->find(fieldName)->second;

            int offset = tableInfo->offsets->find(fieldInfo->hashCode)->second;

            if (fieldInfo->type == DATA_TYPE_INT) {
                memory_page_setint(contents, recoffset + offset, 0);
            } else {
                memory_page_setstring(contents, recoffset + offset, " ");
            }
        }
    }
}

int memory_page_getint(memory_page *memoryPage, int offset) {
    return bytes2int(memoryPage->contents[offset],
                     memoryPage->contents[offset + 1],
                     memoryPage->contents[offset + 2],
                     memoryPage->contents[offset + 3]
    );

};

int memory_page_setint(memory_page *memoryPage, int offset, int val) {
    memoryPage->contents[offset] = val >> 24;
    memoryPage->contents[offset + 1] = val >> 16;
    memoryPage->contents[offset + 2] = val >> 8;
    memoryPage->contents[offset + 3] = val;
};

/**
 * 先读取字符串长度，再根据长度读取字符串
 * @param memoryPage
 * @param offset 偏移量
 * @param val 返回结果
 * @return 返回状态
 */
int memory_page_getstring(memory_page *memoryPage, int offset, char *val) {
    int len = memory_page_getint(memoryPage, offset);
    /*增加一个整型的偏移量*/
    offset += INT_SIZE;
//    val = (char *) malloc(len);
//    memset(val, 0, len);
    memcpy(val, memoryPage->contents + offset , len);
    val[len]='\0';
    return 1;
};

/**
 * 写字符串
 * @param memoryPage
 * @param offset 偏移量
 * @param val 要写的值
 * @return 返回状态
 */
int memory_page_setstring(memory_page *memoryPage, int offset, const char *val) {
    int len = strlen(val);
    /*先写字符串长度*/
    memory_page_setint(memoryPage, offset, len);
    /*增加一个整型的偏移量*/
    offset += INT_SIZE;
    memcpy(memoryPage->contents + offset , val, len);
    return 1;
};
