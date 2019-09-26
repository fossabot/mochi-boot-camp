#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <string.h>

#include "types.h"
#include "kclangc.h"

typedef struct {
  KCDB* db;
} server_data;

static void set(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(set)

static void get(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(get)

static void rm(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(rm)

int main(int argc, char** argv)
{

  /* create the database object */
  KCDB* db;
  db = kcdbnew();

  /* open the database */
  if (!kcdbopen(db, "kyoto-mochi-storage.kch", KCOWRITER | KCOCREATE)) {
    fprintf(stderr, "[main] open error: %s\n", kcecodename(kcdbecode(db)));
  }

  margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
  assert(mid);

  server_data svr_data = {
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
  margo_register_data(mid, get_rpc_id, &svr_data, NULL);

  hg_id_t rm_rpc_id = MARGO_REGISTER(mid, "rm", rm_in_t, rm_out_t, rm);
  margo_register_data(mid, rm_rpc_id, &svr_data, NULL);

  margo_wait_for_finalize(mid);

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

  ret = margo_respond(h, &out);
  assert(ret == HG_SUCCESS);

  ret = margo_free_input(h, &in);
  assert(ret == HG_SUCCESS);

  ret = margo_destroy(h);
  assert(ret == HG_SUCCESS);
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

  char *kbuf, *vbuf;
  size_t ksiz, vsiz;

  out.value = kcdbget(svr_data->db, in.key, strlen(in.key), &vsiz);
  printf("[get] (%s) => %s\n", in.key, out.value);
  if (!out.value) {
    fprintf(stderr, "[get] get error: %s\n", kcecodename(kcdbecode(svr_data->db)));
  }

  ret = margo_respond(h, &out);
  assert(ret == HG_SUCCESS);

  ret = margo_free_input(h, &in);
  assert(ret == HG_SUCCESS);

  ret = margo_destroy(h);
  assert(ret == HG_SUCCESS);

  kcfree(vbuf);
}
DEFINE_MARGO_RPC_HANDLER(get)

static void rm(hg_handle_t h)
{
  printf("[rm] rm() RPC hander\n");
  hg_return_t ret;

  rm_in_t in;
  rm_out_t out;

  margo_instance_id mid = margo_hg_handle_get_instance(h);

  const struct hg_info* info = margo_get_info(h);
  server_data* svr_data = (server_data*)margo_registered_data(mid, info->id);

  ret = margo_get_input(h, &in);
  assert(ret == HG_SUCCESS);

  char *kbuf, *vbuf;
  size_t ksiz, vsiz;

  out.value = kcdbremove(svr_data->db, in.key, strlen(in.key));
  printf("[rm] (%s) => %d\n", in.key, out.value);
  if (!out.value) {
    fprintf(stderr, "[rm] get error: %s\n", kcecodename(kcdbecode(svr_data->db)));
  }

  ret = margo_respond(h, &out);
  assert(ret == HG_SUCCESS);

  ret = margo_free_input(h, &in);
  assert(ret == HG_SUCCESS);

  ret = margo_destroy(h);
  assert(ret == HG_SUCCESS);

  kcfree(vbuf);
}
DEFINE_MARGO_RPC_HANDLER(rm)
