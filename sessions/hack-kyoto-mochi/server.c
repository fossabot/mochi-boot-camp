#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include "types.h"
#include "kclangc.h"

//// kyoto cabinet
/* call back function for an existing record */
const char* visitfull(const char* kbuf, size_t ksiz,
                      const char* vbuf, size_t vsiz, size_t *sp, void* opq) {
  fwrite(kbuf, 1, ksiz, stdout);
  printf(":");
  fwrite(vbuf, 1, vsiz, stdout);
  printf("\n");
  return KCVISNOP;
}

/* call back function for an empty record space */
const char* visitempty(const char* kbuf, size_t ksiz, size_t *sp, void* opq) {
  fwrite(kbuf, 1, ksiz, stdout);
  printf(" is missing\n");
  return KCVISNOP;
}

//// kyoto cabinet

typedef struct {
    int max_rpcs;
    int num_rpcs;
} server_data;

static void sum(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(sum)

int main(int argc, char** argv)
{

  //// kyoto cabinet

  KCDB* db;
  KCCUR* cur;
  char *kbuf, *vbuf;
  size_t ksiz, vsiz;
  const char *cvbuf;

  /* create the database object */
  db = kcdbnew();

  /* open the database */
  if (!kcdbopen(db, "casket.kch", KCOWRITER | KCOCREATE)) {
    fprintf(stderr, "open error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* store records */
  if (!kcdbset(db, "foo", 3, "hop", 3) ||
      !kcdbset(db, "bar", 3, "step", 4) ||
      !kcdbset(db, "baz", 3, "jump", 4)) {
    fprintf(stderr, "set error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* retrieve a record */
  vbuf = kcdbget(db, "foo", 3, &vsiz);
  if (vbuf) {
    printf("%s\n", vbuf);
    kcfree(vbuf);
  } else {
    fprintf(stderr, "get error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* traverse records */
  cur = kcdbcursor(db);
  kccurjump(cur);
  while ((kbuf = kccurget(cur, &ksiz, &cvbuf, &vsiz, 1)) != NULL) {
    printf("%s:%s\n", kbuf, cvbuf);
    kcfree(kbuf);
  }
  kccurdel(cur);

  /* retrieve a record with visitor */
  if (!kcdbaccept(db, "foo", 3, visitfull, visitempty, NULL, 0) ||
      !kcdbaccept(db, "dummy", 5, visitfull, visitempty, NULL, 0)) {
    fprintf(stderr, "accept error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* traverse records with visitor */
  if (!kcdbiterate(db, visitfull, NULL, 0)) {
    fprintf(stderr, "iterate error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* close the database */
  if (!kcdbclose(db)) {
    fprintf(stderr, "close error: %s\n", kcecodename(kcdbecode(db)));
  }

  /* delete the database object */
  kcdbdel(db);

  //// kyoto cabinet

    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    server_data svr_data = {
        .max_rpcs = 4,
        .num_rpcs = 0
    };

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid,my_address);
    printf("Server running at address %s\n", addr_str);

    hg_id_t rpc_id = MARGO_REGISTER(mid, "sum", sum_in_t, sum_out_t, sum);
    margo_register_data(mid, rpc_id, &svr_data, NULL);

    margo_wait_for_finalize(mid);

    return 0;
}

static void sum(hg_handle_t h)
{
    hg_return_t ret;

    sum_in_t in;
    sum_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    server_data* svr_data = (server_data*)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);
    assert(ret == HG_SUCCESS);

    out.ret = in.x + in.y;
    printf("Computed %d + %d = %d\n",in.x,in.y,out.ret);

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
DEFINE_MARGO_RPC_HANDLER(sum)
