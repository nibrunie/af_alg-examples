#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>
#include <string.h>

#ifndef SOL_ALG
#define SOL_ALG 279
#endif

int main(void)
{
  int opfd;
  int tfmfd;
  struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "cbc(aes)"
  };
  struct msghdr msg = {};
  struct cmsghdr *cmsg;
  char cbuf[CMSG_SPACE(4) + CMSG_SPACE(20)] = {0};
  char buf[16];
  struct af_alg_iv *iv;
  struct iovec iov;
  int i;

  tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

  setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY,
       "\x06\xa9\x21\x40\x36\xb8\xa1\x5b"
       "\x51\x2e\x03\xd5\x34\x12\x00\x06", 16);

  opfd = accept(tfmfd, NULL, 0);

  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof(cbuf);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_OP;
  cmsg->cmsg_len = CMSG_LEN(4);
  *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_ENCRYPT;

  cmsg = CMSG_NXTHDR(&msg, cmsg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_IV;
  cmsg->cmsg_len = CMSG_LEN(20);
  iv = (void *)CMSG_DATA(cmsg);
  iv->ivlen = 16;
  memcpy(iv->iv, "\x3d\xaf\xba\x42\x9d\x9e\xb4\x30"
           "\xb4\x22\xda\x80\x2c\x9f\xac\x41", 16);

  iov.iov_base = "Single block msg";
  iov.iov_len = 16;

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  sendmsg(opfd, &msg, 0);
  read(opfd, buf, 16);

  for (i = 0; i < 16; i++) {
    printf("%02x", (unsigned char)buf[i]);
  }
  printf("\n");

  close(opfd);
  close(tfmfd);

  return 0;
}
