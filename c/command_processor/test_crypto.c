/*
 * Copyright (C) 2017 SilkID
 *
 * test_usb_buf.c: USB bulk send test, sets buffer size and pattern to read back
 * and check
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "cmd.h"
#include "tests.h"

#include "libfpsensor.h"
#include "libfpsensordef.h"

#define UECC_CURVE              (uECC_secp256r1())

#define print_buffer(str, ptr, size) {		\
		printf(str"\n");		\
		dump_buffer((void*)ptr, size, 20);	\
}

static void help(void)
{
	fprintf(stderr, "Usage: %s <-c> <-s>\n"
		"c - Test Challenge Response stage (default - true)\n"
		"s - Test Session Key generation stage (default - false)\n"
		"n - Repeat the above test 'n' times  with above options\n"
		"\t\t n < 10\n"
		"", "test_crypto");
}

static int test_crypto_chlg_sig(conf_t *conf)
{
#if 0
	HANDLE *handle = conf->handle;

	int i;
	int ret = 0;
	int cert_num = 0;
	int pub_key_offset = 223;
	int uecc_status;
	silk_crypto_certificate_t cert;
	silk_crypto_random_challenge_t challenge;
	silk_crypto_signature_t signature;

	memset(&cert, 0, sizeof(silk_crypto_certificate_t));
	printf("%s before getCert\n", __func__);

	sensorSetParameter(handle, SENSOR_PARAM_CODE_TEST_ATMEL_ECC, 1);
	usleep(1000 * 1000);

	ret = sensorGetDeviceCertificate(handle, cert_num, &cert);
	if (ret < 0) {
                printf("%s getCert failed\n", __func__);
		goto out;
	} else {
                printf("%s getCert passed\n", __func__);
		print_buffer("Get cert data:", &cert, sizeof(cert));
	}
	print_buffer("\nDevice cert public key:", &cert.certificate[pub_key_offset], 64);

	// Generate a ramdom challenge
	for (i = 0; i < SILK_CRYPTO_RANDOM_LEN; i++)
                challenge.random_challenge[i] = get_random_num() & 0xff;

	print_buffer("\nChallenge:", &challenge, sizeof(challenge));
	ret = sensorGenerateSignatureForChallenge(handle, cert_num, &challenge, &signature);
	if (ret < 0) {
                printf("sensorGenerateSignatureForChallenge failed\n");
		goto out;
	} else
		printf("\nsensorGenerateSignatureForChallenge done\n");

	print_buffer("Signature1:", &signature, sizeof(signature));
	uecc_status = uECC_verify(&cert.certificate[pub_key_offset],
				  challenge.random_challenge,
				  SILK_CRYPTO_RANDOM_LEN,
				  signature.signature, UECC_CURVE);
	printf("\n########### uecc_status=%d ###########\n", uecc_status);
	fflush(stdout);

out:
#endif
	return 0;
}

static int test_crypto_session_key(conf_t *conf)
{
#if 0
	HANDLE *handle = conf->handle;

	int status = 0;
	int uecc_status = 0;
	uint8_t pmk[ATCA_KEY_SIZE] = { 0,};
	size_t kdf_input_len = ATCA_KEY_SIZE + RANDOM_NUM_SIZE;
	uint8_t kdf_input[ATCA_KEY_SIZE + RANDOM_NUM_SIZE] = { 0,};
	uint8_t kdf_output[ATCA_KEY_SIZE] = { 0,};
	uint8_t aes_enc_key[SILK_CRYPTO_AES_KEY_LEN] = { 0,};
	uint8_t host_priv[ATCA_KEY_SIZE] = { 0,};
	silk_crypto_ecdhe_info_t ecdhe_info;
	silk_crypto_public_key_t host_public_key;
#if 0
	size_t kdf_input_len = ATCA_KEY_SIZE + RANDOM_NUM_SIZE;
#endif

	// Host generates a unique NIST P256 public key.
	uecc_status = uECC_make_key(host_public_key.key, host_priv, UECC_CURVE);
	if (uecc_status != 1) {
                printf("uECC_make_key failed\n");
		goto out;
	}
	print_buffer("host_pub_key:", &host_public_key.key[0], sizeof(host_public_key.key));
	fflush(stdout);

	// Initiate the session.  The module public key and a random number will be returned
	status = sensorCryptoInitiateSession(handle, &ecdhe_info, &host_public_key);
	if (status < 0) {
                printf("sensorCryptoInitiateSession failed\n");
		goto out;
	}
	printf("########### status=%d ###########\n", status);
	print_buffer("Device Public Key:", &ecdhe_info.ecdhe_pub_key[0],
		     sizeof(ecdhe_info.ecdhe_pub_key));

	print_buffer("\nDevice Random Number:", &ecdhe_info.random_num[0], sizeof(ecdhe_info.random_num));

	// Use a host side ECDH function to get the Pre Master Secret...
	uecc_status = uECC_shared_secret(ecdhe_info.ecdhe_pub_key, host_priv, pmk, UECC_CURVE);
	if (uecc_status != 1) {
                printf("uECC_shared_secret failed\n");
		goto out;
	}
	printf("\n########### uecc_status=%d ###########\n", uecc_status);

	// Concat the pmk and random_num to create the input for the KDF
	memcpy(&kdf_input[0], pmk, ATCA_KEY_SIZE);
	memcpy(&kdf_input[ATCA_KEY_SIZE], ecdhe_info.random_num, RANDOM_NUM_SIZE);

#if 1
	// Use a int atcac_sw_sha2_256(const uint8_t * data, size_t data_size,
	//                              uint8_t digest[ATCA_SHA2_256_DIGEST_SIZE]);
	// TODO: Not clear right now !!!, commenting
	status = atcac_sw_sha2_256(kdf_input, kdf_input_len, kdf_output);
	printf("########### status=%d ###########\n", status);
#endif
	// Copy SILK_CRYPTO_AES_KEY_LEN bytes into the global encryption key
	memcpy(&aes_enc_key[0], kdf_output, SILK_CRYPTO_AES_KEY_LEN);
	print_buffer("aes_enc_key:", &aes_enc_key[0], sizeof(aes_enc_key));
	printf("\n");

	return 0;
out:
#endif
	return -1;
}

static int test_crypto(cmd_t *cmd, conf_t *conf, int argc, char *argv[])
{

	int ret = 0;
	int opt, i;
	int ntimes = 1;
	bool chlg_resp = true, session_key = false;

	printf("%s argc: %d argv[0]:%s argv[1]:%s\n", __func__, argc, argv[0], argv[1]);
	while ((opt = getopt(argc, argv, "csn:")) != -1) {
		switch (opt) {
		case 'c':
			chlg_resp = true;
			break;
		case 'n':
			ntimes = atoi(optarg);
			if (ntimes > 10) {
				printf("ntimes > 10, setting back to 10\n");
				ntimes = 10;
			}
			break;
		case 's':
			session_key = true;
			break;

		default: /* '?' */
			help();
			goto err_out;
		}
	}

	//sensorSetPowerMgmt(handle, 0);	/* Disable low-power mode */

	for (i = 0; i < ntimes; i++) {
		if (chlg_resp) {
			test_crypto_chlg_sig(conf);
		}

		if (session_key) {
			test_crypto_session_key(conf);
		}

		sleep(1);
	}

err_out:
	return ret;
}

int test_crypto_register(void)
{
	int ret = 0;

	cmd_ops_t test_cmd_ops = {
		.func = test_crypto,
		.help = help,
	};

	cmd_t test_cmd = {
		.name = "test_crypto",
		.desc = "Test Crypto Challenge/Signature and Session Key generation",
		.ops = test_cmd_ops,
	};

	ret = cmd_register(&test_cmd);

	if (ret != 0) {
		printf("%s: couldn't register cmd %s", __func__, test_cmd.name);
	}
	return ret;
}

INIT_FUNCTION(test_crypto_register);
