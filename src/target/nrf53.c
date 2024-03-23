#include "general.h"
#include "target.h"
#include "target_internal.h"
#include "cortexm.h"
#include "adiv5.h"
#include "gdb_packet.h"

/* Non-Volatile Memory Controller (NVMC) Registers */
#define NRF53_APP_NVMC           0x50039000U
#define NRF53_NET_NVMC           0x41080000U

#define NRF53_NVMC_READY     (NRF53_NVMC + 0x400U)
#define NRF53_NVMC_READYNEXT (NRF53_NVMC + 0x408U)
#define NRF53_NVMC_CONFIG    (NRF53_NVMC + 0x504U)
#define NRF53_NVMC_ERASEALL  (NRF53_NVMC + 0x50cU)

#define NVMC_TIMEOUT_MS 300U

#define NRF53_NVMC_CONFIG_REN  0x0U // Read only access
#define NRF53_NVMC_CONFIG_WEN  0x1U // Write enable
#define NRF53_NVMC_CONFIG_EEN  0x2U // Erase enable
#define NRF53_NVMC_CONFIG_PEEN 0x3U // Partial erase enable

#define NRF53_PARTNO 0x70U

#define NRF53_CTRL_AP_RESET          ADIV5_AP_REG(0x000)
#define NRF53_CTRL_AP_ERASEALL       ADIV5_AP_REG(0x004)
#define NRF53_CTRL_AP_ERASEALLSTATUS ADIV5_AP_REG(0x008)
#define NRF53_CTRL_IDR_EXPECTED      0x12880000

#define NRF53_UICR_ERASED_VAL                  0xFFFFFFFFU
#define NRF53_UICR_APPROTECT_UNPROTECT_VAL     0x50FA50FAU
#define NRF53_APP_UICR_APPROTECT               0x00FF8000U
#define NRF53_APP_UICR_SECUREAPPROTECT         0x00FF801CU
#define NRF53_NET_UICR_APPROTECT               0x01FF8000U

#define NRF53_APP_AHB_AP 0
#define NRF53_NET_AHB_AP 1
#define NRF53_APP_CTRL_AP 2
#define NRF53_NET_CTRL_AP 3

unsigned char nrf53app_empty_app[] = {
  0x00, 0x10, 0x00, 0x20, 0x09, 0x00, 0x00, 0x00, 0x0a, 0x4b, 0x0b, 0x4a,
  0x1b, 0x68, 0xc2, 0xf8, 0x44, 0x35, 0x08, 0x4b, 0x08, 0x4a, 0xdb, 0x69,
  0xc2, 0xf8, 0x4c, 0x35, 0x07, 0x4b, 0x4f, 0xf0, 0x10, 0x02, 0xc3, 0xf8,
  0x40, 0x24, 0x4f, 0xf0, 0x50, 0x23, 0x4f, 0xf0, 0x00, 0x02, 0xc3, 0xf8,
  0x14, 0x26, 0xfe, 0xe7, 0x00, 0x80, 0xff, 0x00, 0x00, 0x60, 0x00, 0x50,
  0x00, 0x30, 0x00, 0x50
};
unsigned int nrf53app_empty_app_len = 64;

unsigned char nrf53net_empty_app[] = {
  0x00, 0x10, 0x00, 0x21, 0x09, 0x00, 0x00, 0x01, 0x02, 0x4b, 0x03, 0x4a,
  0x1b, 0x68, 0xc2, 0xf8, 0x44, 0x35, 0xfe, 0xe7, 0x00, 0x80, 0xff, 0x01,
  0x00, 0x60, 0x00, 0x41
};
unsigned int nrf53net_empty_app_len = 28;

bool nrf91_probe(target_s *target)
{
	adiv5_access_port_s *ap = cortex_ap(target);

	if (ap->dp->version < 2U || ap->dp->target_partno != NRF53_PARTNO)
		return false;
        target->driver = "Nordic nRF53";
	target->target_options |= TOPT_INHIBIT_NRST;
	target_add_ram(target, 0x20000000, 512U * 1024U);
	target_add_ram(target, 0x21000000, 64U * 1024U);
	nrf53_add_flash(target, 0x00000000, 4096U * 256U, 4096U);
	nrf53_add_flash(target, 0x01000000, 2048U * 128U, 2048U);

	target->mass_erase = nrf53_mass_erase;
	target->exit_flash_mode = nrf53_exit_flash_mode;

	return true;

}
