// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021, Open Mobile Platform LLC
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <tee_plugin_method.h>
#include <test_supp_plugin.h>
#include <unistd.h>

static TEEC_Result pass_values(unsigned int sub_cmd, void *data,
			      size_t data_len, size_t *out_len)
{
	struct add_op {
		uint32_t a;
		uint32_t b;
	} *values = NULL;

	values = (struct add_op *)data;
	*out_len = sizeof(uint32_t);

	if (sub_cmd == '+')
		values->a = values->a + values->b;
	else
		values->a = values->a - values->b;

	return TEEC_SUCCESS;
}

static TEEC_Result write_test_arr(unsigned int sub_cmd, void *data,
				  size_t data_len, size_t *out_len)
{
	(void)sub_cmd;
	(void)out_len;

	int fd = -1;
	int n = -1;
	char template[] = "/tmp/testplugin-XXXXXX";
	char fname[PATH_MAX] = { 0 };

	strcpy(fname, template);

	fd = mkstemp(fname);
	if (fd < 0)
		return TEEC_ERROR_GENERIC;

	n = write(fd, (const void *)data, data_len);
	if (n < 0) {
		close(fd);
		return TEEC_ERROR_GENERIC;
	}

	*out_len = sizeof(template);
	memcpy(data, (const void *)fname, *out_len);
	close(fd);

	return TEEC_SUCCESS;
}

static TEEC_Result get_test_arr(unsigned int sub_cmd, void *data,
				size_t data_len, size_t *out_len)
{
	(void)sub_cmd;
	char test_arr[] = "Array from plugin";
	size_t test_size = sizeof(test_arr);

	if (data_len < test_size)
		return TEEC_ERROR_GENERIC;

	memcpy(data, test_arr, test_size);
	*out_len = test_size;

	return TEEC_SUCCESS;
}

static TEEC_Result test_plugin_invoke(unsigned int cmd, unsigned int sub_cmd,
				      void *data, size_t data_len,
				      size_t *out_len)
{
	switch (cmd) {
	case TEST_PLUGIN_CMD_PING:
		return TEEC_SUCCESS;
	case TEST_PLUGIN_CMD_PASS_VALUES:
		return pass_values(sub_cmd, data, data_len, out_len);
	case TEST_PLUGIN_CMD_WRITE_ARR:
		return write_test_arr(sub_cmd, data, data_len, out_len);
	case TEST_PLUGIN_CMD_GET_ARR:
		return get_test_arr(sub_cmd, data, data_len, out_len);
	default:
		break;
	}

	return TEEC_ERROR_NOT_SUPPORTED;
}

struct plugin_method plugin_method = {
	.name = "test",
	.uuid = TEST_PLUGIN_UUID,
	.invoke = test_plugin_invoke,
};
