/*
 * Copyright (c) 2020 - 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <nrf_802154_config.h>
#include <platform/nrf_802154_clock.h>

#include <stddef.h>

#include <compiler_abstraction.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/clock_control/nrf_clock_control.h>
#include <nrf_sys_event.h>

static bool hfclk_is_running;

void nrf_802154_clock_init(void)
{
#ifdef NRF54L_SERIES
	uint32_t clock_latency_us = z_nrf_clock_bt_ctlr_hf_get_startup_time_us();

	nrf_802154_clock_hfclk_latency_set(clock_latency_us);
#endif
}

void nrf_802154_clock_deinit(void)
{
	/* Intentionally empty. */
}

bool nrf_802154_clock_hfclk_is_running(void)
{
	return hfclk_is_running;
}


static struct onoff_client hfclk_cli;

static void hfclk_on_callback(struct onoff_manager *mgr,
			      struct onoff_client  *cli,
			      uint32_t state,
			      int res)
{
	hfclk_is_running = true;
	nrf_802154_clock_hfclk_ready();
}

#if defined(CONFIG_CLOCK_CONTROL_NRF)
void nrf_802154_clock_hfclk_start(void)
{
	struct onoff_manager *mgr =
		z_nrf_clock_control_get_onoff(CLOCK_CONTROL_NRF_SUBSYS_HF);

	__ASSERT_NO_MSG(mgr != NULL);

	sys_notify_init_callback(&hfclk_cli.notify, hfclk_on_callback);

	/*
	 * todo: replace constlat request with PM policy API when
	 * controlling the event latency becomes possible.
	 */
	if (IS_ENABLED(CONFIG_NRF_802154_CONSTLAT_CONTROL)) {
		nrf_sys_event_request_global_constlat();
	}

	int ret = onoff_request(mgr, &hfclk_cli);
	__ASSERT_NO_MSG(ret >= 0);
	(void)ret;
}

void nrf_802154_clock_hfclk_stop(void)
{
	struct onoff_manager *mgr =
		z_nrf_clock_control_get_onoff(CLOCK_CONTROL_NRF_SUBSYS_HF);

	__ASSERT_NO_MSG(mgr != NULL);

	int ret = onoff_cancel_or_release(mgr, &hfclk_cli);
	__ASSERT_NO_MSG(ret >= 0);
	(void)ret;

	if (IS_ENABLED(CONFIG_NRF_802154_CONSTLAT_CONTROL)) {
		nrf_sys_event_release_global_constlat();
	}

	hfclk_is_running = false;
}

#elif DT_NODE_HAS_STATUS(DT_NODELABEL(hfxo), okay) && \
	DT_NODE_HAS_COMPAT(DT_NODELABEL(hfxo), nordic_nrf54h_hfxo)

void nrf_802154_clock_hfclk_start(void)
{
	sys_notify_init_callback(&hfclk_cli.notify, hfclk_on_callback);
	int ret = nrf_clock_control_request(DEVICE_DT_GET(DT_NODELABEL(hfxo)), NULL, &hfclk_cli);

	__ASSERT_NO_MSG(ret >= 0);
	(void)ret;
}

void nrf_802154_clock_hfclk_stop(void)
{
	int ret = nrf_clock_control_cancel_or_release(DEVICE_DT_GET(DT_NODELABEL(hfxo)),
						      NULL, &hfclk_cli);

	__ASSERT_NO_MSG(ret >= 0);
	(void)ret;
}

#endif
