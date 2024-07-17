#ifndef DS_MGR_H
#define DS_MGR_H

#include "mbuf_header.h"

const int PAGE_SIZE = 4 * 1024;
const int NUM_PAGE_TOTAL = 50001;

const int CONTENT_ITEM_SIZE = 4;
const int CONTENT_MAX_SIZE = (PAGE_SIZE / CONTENT_ITEM_SIZE);
const int PAGE_NUM_IN_CONTENT = (CONTENT_MAX_SIZE - 1);

struct DataStorageMgr {
private:
    FILE *currFile;
    int numPages;
    long long io_cnt_total;

    int cur_content_id;
    off_t cur_content_offset;
    off_t contents[CONTENT_MAX_SIZE];

    string filename = "data.dbf";
    string filepath = "File/";

    void OpenFile(string filename, bool write) {
        if (write)
            currFile = fopen((filepath + filename).c_str(), "wb+");
        else
            currFile = fopen((filepath + filename).c_str(), "rb+");
    }
    void CloseFile() {
        fclose(currFile);
    }
    void Seek(off_t offset) {
        int ret = fseek(currFile, offset, SEEK_SET);
    }
    void IncIOCnt() {
        io_cnt_total += 1;
    }
    void FRead(void *dst, size_t sz, size_t cnt, FILE *fp) {
        int ret = fread(dst, sz, cnt, fp);

        IncIOCnt();
    }
    void FWrite(void *str, size_t sz, size_t cnt, FILE *fp) {
        int ret = fwrite(str, sz, cnt, fp);

        IncIOCnt();
    }
    void ClearContents() {
        memset(contents, 0, sizeof(contents));
        cur_content_id = -1;
        cur_content_offset = 0;
    }
    void ReadContent(off_t offset, int index) {
        Seek(offset);
        ClearContents();
        FRead(contents, 1, sizeof(contents), currFile);
        cur_content_id = index;
        cur_content_offset = offset;
    }
    void WriteContent(off_t offset, int index) {
        Seek(offset);
        FWrite(contents, 1, sizeof(contents), currFile);
        cur_content_id = index;
        cur_content_offset = offset;
    }
    off_t SkipContent(int num_skip) {
        if (cur_content_id == num_skip) {
            return cur_content_offset;
        }
        off_t cont_off = 0;
        int i = 0;
        if (cur_content_id != -1 && cur_content_id < num_skip) {
            cont_off = contents[CONTENT_MAX_SIZE - 1];
            i = cur_content_id + 1;
        }
        for (; i <= num_skip; i++) {
            ReadContent(cont_off, i);
            if (i == num_skip)
                return cont_off;
            cont_off = contents[CONTENT_MAX_SIZE - 1];
        }
        return cont_off;
    }
    void IncNumpage() {
        numPages += 1;
    }

public:
    DataStorageMgr(bool create_file = true) {
        currFile = nullptr;
        numPages = 0;
        io_cnt_total = 0;
        ClearContents();
        OpenFile(filename, create_file);
    }
    ~DataStorageMgr() {
        CloseFile();
    }
    int ReadPage(int page_id, char* buffer) {
        int num_skip = page_id / PAGE_NUM_IN_CONTENT;
        SkipContent(num_skip);
        off_t target_offset = contents[page_id % PAGE_NUM_IN_CONTENT];
        Seek(target_offset);
        FRead(buffer, 1, MBUFFER_SIZE, currFile);
        return 0;
    }
    int WritePage(int page_id, char* buffer) {
        int num_skip = page_id / PAGE_NUM_IN_CONTENT;
        SkipContent(num_skip);
        off_t target_offset = contents[page_id % PAGE_NUM_IN_CONTENT];
        Seek(target_offset);
        FWrite(buffer, 1, MBUFFER_SIZE, currFile);
        return 0;
    }
    int WriteNewPage(char* buffer) {
        if (numPages == 0) {
            ClearContents();
            const off_t target_offset = sizeof(contents);
            contents[0] = target_offset;
            WriteContent(0, 0);
            Seek(target_offset);
            FWrite(buffer, 1, MBUFFER_SIZE, currFile);
            IncNumpage();
            return 0;
        }
        const int page_id = numPages;
        int num_skip = page_id / PAGE_NUM_IN_CONTENT;
        int target_index = page_id % PAGE_NUM_IN_CONTENT;
        if (target_index == 0) {
            num_skip -= 1;
            off_t cont_offset = SkipContent(num_skip);
            off_t new_content_offset = contents[PAGE_NUM_IN_CONTENT - 1] + PAGE_SIZE;
            contents[CONTENT_MAX_SIZE - 1] = new_content_offset;
            WriteContent(cont_offset, num_skip);
            ClearContents();
            off_t new_page_offset = new_content_offset + sizeof(contents);
            contents[0] = new_page_offset;
            WriteContent(new_content_offset, num_skip + 1);
            Seek(new_page_offset);
            FWrite(buffer, 1, MBUFFER_SIZE, currFile);
        } else {
            off_t cont_offset = SkipContent(num_skip);
            off_t target_offset = contents[target_index - 1] + PAGE_SIZE;
            contents[target_index] = target_offset;
            WriteContent(cont_offset, num_skip);
            Seek(target_offset);
            FWrite(buffer, 1, MBUFFER_SIZE, currFile);
        }
        IncNumpage();
        return 0;
    }
    int GetNumPages() {
        return numPages;
    }
    int GetTotalIO() {
        return io_cnt_total;
    }
    long Ftell() {
        long ret = ftell(currFile);
        return ret;
    }

};

#endif // DS_MGR_H
