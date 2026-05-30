/* *****************************************************************************
 * json.h v0.0.0000
 * 
 * json read/write 
 * not yet.
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef JSON_INCLUDED_
#define JSON_INCLUDED_

typedef struct {
  void *nothing;
} JSON;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


int json_parse(JSON *, const char *);
int json_free(JSON*);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //JSON_INCLUDED_