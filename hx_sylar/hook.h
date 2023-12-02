//
// Created by hx on 23-11-27.
//

#ifndef hx_sylar_HOOK_H
#define hx_sylar_HOOK_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
namespace hx_sylar {
auto is_hook_enable() -> bool;
void set_hook_enable(bool flag);

}  // namespace hx_sylar
extern "C" {
//sleep
using sleep_fun = unsigned int (*)(unsigned int);
extern sleep_fun sleep_f;

using usleep_fun = int (*)(useconds_t);
extern usleep_fun usleep_f;

using nanosleep_fun = int (*)(const struct timespec *, struct timespec *);
extern nanosleep_fun nanosleep_f;

//socket
using socket_fun = int (*)(int, int, int);
extern socket_fun socket_f;

using connect_fun = int (*)(int, const struct sockaddr *, socklen_t);
extern connect_fun connect_f;

using accept_fun = int (*)(int, struct sockaddr *, socklen_t *);
extern accept_fun accept_f;

//read
using read_fun = ssize_t (*)(int, void *, size_t);
extern read_fun read_f;

using readv_fun = ssize_t (*)(int, const struct iovec *, int);
extern readv_fun readv_f;

using recv_fun = ssize_t (*)(int, void *, size_t, int);
extern recv_fun recv_f;

using recvfrom_fun = ssize_t (*)(int, void *, size_t, int, struct sockaddr *, socklen_t *);
extern recvfrom_fun recvfrom_f;

using recvmsg_fun = ssize_t (*)(int, struct msghdr *, int);
extern recvmsg_fun recvmsg_f;

//write
using write_fun = ssize_t (*)(int, const void *, size_t);
extern write_fun write_f;

using writev_fun = ssize_t (*)(int, const struct iovec *, int);
extern writev_fun writev_f;

using send_fun = ssize_t (*)(int, const void *, size_t, int);
extern send_fun send_f;

using sendto_fun = ssize_t (*)(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
extern sendto_fun sendto_f;

using sendmsg_fun = ssize_t (*)(int, const struct msghdr *, int);
extern sendmsg_fun sendmsg_f;

using close_fun = int (*)(int);
extern close_fun close_f;

//
using fcntl_fun = int (*)(int, int, ...);
extern fcntl_fun fcntl_f;

using ioctl_fun = int (*)(int, unsigned long, ...);
extern ioctl_fun ioctl_f;

using getsockopt_fun = int (*)(int, int, int, void *, socklen_t *);
extern getsockopt_fun getsockopt_f;

using setsockopt_fun = int (*)(int, int, int, const void *, socklen_t);
extern setsockopt_fun setsockopt_f;

extern int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms);

}

#endif  // hx_sylar_HOOK_H
