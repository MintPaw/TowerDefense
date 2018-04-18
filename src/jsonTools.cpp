#include "jsonTools.h"
#include "json.h"

json_value_s *parseJson(const void *src, int len) {
	json_value_s *json = json_parse(src, len);
	return json;
}

int getJsonLength(json_value_s *json) {
	json_object_s *object = (json_object_s *)json->payload;
	return object->length;
}
