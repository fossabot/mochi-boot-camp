#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include "types.h"

int main(int argc, char** argv)
{
  if(argc != 5) {
    fprintf(stderr,"Usage: %s <server address> <rpc_id> <key> <value>\n\n", argv[0]);
    fprintf(stderr,"    rpc_id: 1 = set, 2 = get\n");
    exit(0);
  }

  margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);

  hg_id_t set_rpc_id = MARGO_REGISTER(mid, "set", set_in_t, set_out_t, NULL);
  hg_id_t get_rpc_id = MARGO_REGISTER(mid, "get", get_in_t, get_out_t, NULL);

  hg_addr_t svr_addr;
  margo_addr_lookup(mid, argv[1], &svr_addr);

  set_in_t set_args;
  set_out_t set_resp;
  get_in_t get_args;
  get_out_t get_resp;

  int rpc_id = atoi(argv[2]);
  switch (rpc_id) {
  case 1:
    // Call "set" RPC
    set_args.key = argv[3];
    set_args.value = argv[4];
    
    hg_handle_t h;
    margo_create(mid, svr_addr, set_rpc_id, &h);
    margo_forward(h, &set_args);
    margo_get_output(h, &set_resp);

    printf("[set] Get response: (%s, %s) => %d\n", set_args.key, set_args.value, set_resp.ret);

    margo_free_output(h, &set_resp);
    margo_destroy(h);

    break;

  case 2:
    // Call "get" RPC
    get_args.key = argv[3];

    margo_create(mid, svr_addr, get_rpc_id, &h);
    margo_forward(h, &get_args);
    margo_get_output(h, &get_resp);

    printf("[get] Got response: %s => %s\n", get_args.key, get_resp.value);

    margo_free_output(h, &get_resp);
    margo_destroy(h);    

    break;

  default:
    printf("unknown rpc name");
    break;
  }

  // destroy margo client
  margo_addr_free(mid, svr_addr);
  margo_finalize(mid);

  return 0;
}
