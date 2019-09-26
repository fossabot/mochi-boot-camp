#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include "types.h"

int main(int argc, char** argv)
{
  if(argc != 4) {
    fprintf(stderr,"Usage: %s <server address> <key> <value>\n", argv[0]);
    exit(0);
  }

  margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);

  hg_id_t set_rpc_id = MARGO_REGISTER(mid, "set", set_in_t, set_out_t, NULL);
  hg_id_t get_rpc_id = MARGO_REGISTER(mid, "get", get_in_t, get_out_t, NULL);

  hg_addr_t svr_addr;
  margo_addr_lookup(mid, argv[1], &svr_addr);

  // Call "set" RPC
  set_in_t set_args;
  set_args.key = argv[2];
  set_args.value = argv[3];

  hg_handle_t h;
  margo_create(mid, svr_addr, set_rpc_id, &h);
  margo_forward(h, &set_args);

  set_out_t resp;
  margo_get_output(h, &resp);

  printf("[set] Get response: (%s, %s) => %d\n", set_args.key, set_args.value, resp.ret);

  margo_free_output(h,&resp);
  margo_destroy(h);    

  // Call "get" RPC
  get_in_t get_args;
  get_args.key = argv[2];

  margo_create(mid, svr_addr, get_rpc_id, &h);
  margo_forward(h, &get_args);

  get_out_t get_resp;
  margo_get_output(h, &get_resp);

  printf("[get] Got response: %s => %s\n", get_args.key, get_resp.value);

  margo_free_output(h, &get_resp);
  margo_destroy(h);    

  // destroy margo client
  margo_addr_free(mid, svr_addr);

  margo_finalize(mid);

  return 0;
}
