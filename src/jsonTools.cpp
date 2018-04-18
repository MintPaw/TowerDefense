#include "jsonTools.h"
#include "json.h"

struct Json {
	json_value_s *root;
};

Json *parseJson(const char *src, int len) {
	Json *json = (Json *)malloc(sizeof(Json));
	json->root = json_parse(src, len);
	return json;
}
