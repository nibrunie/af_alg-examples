# af_alg-examples
Some examples using user-space API for linux kernel crypto module

## Description
  the first example: cipher_basic is a standard call to symmetric key cipher (skcipher) aes-cbc through af_alg API as described by Herbert Xu in http://lwn.net/Articles/410833/

  stream_hash is an attempt to compute a digest on a buffer while streaming this buffer through multiple calls to write / sendmsg 

  multi_connections opens multiple connections on a single socket and interleaves send and recv between those connections. As long as every send/recv pair on a single connection are made in order it seems several connections can be maintained concurrently
