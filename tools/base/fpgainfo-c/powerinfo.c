// Copyright(c) 2018, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <getopt.h>
#include "fpgainfo.h"
#include "sysinfo.h"
#include "powerinfo.h"
#include <opae/fpga.h>
#include <uuid/uuid.h>

#include "bmcinfo.h"
#include "safe_string/safe_string.h"

#define MODEL_SIZE 64

#if 0
struct power_info {
	fpga_guid guid;
	uint64_t object_id;
	uint8_t bus;
	uint8_t device;
	uint8_t function;
	uint8_t socket_id;
	uint32_t device_id;
	char model[MODEL_SIZE];
	uint32_t num_slots;
	uint64_t bbs_id;
	fpga_version bbs_version;
	uint64_t capabilities;
};

static fpga_result get_power_info(fpga_token tok, struct power_info *finfo)
{
	fpga_result res = FPGA_OK;
	fpga_properties props;
	res = fpgaGetProperties(tok, &props);
	ON_FPGAINFO_ERR_GOTO(res, out, "reading properties from token");

	fpgaPropertiesGetObjectID(props, &finfo->object_id);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading object_id from properties");

	fpgaPropertiesGetGUID(props, &finfo->guid);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy, "reading guid from properties");

	fpgaPropertiesGetBus(props, &finfo->bus);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy, "reading bus from properties");

	fpgaPropertiesGetDevice(props, &finfo->device);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading device from properties");

	fpgaPropertiesGetFunction(props, &finfo->function);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading function from properties");

	fpgaPropertiesGetSocketID(props, &finfo->socket_id);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading socket_id from properties");

	// TODO: Implement once device_id, model, and capabilities accessors are
	// implemented

	// fpgaPropertiesGetDeviceId(props, &finfo->device_id);
	// ON_FPGAINFO_ERR_GOTO(res, out_destroy, "reading device_id from
	// properties");

	// fpgaPropertiesGetModel(props, &finfo->model);
	// ON_FPGAINFO_ERR_GOTO(res, out_destroy, "reading model from
	// properties");

	// fpgaPropertiesGetCapabilities(props, &finfo->capabilities);
	// ON_FPGAINFO_ERR_GOTO(res, out_destroy, "reading capabilities from
	// properties");

	fpgaPropertiesGetNumSlots(props, &finfo->num_slots);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading num_slots from properties");

	fpgaPropertiesGetBBSID(props, &finfo->bbs_id);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading bbs_id from properties");

	fpgaPropertiesGetBBSVersion(props, &finfo->bbs_version);
	ON_FPGAINFO_ERR_GOTO(res, out_destroy,
			     "reading bbs_version from properties");

out_destroy:
	fpgaDestroyProperties(&props);
out:
	return res;
}
#endif

/*
 * Print help
 */
void power_help(void)
{
	printf("\nPrint power metrics\n"
	       "        fpgainfo power [-h]\n"
	       "                -h,--help           Print this help\n"
	       "\n");
}

static void print_power_info(fpga_properties props)
{
	fpga_result res = FPGA_OK;

	fpgainfo_print_common("//****** POWER ******//", props);

	res = bmc_print_values(get_sysfs_path(props, FPGA_DEVICE, NULL),
			       BMC_POWER);
	fpgainfo_print_err("Cannot read BMC telemetry", res);
}

fpga_result power_filter(fpga_properties *filter, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	fpga_result res = FPGA_OK;
	res = fpgaPropertiesSetObjectType(*filter, FPGA_DEVICE);
	fpgainfo_print_err("setting type to FPGA_DEVICE", res);
	return res;
}

fpga_result power_command(fpga_token *tokens, int num_tokens, int argc,
			  char *argv[])
{
	(void)tokens;
	(void)num_tokens;
	(void)argc;
	(void)argv;

	fpga_result res = FPGA_OK;
	fpga_properties props;

	optind = 0;
	struct option longopts[] = {{"help", no_argument, NULL, 'h'},
				    {0, 0, 0, 0}};

	int getopt_ret;
	int option_index;

	while (-1
	       != (getopt_ret = getopt_long(argc, argv, ":h",
					    longopts, &option_index))) {
		const char *tmp_optarg = optarg;

		if ((optarg) && ('=' == *tmp_optarg)) {
			++tmp_optarg;
		}

		switch (getopt_ret) {
		case 'h': /* help */
			power_help();
			return res;

		case ':': /* missing option argument */
			fprintf(stderr, "Missing option argument\n");
			power_help();
			return FPGA_INVALID_PARAM;

		case '?':
		default: /* invalid option */
			fprintf(stderr, "Invalid cmdline options\n");
			power_help();
			return FPGA_INVALID_PARAM;
		}
	}

	int i = 0;
	for (i = 0; i < num_tokens; ++i) {
		res = fpgaGetProperties(tokens[i], &props);
		ON_FPGAINFO_ERR_GOTO(res, out_destroy,
				     "reading properties from token");

		print_power_info(props);
		fpgaDestroyProperties(&props);
	}

	return res;

out_destroy:
	fpgaDestroyProperties(&props);
	return res;
}