//
// Created by sam on 2018/11/15.
//

#ifndef DONGMENDB_TABLESCAN_H
#define DONGMENDB_TABLESCAN_H


#include <physicalplan/Scan.h>

/*表扫描*/
class TableScan : public Scan {

public:
    RecordFile *m_recordFile;
    TableInfo *m_tableInfo;

    TableScan(DongmenDB *db, string tableName, Transaction *tx);

    int beforeFirst();

    int next();

    int close();

    variant *getValueByIndex(int index);

    int getIntByIndex(int index);

    string getStringByIndex(int index);

    int getInt(string tableName, string fieldName);

    variant* getValue(string fieldName);

    string getString(string tableName, string fieldName);

    int hasField(string tableName, string fieldName);

    FieldInfo * getField(string tableName, string fieldName);

    vector<char*> getFieldsName(string tableName);

    int setInt(string tableName, string fieldName, int value);

    int setString(string tableName, string fieldName, string value);

    int deleteRecord();

    int insertRecord();

    int getRID(RecordID *recordID);

    int moveTo(RecordID *recordID);
};

#endif //DONGMENDB_TABLESCAN_H
