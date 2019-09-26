#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <string.h>

#include "types.h"
#include "kclangc.h"

//// kyoto cabinet

/* call back function for an existing record */
const char* visitfull(const char* kbuf, size_t ksiz,
                      const char* vbuf, size_t vsiz, size_t *sp, void* opq) {
  printf("** visitfull() callback\n");
  fwrite(kbuf, 1, ksiz, stdout);
  printf(":");
  fwrite(vbuf, 1, vsiz, stdout);
  printf("\n");
  return KCVISNOP;
}

/* call back function for an empty record space */
const char* visitempty(const char* kbuf, size_t ksiz, size_t *sp, void* opq) {
  printf("** visitempty() callback\n");
  fwrite(kbuf, 1, ksiz, stdout);
  printf(" is missing\n");
  return KCVISNOP;
}

//// END kyoto cabinet

typedef struct {
  int max_rpcs;
  int num_rpcs;
  KCDB* db;
} server_data;

static void set(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(set)

static void get(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(get)

int main(int argc, char** argv)
{

  //// kyoto cabinet

  KCDB* db;
  KCCUR* cur;
  char *kbuf, *vbuf;
  size_t ksiz, vsiz;
  const char *cvbuf;

  /*

  /* retrieve a record 
  printf("* retrieve 'foo'\n");
  vbuf = kcdbget(db, "foo", 3, &vsiz);
  if (vbuf) {
    printf("%s\n", vbuf);
    kcfree(vbuf);
  } else {
    fprintf(stderr, "get error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* traverse records 
  printf("* traverse\n");
  cur = kcdbcursor(db);
  kccurjump(cur);
  while ((kbuf = kccurget(cur, &ksiz, &cvbuf, &vsiz, 1)) != NULL) {
    printf("%s:%s\n", kbuf, cvbuf);
    kcfree(kbuf);
  }
  kccurdel(cur);

  /* retrieve a record with visitor 
  printf("* retrieve one record with visitor\n");
  if (!kcdbaccept(db, "foo", 3, visitfull, visitempty, NULL, 0) ||
      !kcdbaccept(db, "dummy", 5, visitfull, visitempty, NULL, 0)) {
    fprintf(stderr, "accept error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* traverse records with visitor 
  printf("* retrieve records with visitor\n");
  if (!kcdbiterate(db, visitfull, NULL, 0)) {
    fprintf(stderr, "iterate error: %s\n", kcecodename(kcdbecode(db)));
  }

  */

  //// END kyoto cabinet

  /* create the database object */
  db = kcdbnew();

  /* open the database */
  if (!kcdbopen(db, "kyoto-mochi-storage.kch", KCOWRITER | KCOCREATE)) {
    fprintf(stderr, "[main] open error: %s\n", kcecodename(kcdbecode(db)));
  }

  margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
  assert(mid);

  server_data svr_data = {
    .max_rpcs = 4,
    .num_rpcs = 0,
    .db = db
  };

  hg_addr_t my_address;
  margo_addr_self(mid, &my_address);
  char addr_str[128];
  size_t addr_str_size = 128;
  margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
  margo_addr_free(mid,my_address);
  printf("[main] Server running at address %s\n", addr_str);

  hg_id_t set_rpc_id = MARGO_REGISTER(mid, "set", set_in_t, set_out_t, set);
  margo_register_data(mid, set_rpc_id, &svr_data, NULL);

  hg_id_t get_rpc_id = MARGO_REGISTER(mid, "get", get_in_t, get_out_t, get);
  margo_register_data(mid, set_rpc_id, &svr_data, NULL);

  printf("before margo_wait_for_finalize");
  margo_wait_for_finalize(mid);
  printf("after margo_wait_for_finalize");

  /* close the database */
  printf("[main] close the database\n");
  if (!kcdbclose(db)) {
    fprintf(stderr, "[main] close error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* delete the database object */
  kcdbdel(db);

  return 0;
}

static void set(hg_handle_t h)
{
  printf("[set] set() RPC hander\n");
  hg_return_t ret;

  set_in_t in;
  set_out_t out;

  margo_instance_id mid = margo_hg_handle_get_instance(h);

  const struct hg_info* info = margo_get_info(h);
  server_data* svr_data = (server_data*)margo_registered_data(mid, info->id);

  ret = margo_get_input(h, &in);
  assert(ret == HG_SUCCESS);

  /* set a record */
  printf("[set] set a record\n");
  if (!kcdbset(svr_data->db, in.key, strlen(in.key), in.value, strlen(in.value))) {
    fprintf(stderr, "set error: %s\n", kcecodename(kcdbecode(svr_data->db)));
  }
  out.ret = strlen(in.key) + strlen(in.value);
  printf("[set] (%s, %s) => %d\n", in.key, in.value, out.ret);

  /// check input
  char *kbuf, *vbuf;
  size_t ksiz, vsiz;

  printf("[set] retrieve 'kyoto'\n");
  vbuf = kcdbget(svr_data->db, "kyoto", 5, &vsiz);
  if (vbuf) {
    printf("[set] %s\n", vbuf);
    kcfree(vbuf);
  } else {
    fprintf(stderr, "[set] get error: %s\n", kcecodename(kcdbecode(svr_data->db)));
  }
  /// END

  ret = margo_respond(h, &out);
  assert(ret == HG_SUCCESS);

  ret = margo_free_input(h, &in);
  assert(ret == HG_SUCCESS);

  ret = margo_destroy(h);
  assert(ret == HG_SUCCESS);

  svr_data->num_rpcs += 1;
  if(svr_data->num_rpcs == svr_data->max_rpcs) {
    margo_finalize(mid);
  }
}
DEFINE_MARGO_RPC_HANDLER(set)

static void get(hg_handle_t h)
{
  printf("[get] get() RPC hander\n");
  hg_return_t ret;

  get_in_t in;
  get_out_t out;

  margo_instance_id mid = margo_hg_handle_get_instance(h);

  const struct hg_info* info = margo_get_info(h);
  server_data* svr_data = (server_data*)margo_registered_data(mid, info->id);

  ret = margo_get_input(h, &in);
  assert(ret == HG_SUCCESS);

  /// check input
  char *kbuf, *vbuf;
  size_t ksiz, vsiz;

  printf("[get] retrieve 'kyoto'\n");
  vbuf = kcdbget(svr_data->db, "kyoto", 5, &vsiz);
  if (vbuf) {
    printf("[get] %s\n", vbuf);
    kcfree(vbuf);
  } else {
    fprintf(stderr, "[get] get error: %s\n", kcecodename(kcdbecode(svr_data->db)));
  }
  /// END

  /* get a record 
  char *vbuf;
  size_t vsiz;

  printf("[get] get a record\n");
  vbuf = kcdbget(svr_data->db, in.key, strlen(in.key), &vsiz);
  printf("[get] END get a record\n");
  if (vbuf) {
    printf("[get] kcdbget: %s\n", vbuf);
  } else {
    fprintf(stderr, "[get] get error: %s\n", kcecodename(kcdbecode(svr_data->db)));
  }
  */

  out.value = vbuf;
  printf("[get] get record (%s) => %s\n", in.key, out.value);

  ret = margo_respond(h, &out);
  assert(ret == HG_SUCCESS);

  ret = margo_free_input(h, &in);
  assert(ret == HG_SUCCESS);

  ret = margo_destroy(h);
  assert(ret == HG_SUCCESS);

  svr_data->num_rpcs += 1;
  if(svr_data->num_rpcs == svr_data->max_rpcs) {
    margo_finalize(mid);
  }

  kcfree(vbuf);
}
DEFINE_MARGO_RPC_HANDLER(get)
