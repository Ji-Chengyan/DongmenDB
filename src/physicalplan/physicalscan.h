//
// Created by Sam on 2018/2/7.
//

#ifndef DONGMENDB_PHYSICALSCAN_H
#define DONGMENDB_PHYSICALSCAN_H

#include <recordfile.h>
#include <sra.h>
#include "physical_scan_table.h"
#include "physical_scan_join_nest_loop.h"
#include "physical_scan_select.h"
#include "physical_scan_project.h"

typedef struct variant_{
    enum data_type type;
    union{
        int intValue;
        char *strValue;
        int booleanValue;
    };
}variant;

typedef enum {
    SCAN_TYPE_JOIN_NEST_LOOP,  //使用嵌套循环实现连接
    SCAN_TYPE_TABLE,
    SCAN_TYPE_SELECT,
    SCAN_TYPE_PROJECT
}scan_type;

typedef struct physical_scan_table_ physical_scan_table;
typedef struct physical_scan_ physical_scan;
typedef struct physical_scan_project_ physical_scan_project;
typedef struct physical_scan_select_ physical_scan_select;
typedef struct physical_scan_join_nest_loop_ physical_scan_join_nest_loop;

typedef int (*physical_scan_before_first)(physical_scan *scan);

typedef int (* physical_scan_next)(physical_scan *scan);

typedef int (* physical_scan_close)(physical_scan *scan);

typedef int (* physical_scan_get_int)(physical_scan *scan, char *fieldName);

typedef int (* physical_scan_get_string)(physical_scan *scan, char *fieldName, char *value);

typedef int (* physical_scan_has_field)(physical_scan *scan, char *fieldName);

typedef int (* physical_scan_set_int)(physical_scan *scan, char *fieldName, int value);

typedef int (* physical_scan_set_string)(physical_scan *scan, char *fieldName, char *value);

typedef int (* physical_scan_delete)(physical_scan *scan);

typedef int (* physical_scan_insert)(physical_scan *scan);

typedef int (* physical_scan_get_rid)(physical_scan *scan, record_id *recordId);

typedef int (* physical_scan_moveto_rid)(physical_scan *scan, record_id *recordId);

typedef struct physical_scan_{
    scan_type scanType;
    union {
        physical_scan_table *physicalScanTable;
        physical_scan_join_nest_loop *physicalScanJoinNestLoop;
        physical_scan_select *physicalScanSelect;
        physical_scan_project *physicalScanProject;
    };
    physical_scan_before_first beforeFirst;
    physical_scan_next next;
    physical_scan_close close;
    physical_scan_get_int getInt;
    physical_scan_get_string getString;
    physical_scan_has_field hasField;
    physical_scan_set_int setInt;
    physical_scan_set_string setString;
    physical_scan_delete delete;
    physical_scan_insert insert;
    physical_scan_get_rid getRid;
    physical_scan_moveto_rid movetoRid;
} physical_scan;

physical_scan *physical_scan_generate(dongmengdb *db, SRA_t *sra, transaction *tx);
Expression *physical_scan_evaluate_expression(Expression *expr, physical_scan *scan, variant *var);

#endif //DONGMENDB_PHYSICALSCAN_H
