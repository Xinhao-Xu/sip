#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

int uploadCurrentResult();
int ftpUpload(char *chUploadFile);
int uploadFileCreate();

int db_process();
int db_combine_item();
int db_export();
unsigned long int rand_int(int low, int hi);

int ftp_trans();


typedef struct _tagSQL_DB
{
	int			summary_result;
	char		exp_name[64];		//store db export file(.txt)name
}SQL_DB,*PSQL_DB;



extern SQL_DB sql_db;


#endif

