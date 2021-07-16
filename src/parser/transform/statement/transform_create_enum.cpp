#include "duckdb/parser/parsed_data/create_enum_info.hpp"
#include "duckdb/parser/transformer.hpp"
#include "duckdb/parser/statement/create_statement.hpp"
namespace duckdb {

vector<string> ReadPgListToString(duckdb_libpgquery::PGList *column_list) {
	vector<string> result;
	if (!column_list){
		return result;
	}
	for (auto c = column_list->head; c != nullptr; c = lnext(c)) {
		auto target = (duckdb_libpgquery::PGResTarget *)(c->data.ptr_value);
		result.emplace_back(target->name);
	}
	return result;
}

unique_ptr<CreateStatement> Transformer::TransformCreateEnum(duckdb_libpgquery::PGNode *node) {
	auto stmt = reinterpret_cast<duckdb_libpgquery::PGCreateEnumStmt *>(node);
	D_ASSERT(stmt);
	auto result = make_unique<CreateStatement>();
	auto info = make_unique<CreateEnumInfo>();
	info->name = ReadPgListToString(stmt->typeName)[0];
	info->values = ReadPgListToString(stmt->vals);
	result->info = move(info);
	return result;
}
} // namespace duckdb