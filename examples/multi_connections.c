#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <openssl/sha.h>

#ifndef SOL_ALG
#define SOL_ALG 279
#endif

int buffer_size = 4096;
int iteration = 10;

void test_connection_send(int opfd, unsigned char* input, int* input_sizes)
{
  int j;
  int ret;

  for (j = 0; j < iteration; ++j) {
    int input_size = input_sizes[j];

    //ret = write(opfd, input, input_size); 
    ret = send(opfd, input, input_size, j == (iteration - 1) ? 0 : MSG_MORE); 
    assert(ret > 0 && "write failed");

    input += input_size;
  }
}

void test_connection_receive(int opfd, unsigned char* input, int* input_sizes)
{
  unsigned char buf[32];
  int total = 0;
  int i;
  for (i = 0; i < iteration; ++i) total += input_sizes[i];
  // computing expected 
  unsigned char expected_digest[32];
  SHA256(input, total, expected_digest);

  int ret = read(opfd, buf, 32);
  assert(ret > 0 && "read failed");

  printf("result   ");
  for (i = 0; i < 32; i++) {
    printf("%02x", (unsigned char)buf[i]);
  }
  printf("\n");
  printf("expected ");
  for (i = 0; i < 32; i++) {
    printf("%02x", (unsigned char)expected_digest[i]);
  }
  printf("\n");
  assert(!memcmp(expected_digest, buf, 32) && "digest differs from expected");

}


void test_gen_input(unsigned char* input, int* input_sizes)
{
  int j, i;

  for (j = 0; j < iteration; ++j) {
    int size = rand() % (buffer_size / iteration);
    for (i = 0; i < size; ++i) input[i] = rand(); 

    input_sizes[j] = size;

    input += size;
  }
}
void test_connection(int opfd)
{
  unsigned char* init_input = malloc(buffer_size);
  int* input_sizes = malloc(sizeof(int) * iteration);

  test_gen_input(init_input, input_sizes);

  test_connection_send(opfd, init_input, input_sizes);

  test_connection_receive(opfd, init_input, input_sizes);

  free(init_input);
  free(input_sizes);
}


int main(void)
{
  int opfd0, opfd1;
  int tfmfd;
  struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "hash",
    .salg_name = "sha256"
  };

  unsigned char inputs[3][buffer_size];
  int input_sizes[3][10];

  // generating inputs
  int i;
  for (i = 0; i < 3; ++i) test_gen_input(inputs[i], input_sizes[i]);

  tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

  // accepting a new connection
  opfd0 = accept(tfmfd, NULL, 0);

  // accepting a second connection
  opfd1 = accept(tfmfd, NULL, 0);

  test_connection_send(opfd0, inputs[0], input_sizes[0]);

  test_connection_send(opfd1, inputs[1], input_sizes[1]);
  printf("checking test 0\n");
  test_connection_receive(opfd1, inputs[1], input_sizes[1]);
  printf("checking test 1\n");
  test_connection_receive(opfd0, inputs[0], input_sizes[0]);
  test_connection_send(opfd0, inputs[2], input_sizes[2]);
  printf("checking test 2\n");
  test_connection_receive(opfd0, inputs[2], input_sizes[2]);

  close(opfd1);
  close(opfd0);
  close(tfmfd);


  return 0;
}
