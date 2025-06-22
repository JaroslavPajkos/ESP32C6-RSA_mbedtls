/*
	AUTHOR: Jaroslav Pajkos
	PROJECT: RSA (2048-4096) digital signature
	*******************************************
	2025
	*******************************************
*/
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <stdio.h>
#include <string.h>

#include "esp_task_wdt.h"

#define HASH_LEN 32
#define MAX_KEY_SIZE 4096
#define MAX_SIG_LEN (MAX_KEY_SIZE / 8)

// Start cycle counter
static inline uint32_t start() {
 uint32_t value;
 asm volatile(
     "csrw 0x7E0, %1\n\t"
     "csrw 0x7E1, %1\n\t"
     "csrr %0, 0x7E2\n\t"
     : "=r"(value)
     : "r"(1));
 return value;
}

// End cycle counter
static inline uint32_t end() {
 uint32_t end;
 asm volatile("csrr %0, 0x7E2\n\t" : "=r"(end));
 return end;
}

// Print buffer in hex format
static void print_hex(const char *label, const unsigned char *buf,
                      size_t len) {
 printf("\n");
}

void app_main(void) {
 // Disable watchdog timer
 esp_task_wdt_deinit();

 // Initialize variables and contexts
 int ret;
 mbedtls_pk_context pk;
 mbedtls_rsa_context rsa;
 mbedtls_entropy_context entropy;
 mbedtls_ctr_drbg_context ctr_drbg;
 const char *pers = "rsa_time_comparison";
 unsigned char hash[HASH_LEN];
 unsigned char signature_pkcs1[MAX_SIG_LEN];
 unsigned char signature_pss[MAX_SIG_LEN];
 size_t sig_len = 0;
 const char *message = "OPEN MESSAGE FOR ENCRYPTION";
 const int key_sizes[] = {2048, 3072};
 const int num_key_sizes = sizeof(key_sizes) / sizeof(key_sizes[0]);

 printf("\nStarting RSA Time Comparison\n");

 // Initialize mbedtls contexts
 mbedtls_pk_init(&pk);
 mbedtls_rsa_init(&rsa);
 mbedtls_entropy_init(&entropy);
 mbedtls_ctr_drbg_init(&ctr_drbg);

 // Seed random number generator
 if ((ret = mbedtls_ctr_drbg_seed(
          &ctr_drbg, mbedtls_entropy_func, &entropy,
          (const unsigned char *)pers, strlen(pers))) != 0) {
  printf("Failed to seed RNG: -0x%04x\n", -ret);
  goto exit;
 }

 // Compute SHA-256 hash of message
 if ((ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                       (const unsigned char *)message,
                       strlen(message), hash)) != 0) {
  printf("Failed to compute SHA-256 hash: -0x%04x\n", -ret);
  goto exit;
 }
 print_hex("Message hash", hash, HASH_LEN);

 // Test RSA for each key size
 for (int i = 0; i < num_key_sizes; i++) {
  int key_size = key_sizes[i];
  printf("\n=== Testing RSA-%d ===\n", key_size);

  // Set up public key context
  if ((ret = mbedtls_pk_setup(
           &pk, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA))) != 0) {
   printf("Failed to set up PK context: -0x%04x\n", -ret);
   goto exit;
  }

  // Generate RSA key
  mbedtls_rsa_context *rsa_pk = mbedtls_pk_rsa(pk);
  if ((ret = mbedtls_rsa_gen_key(rsa_pk, mbedtls_ctr_drbg_random,
                                 &ctr_drbg, key_size, 65537)) != 0) {
   printf("Failed to generate RSA-%d key: -0x%04x\n", key_size, -ret);
   goto exit;
  }

  // Copy RSA context
  mbedtls_rsa_init(&rsa);
  mbedtls_rsa_copy(&rsa, rsa_pk);

  // Sign with PKCS#1 v1.5 padding
  mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V15,
                          MBEDTLS_MD_SHA256);
  printf("Signing message (PKCS#1 v1.5)...\n");
  if ((ret = mbedtls_rsa_pkcs1_sign(
           &rsa, mbedtls_ctr_drbg_random, &ctr_drbg,
           MBEDTLS_MD_SHA256, HASH_LEN, hash, signature_pkcs1)) !=
      0) {
   printf("Failed to sign message (PKCS#1 v1.5): -0x%04x\n", -ret);
   goto exit;
  }
  sig_len = mbedtls_rsa_get_len(&rsa);
  print_hex("PKCS#1 v1.5 Signature", signature_pkcs1, sig_len);

  // Sign with PSS padding
  mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V21,
                          MBEDTLS_MD_SHA256);
  printf("Signing message (PSS)...\n");
  if ((ret = mbedtls_rsa_rsassa_pss_sign(
           &rsa, mbedtls_ctr_drbg_random, &ctr_drbg,
           MBEDTLS_MD_SHA256, HASH_LEN, hash, signature_pss)) != 0) {
   printf("Failed to sign message (PSS): -0x%04x\n", -ret);
   goto exit;
  }
  sig_len = mbedtls_rsa_get_len(&rsa);
  print_hex("PSS Signature", signature_pss, sig_len);

  // Verify PKCS#1 v1.5 signature with timing
  mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V15,
                          MBEDTLS_MD_SHA256);
  printf(
      "\nMeasuring RSA-%d PKCS#1 v1.5 Verify "
      "(mbedtls_rsa_pkcs1_verify)...\n",
      key_size);
  uint32_t st = start();
  if ((ret =
           mbedtls_rsa_pkcs1_verify(&rsa, MBEDTLS_MD_SHA256, HASH_LEN,
                                    hash, signature_pkcs1)) != 0) {
   printf("Signature verification failed (PKCS#1 v1.5): -0x%04x\n",
          -ret);
   goto exit;
  }
  uint32_t en = end();
  printf(
      "RSA-%d PKCS#1 v1.5 Verify (mbedtls_rsa_pkcs1_verify): %lu "
      "cycles\n",
      key_size, (en - st) / 160);

  // Verify PSS signature with timing
  mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V21,
                          MBEDTLS_MD_SHA256);
  printf(
      "\nMeasuring RSA-%d PSS Verify "
      "(mbedtls_rsa_rsassa_pss_verify)...\n",
      key_size);
  st = start();
  if ((ret = mbedtls_rsa_rsassa_pss_verify(
           &rsa, MBEDTLS_MD_SHA256, HASH_LEN, hash, signature_pss)) !=
      0) {
   printf("Signature verification failed (PSS): -0x%04x\n", -ret);
   goto exit;
  }
  en = end();
  printf(
      "RSA-%d PSS Verify (mbedtls_rsa_rsassa_pss_verify): %lu "
      "cycles\n",
      key_size, (en - st) / 160);

  // Clean up RSA and PK contexts
  mbedtls_rsa_free(&rsa);
  mbedtls_pk_free(&pk);
 }

 printf("\nTime Comparison Completed Successfully!\n");

exit:
 // Free all contexts
 mbedtls_rsa_free(&rsa);
 mbedtls_pk_free(&pk);
 mbedtls_ctr_drbg_free(&ctr_drbg);
 mbedtls_entropy_free(&entropy);

 // Print error details if any
 if (ret != 0) {
  char error_buf[100];
  mbedtls_strerror(ret, error_buf, sizeof(error_buf));
  printf("Error: %s (-0x%04x)\n", error_buf, -ret);
 }
}