#pragma once

#include "cpp11.hpp"

#include <Rdefines.h>
#include <R_ext/Altrep.h>

#include "duckdb.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/common/unordered_map.hpp"
#include "duckdb/parser/tableref/table_function_ref.hpp"
#include "duckdb/common/mutex.hpp"

namespace duckdb {

typedef unordered_map<std::string, SEXP> arrow_scans_t;

struct DBWrapper {
	unique_ptr<DuckDB> db;
	arrow_scans_t arrow_scans;
	mutex lock;
};

struct ConnWrapper {
	unique_ptr<Connection> conn;
	SEXP db_sexp;
};

struct RStatement {
	unique_ptr<PreparedStatement> stmt;
	vector<Value> parameters;
};

void ConnDeleter(ConnWrapper*);
void DBDeleter(DBWrapper*);

typedef cpp11::external_pointer<DBWrapper, DBDeleter> db_eptr_t;
typedef cpp11::external_pointer<ConnWrapper, ConnDeleter> conn_eptr_t;
typedef cpp11::external_pointer<RStatement> stmt_eptr_t;

SEXP DuckDBExecuteArrow(SEXP query_resultsexp, SEXP streamsexp, SEXP vector_per_chunksexp,
                               SEXP return_tablesexp);

SEXP DuckDBRecordBatchR(SEXP query_resultsexp, SEXP approx_batch_sizeexp);

void RegisterArrow(SEXP connsexp, SEXP namesexp, SEXP export_funsexp, SEXP valuesexp);

void UnregisterArrow(SEXP connsexp, SEXP namesexp);

SEXP PointerToString(SEXP extptr);

// internal
unique_ptr<TableFunctionRef> ArrowScanReplacement(const std::string &table_name, void *data);

SEXP StringsToSexp(vector<std::string> s);

SEXP ToUtf8(SEXP string_sexp);

struct RProtector {
	RProtector() : protect_count(0) {
	}
	~RProtector() {
		if (protect_count > 0) {
			UNPROTECT(protect_count);
		}
	}

	SEXP Protect(SEXP sexp) {
		protect_count++;
		return PROTECT(sexp);
	}

private:
	int protect_count;
};

struct DataFrameScanFunction : public TableFunction {
	DataFrameScanFunction();
};

struct RStrings {
	SEXP secs; // Rf_mkChar
	SEXP mins;
	SEXP hours;
	SEXP days;
	SEXP weeks;
	SEXP POSIXct;
	SEXP POSIXt;
	SEXP UTC_str; // Rf_mkString
	SEXP Date_str;
	SEXP factor_str;
	SEXP difftime_str;
	SEXP secs_str;
	SEXP arrow_str; // StringsToSexp
	SEXP POSIXct_POSIXt_str;
	SEXP enc2utf8_sym; // Rf_install
	SEXP tzone_sym;
	SEXP units_sym;
	SEXP getNamespace_sym;
	SEXP Table__from_record_batches_sym;
	SEXP ImportSchema_sym;
	SEXP ImportRecordBatch_sym;
	SEXP ImportRecordBatchReader_sym;

	static const RStrings &get() {
		// On demand
		static RStrings strings;
		return strings;
	}

private:
	RStrings();
};

} // namespace duckdb

// moved out of duckdb namespace for the time being (r-lib/cpp11#262)

duckdb::db_eptr_t rapi_startup(std::string, bool, cpp11::list);

void rapi_shutdown(duckdb::db_eptr_t);

duckdb::conn_eptr_t rapi_connect(duckdb::db_eptr_t);

void rapi_disconnect(duckdb::conn_eptr_t);

cpp11::list rapi_prepare(duckdb::conn_eptr_t, std::string);

cpp11::list rapi_bind(duckdb::stmt_eptr_t, SEXP paramsexp, bool);

SEXP rapi_execute(duckdb::stmt_eptr_t, bool);

void rapi_release(duckdb::stmt_eptr_t);

void rapi_register_df(duckdb::conn_eptr_t, std::string, cpp11::data_frame);

void rapi_unregister_df(duckdb::conn_eptr_t, std::string);
