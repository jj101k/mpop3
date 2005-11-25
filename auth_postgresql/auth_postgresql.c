#include <stdlib.h>
#include <string.h>
#include "pop3.h"
#include "conffile.h"

#include <libpq-fe.h>

#define MAX_PSQL_STRING_LEN 1024

// Example connection: "dbname=test1 user=testuser password=testpassword"
char conn_string[MAX_PSQL_STRING_LEN]="";

// Example query: "SELECT password, mailbox FROM auth WHERE username=$1"
char query_string[MAX_PSQL_STRING_LEN]="";

char set_conn_string(char const *i_conn_string) {
	if(!i_conn_string || !i_conn_string[0]) return 0;
	strncpy(conn_string, i_conn_string, MAX_PSQL_STRING_LEN-1);
	conn_string[MAX_PSQL_STRING_LEN-1]=0;
	return 1;
}

char set_query_string(char const *i_query_string) {
	if(!i_query_string || !i_query_string[0]) return 0;
	strncpy(query_string, i_query_string, MAX_PSQL_STRING_LEN-1);
	query_string[MAX_PSQL_STRING_LEN-1]=0;
	return 1;
}

struct simpleConfig my_config_tags[]={
	{&set_conn_string, "connection"},
	{&set_query_string, "query"},
	{NULL, NULL}
};

void _init() { }

struct simpleConfig *config_hookup () {
	return my_config_tags;
}

char *mailbox=NULL;

// This is safe since we know that _auth_login will follow from a *successful* _auth_password.
char const *_auth_login(char const *username) {
	if(mailbox) return mailbox;
	return username;
}

char const *_auth_password(char const *username) {
	int length=strlen(username);
	if(!conn_string[0]) return NULL;
	if(!query_string[0]) return NULL;
	PGconn *pg_fh=PQconnectdb(conn_string);

	if(!pg_fh) return NULL;

	PGresult *result=PQexecParams(
		pg_fh, query_string,
		1, NULL, &username, &length, NULL,
		1);

	char *password_out=NULL;
	if(PQresultStatus(result)==PGRES_TUPLES_OK) {
		if(PQntuples(result)>0) {
			password_out=PQgetvalue(result, 0, 0);
			if(PQnfields(result)>1) mailbox=PQgetvalue(result, 0, 1);
		}
		PQclear(result);
	}
	PQfinish(pg_fh);
	return password_out;
}
