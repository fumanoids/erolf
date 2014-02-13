/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBOPENCM3_RTC_H
#define LIBOPENCM3_RTC_H

#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/pwr.h>

/* --- RTC registers ------------------------------------------------------- */

/* RTC control register high (RTC_CRH) */
#define RTC_TR              MMIO32(RTC_BASE + 0x00)
#define RTC_DR              MMIO32(RTC_BASE + 0x04)
#define RTC_CR              MMIO32(RTC_BASE + 0x08)
#define RTC_ISR             MMIO32(RTC_BASE + 0x0c)
#define RTC_PRER             MMIO32(RTC_BASE + 0x10)
#define RTC_WUT             MMIO32(RTC_BASE + 0x14)
#define RTC_CALIBR             MMIO32(RTC_BASE + 0x18)
#define RTC_ALRMAR             MMIO32(RTC_BASE + 0x1c)
#define RTC_ALRMBR             MMIO32(RTC_BASE + 0x20)
#define RTC_WPR             MMIO32(RTC_BASE + 0x24)
#define RTC_SSR             MMIO32(RTC_BASE + 0x28)
#define RTC_SHIFTR             MMIO32(RTC_BASE + 0x2c)
#define RTC_TSTR             MMIO32(RTC_BASE + 0x30)
#define RTC_TSSSR             MMIO32(RTC_BASE + 0x38)
#define RTC_CALR             MMIO32(RTC_BASE + 0x3c)
#define RTC_TAFCR             MMIO32(RTC_BASE + 0x40)
#define RTC_ALRMASSR             MMIO32(RTC_BASE + 0x44)
#define RTC_ALRMBSSR             MMIO32(RTC_BASE + 0x48)

#define RTC_BK0R             MMIO32(RTC_BASE + 0x50)
#define RTC_BK1R             MMIO32(RTC_BASE + 0x54)
#define RTC_BK2R             MMIO32(RTC_BASE + 0x58)
#define RTC_BK3R             MMIO32(RTC_BASE + 0x5c)
#define RTC_BK4R             MMIO32(RTC_BASE + 0x60)
#define RTC_BK5R             MMIO32(RTC_BASE + 0x64)
#define RTC_BK6R             MMIO32(RTC_BASE + 0x68)
#define RTC_BK7R             MMIO32(RTC_BASE + 0x6c)
#define RTC_BK8R             MMIO32(RTC_BASE + 0x70)
#define RTC_BK9R             MMIO32(RTC_BASE + 0x74)
#define RTC_BK10R             MMIO32(RTC_BASE + 0x78)
#define RTC_BK11R             MMIO32(RTC_BASE + 0x7c)
#define RTC_BK12R             MMIO32(RTC_BASE + 0x80)
#define RTC_BK13R             MMIO32(RTC_BASE + 0x84)
#define RTC_BK14R             MMIO32(RTC_BASE + 0x88)
#define RTC_BK15R             MMIO32(RTC_BASE + 0x8c)
#define RTC_BK16R             MMIO32(RTC_BASE + 0x90)
#define RTC_BK17R             MMIO32(RTC_BASE + 0x94)
#define RTC_BK18R             MMIO32(RTC_BASE + 0x98)
#define RTC_BK19R             MMIO32(RTC_BASE + 0x9c)

#define RTC_TR_PM_SHIFT 22
#define RTC_TR_HT_SHIFT 20
#define RTC_TR_HU_SHIFT 16
#define RTC_TR_MNT_SHIFT 12
#define RTC_TR_MNU_SHIFT 8
#define RTC_TR_ST_SHIFT 4
#define RTC_TR_SU_SHIFT 0


#define RTC_DR_YT_SHIFT 20
#define RTC_DR_YU_SHIFT 16
#define RTC_DR_WDU_SHIFT 13
#define RTC_DR_MT_SHIFT 12
#define RTC_DR_MU_SHIFT 8
#define RTC_DR_DT_SHIFT 4
#define RTC_DR_DU_SHIFT 0


#define RTC_CR_COE_SHIFT 23
#define RTC_CR_OSEL_SHIFT 21
#define RTC_CR_POL_SHIFT 20
#define RTC_CR_COSEL_SHIFT 19
#define RTC_CR_BKP_SHIFT 18
#define RTC_CR_SUB1H_SHIFT 17
#define RTC_CR_ADD1H_SHIFT 16
#define RTC_CR_TSIE_SHIFT 15
#define RTC_CR_WUTIE_SHIFT 14
#define RTC_CR_ALRBIE_SHIFT 13
#define RTC_CR_ALRAIE_SHIFT 12
#define RTC_CR_TSE_SHIFT 11
#define RTC_CR_WUTE_SHIFT 10
#define RTC_CR_ALRBE_SHIFT 9
#define RTC_CR_ALRAE_SHIFT 8
#define RTC_CR_DCE_SHIFT 7
#define RTC_CR_FMT_SHIFT 6
#define RTC_CR_BYPSHAD_SHIFT 5
#define RTC_CR_REFCKON_SHIFT 4
#define RTC_CR_TSEDGE_SHIFT 3
#define RTC_CR_WCKSEL_SHIFT 0


#define RTC_ISR_TAMP2F_SHIFT 14
#define RTC_ISR_TAMP1F_SHIFT 13
#define RTC_ISR_TSOVF_SHIFT 12
#define RTC_ISR_TSF_SHIFT 11
#define RTC_ISR_WUTF_SHIFT 10
#define RTC_ISR_ALRBF_SHIFT 9
#define RTC_ISR_ALRAF_SHIFT 8
#define RTC_ISR_INIT_SHIFT 7
#define RTC_ISR_INITF_SHIFT 6
#define RTC_ISR_RSF_SHIFT 5
#define RTC_ISR_INITS_SHIFT 4
#define RTC_ISR_SHPF_SHIFT 3
#define RTC_ISR_WUTWF_SHIFT 2
#define RTC_ISR_ALRBWF_SHIFT 1
#define RTC_ISR_ALRAWF_SHIFT 0


#define RTC_PRER_PREDIV_A_SHIFT 16
#define RTC_PRER_PREDIV_S_SHIFT 0

#define RTC_WUTR_WUT_SHIFT 0

#define RTC_CALIBR_DCS_SHIFT 7
#define RTC_CALIBR_DC_SHIFT 0


#define RTC_ALRMAR_MSK4_SHIFT 31
#define RTC_ALRMAR_WDSEL_SHIFT 30
#define RTC_ALRMAR_DT_SHIFT 28
#define RTC_ALRMAR_DU_SHIFT 24
#define RTC_ALRMAR_MKS3_SHIFT 23
#define RTC_ALRMAR_PM_SHIFT 22
#define RTC_ALRMAR_HT_SHIFT 20
#define RTC_ALRMAR_HU_SHIFT 16
#define RTC_ALRMAR_MKS2_SHIFT 15
#define RTC_ALRMAR_MNT_SHIFT 12
#define RTC_ALRMAR_MNU_SHIFT 8
#define RTC_ALRMAR_MSK1_SHIFT 7
#define RTC_ALRMAR_ST_SHIFT 5
#define RTC_ALRMAR_SU_SHIFT 0

#define RTC_ALRMBR_MSK4_SHIFT 31
#define RTC_ALRMBR_WDSEL_SHIFT 30
#define RTC_ALRMBR_DT_SHIFT 28
#define RTC_ALRMBR_DU_SHIFT 24
#define RTC_ALRMBR_MKS3_SHIFT 23
#define RTC_ALRMBR_PM_SHIFT 22
#define RTC_ALRMBR_HT_SHIFT 20
#define RTC_ALRMBR_HU_SHIFT 16
#define RTC_ALRMBR_MKS2_SHIFT 15
#define RTC_ALRMBR_MNT_SHIFT 12
#define RTC_ALRMBR_MNU_SHIFT 8
#define RTC_ALRMBR_MSK1_SHIFT 7
#define RTC_ALRMBR_ST_SHIFT 5
#define RTC_ALRMBR_SU_SHIFT 0

#define RTC_WPR_KEY_SHIFT 0
#define RTC_WPR_KEY_1 (0xCA)
#define RTC_WPR_KEY_2 (0x53)


#define RTC_SSR_SS_SHIFT 0

#define RTC_SHIFTR_ADD1S_SHIFT 31
#define RTC_SHIFTR_SUBFS_SHIFT 0


#define RTC_TSTR_PM_SHIFT 22
#define RTC_TSTR_HT_SHIFT 20
#define RTC_TSTR_HU_SHIFT 16
#define RTC_TSTR_MNT_SHIFT 12
#define RTC_TSTR_MNU_SHIFT 8
#define RTC_TSTR_ST_SHIFT 5
#define RTC_TSTR_SU_SHIFT 0


#define RTC_TSSSR_SS_SHIFT 0


#define RTC_CALR_CALP_SHIFT 15
#define RTC_CALR_CALW8_SHIFT 14
#define RTC_CALR_CALW16_SHIFT 13
#define RTC_CALR_CALM_SHIFT 0

#define RTC_TAFCR_ALARMOUTTYPE_SHIFT 18
#define RTC_TAFCR_TSINSEL_SHIFT 17
#define RTC_TAFCR_TAMP1INSEL_SHIFT 16
#define RTC_TAFCR_TAMPPUDIS_SHIFT 15
#define RTC_TAFCR_TAMPPRCH_SHIFT 13
#define RTC_TAFCR_TAMPFLT_SHIFT 11
#define RTC_TAFCR_TAMPFREQ_SHIFT 8
#define RTC_TAFCR_TAMPTS_SHIFT 7
#define RTC_TAFCR_TAMP2TRG_SHIFT 4
#define RTC_TAFCR_TAMP2E_SHIFT 3
#define RTC_TAFCR_TAMPIE_SHIFT 2
#define RTC_TAFCR_TAMP1ETRG_SHIFT 1
#define RTC_TAFCR_TAMP1E_SHIFT 0


#define RTC_ALRMASSR_MASKSS_SHIFT 24
#define RTC_ALRMASSR_SS_SHIFT 0


#define RTC_ALRMBSSR_MASKSS_SHIFT 24
#define RTC_ALRMBSSR_SS_SHIFT 0



#endif
