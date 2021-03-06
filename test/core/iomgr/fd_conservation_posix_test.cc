/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <sys/resource.h>

#include <grpc/support/log.h>

#include "src/core/lib/iomgr/endpoint_pair.h"
#include "src/core/lib/iomgr/iomgr.h"
#include "test/core/util/test_config.h"

int main(int argc, char** argv) {
  int i;
  struct rlimit rlim;
  grpc_endpoint_pair p;

  grpc_test_init(argc, argv);
  grpc_init();
  grpc_exec_ctx exec_ctx = GRPC_EXEC_CTX_INIT;

  /* set max # of file descriptors to a low value, and
     verify we can create and destroy many more than this number
     of descriptors */
  rlim.rlim_cur = rlim.rlim_max = 10;
  GPR_ASSERT(0 == setrlimit(RLIMIT_NOFILE, &rlim));
  grpc_resource_quota* resource_quota =
      grpc_resource_quota_create("fd_conservation_posix_test");

  for (i = 0; i < 100; i++) {
    p = grpc_iomgr_create_endpoint_pair("test", nullptr);
    grpc_endpoint_destroy(&exec_ctx, p.client);
    grpc_endpoint_destroy(&exec_ctx, p.server);
    grpc_exec_ctx_flush(&exec_ctx);
  }

  grpc_resource_quota_unref(resource_quota);

  grpc_exec_ctx_finish(&exec_ctx);
  grpc_shutdown();
  return 0;
}
