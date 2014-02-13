/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Thomas Otto <tommi@viadmin.org>
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

#ifndef LIBOPENCM3_DMA_H
#define LIBOPENCM3_DMA_H

#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/common.h>

/* --- Convenience macros -------------------------------------------------- */

/* DMA register base adresses (for convenience) */
#define DMA1				DMA1_BASE
#define DMA2				DMA2_BASE

#define DMA_STREAM_0        (0)
#define DMA_STREAM_1        (1)
#define DMA_STREAM_2        (2)
#define DMA_STREAM_3        (3)
#define DMA_STREAM_4        (4)
#define DMA_STREAM_5        (5)
#define DMA_STREAM_6        (6)
#define DMA_STREAM_7        (7)

/* --- DMA registers ------------------------------------------------------- */

/* DMA interrupt status register (DMAx_ISR) */
#define DMA_LISR(dma_base)		MMIO32(dma_base + 0x00)
#define DMA_HISR(dma_base)		MMIO32(dma_base + 0x04)

#define DMA1_LISR			DMA_LISR(DMA1)
#define DMA2_LISR			DMA_LISR(DMA2)

#define DMA1_HISR			DMA_HISR(DMA1)
#define DMA2_HISR			DMA_HISR(DMA2)

/* DMA interrupt flag clear register (DMAx_IFCR) */
#define DMA_LIFCR(dma_base)		MMIO32(dma_base + 0x08)
#define DMA_HIFCR(dma_base)		MMIO32(dma_base + 0x0c)

#define DMA1_LIFCR			DMA_LIFCR(DMA1)
#define DMA2_LIFCR			DMA_LIFCR(DMA2)

#define DMA1_HIFCR			DMA_HIFCR(DMA1)
#define DMA2_HIFCR			DMA_HIFCR(DMA2)


/* DMA configuration register (DMAx_SyCR1) */
#define DMA_SCCR(dma_base, dma_stream)		MMIO32(dma_base + 0x10 + 0x18 * (dma_stream))

/* DMA number of data register (DMAx_SyNDTR) */
#define DMA_SNDTR(dma_base, dma_stream)		MMIO32(dma_base + 0x14 + 0x18 * (dma_stream))

/* DMA peripheral address register (DMAx_SyPAR) */
#define DMA_SPAR(dma_base, dma_stream)		MMIO32(dma_base + 0x18 + 0x18 * (dma_stream))

/* DMA memory address register (DMAx_SyM1AR) */
#define DMA_SM0AR(dma_base, dma_stream)		MMIO32(dma_base + 0x1c + 0x18 * (dma_stream))

/* DMA memory address register (DMAx_SyM0AR) */
#define DMA_SM1AR(dma_base, dma_stream)		MMIO32(dma_base + 0x20 + 0x18 * (dma_stream))

/* DMA double buffer address register (DMAx_SyM1AR) */
#define DMA_SFCR(dma_base, dma_stream)		MMIO32(dma_base + 0x24 + 0x18 * (dma_stream))


/* --- DMA_LISR values ------------------------------------------------------ */

#define DMA_LISR_TCIF3          (1 << 27)
#define DMA_LISR_TCIF3_SHIFT    27

#define DMA_LISR_HTIF3          (1 << 26)
#define DMA_LISR_HTIF3_SHIFT	26

#define DMA_LISR_TEIF3          (1 << 25)
#define DMA_LISR_TEIF3_SHIFT    25

#define DMA_LISR_DMEIF3         (1 << 24)
#define DMA_LISR_DMEIF3_SHIFT   24

#define DMA_LISR_FEIF3          (1 << 22)
#define DMA_LISR_FEIF3_SHIFT    22

#define DMA_LISR_TCIF2          (1 << 21)
#define DMA_LISR_TCIF2_SHIFT    21

#define DMA_LISR_HTIF2          (1 << 20)
#define DMA_LISR_HTIF2_SHIFT    20

#define DMA_LISR_TEIF2          (1 << 19)
#define DMA_LISR_TEIF2_SHIFT    19

#define DMA_LISR_DMEIF2         (1 << 18)
#define DMA_LISR_DMEIF2_SHIFT	18

#define DMA_LISR_FEIF2          (1 << 16)
#define DMA_LISR_FEIF2_SHIFT    16

#define DMA_LISR_TCIF1          (1 << 11)
#define DMA_LISR_TCIF1_SHIFT    11

#define DMA_LISR_HTIF1          (1 << 10)
#define DMA_LISR_HTIF1_SHIFT    10

#define DMA_LISR_TEIF1          (1 << 9)
#define DMA_LISR_TEIF1_SHIFT    9

#define DMA_LISR_DMEIF1         (1 << 8)
#define DMA_LISR_DMEIF1_SHIFT   8

#define DMA_LISR_FEIF1          (1 << 6)
#define DMA_LISR_FEIF1_SHIFT    6

#define DMA_LISR_TCIF0          (1 << 5)
#define DMA_LISR_TCIF0_SHIFT    5

#define DMA_LISR_HTIF0          (1 << 4)
#define DMA_LISR_HTIF0_SHIFT    4

#define DMA_LISR_TEIF0          (1 << 3)
#define DMA_LISR_TEIF0_SHIFT	3

#define DMA_LISR_DMEIF0         (1 << 2)
#define DMA_LISR_DMEIF0_SHIFT   2

#define DMA_LISR_FEIF0          (1 << 0)
#define DMA_LISR_FEIF0_SHIFT    0


/* --- DMA_HISR values ------------------------------------------------------ */

#define DMA_HISR_TCIF7          (1 << 27)
#define DMA_HISR_TCIF7_SHIFT	27

#define DMA_HISR_HTIF7          (1 << 26)
#define DMA_HISR_HTIF7_SHIFT    26

#define DMA_HISR_TEIF7          (1 << 25)
#define DMA_HISR_TEIF7_SHIFT 	25

#define DMA_HISR_DMEIF7         (1 << 24)
#define DMA_HISR_DMEIF7_SHIFT   24

#define DMA_HISR_FEIF7          (1 << 22)
#define DMA_HISR_FEIF7_SHIFT    22

#define DMA_HISR_TCIF6          (1 << 21)
#define DMA_HISR_TCIF6_SHIFT    21

#define DMA_HISR_HTIF6          (1 << 20)
#define DMA_HISR_HTIF6_SHIFT    20

#define DMA_HISR_TEIF6          (1 << 19)
#define DMA_HISR_TEIF6_SHIFT    19

#define DMA_HISR_DMEIF6         (1 << 18)
#define DMA_HISR_DMEIF6_SHIFT   18

#define DMA_HISR_FEIF6          (1 << 16)
#define DMA_HISR_FEIF6_SHIFT    16

#define DMA_HISR_TCIF5          (1 << 11)
#define DMA_HISR_TCIF5_SHIFT    11

#define DMA_HISR_HTIF5          (1 << 10)
#define DMA_HISR_HTIF5_SHIFT    10

#define DMA_HISR_TEIF5          (1 << 9)
#define DMA_HISR_TEIF5_SHIFT    9

#define DMA_HISR_DMEIF5         (1 << 8)
#define DMA_HISR_DMEIF5_SHIFT   8

#define DMA_HISR_FEIF5          (1 << 6)
#define DMA_HISR_FEIF5_SHIFT    6

#define DMA_HISR_TCIF4          (1 << 5)
#define DMA_HISR_TCIF4_SHIFT    5

#define DMA_HISR_HTIF4          (1 << 4)
#define DMA_HISR_HTIF4_SHIFT    4

#define DMA_HISR_TEIF4          (1 << 3)
#define DMA_HISR_TEIF4_SHIFT    3

#define DMA_HISR_DMEIF4         (1 << 2)
#define DMA_HISR_DMEIF4_SHIFT   2

#define DMA_HISR_FEIF4          (1 << 0)
#define DMA_HISR_FEIF4_SHIFT    0

/* --- DMA_LIFCR values ------------------------------------------------------ */

#define DMA_LIFCR_CTCIF3        (1 << 27)
#define DMA_LIFCR_CTCIF3_SHIFT  27

#define DMA_LIFCR_CHTIF3        (1 << 26)
#define DMA_LIFCR_CHTIF3_SHIFT  26

#define DMA_LIFCR_CTEIF3        (1 << 25)
#define DMA_LIFCR_CTEIF3_SHIFT  25

#define DMA_LIFCR_CDMEIF3       (1 << 24)
#define DMA_LIFCR_CDMEIF3_SHIFT 24

#define DMA_LIFCR_CFEIF3        (1 << 22)
#define DMA_LIFCR_CFEIF3_SHIFT  22

#define DMA_LIFCR_CTCIF2        (1 << 21)
#define DMA_LIFCR_CTCIF2_SHIFT  21

#define DMA_LIFCR_CHTIF2        (1 << 20)
#define DMA_LIFCR_CHTIF2_SHIFT  20

#define DMA_LIFCR_CTEIF2        (1 << 19)
#define DMA_LIFCR_CTEIF2_SHIFT  19

#define DMA_LIFCR_CDMEIF2       (1 << 18)
#define DMA_LIFCR_CDMEIF2_SHIFT	18

#define DMA_LIFCR_CFEIF2        (1 << 17)
#define DMA_LIFCR_CFEIF2_SHIFT  17

#define DMA_LIFCR_CTCIF1        (1 << 11)
#define DMA_LIFCR_CTCIF1_SHIFT  11

#define DMA_LIFCR_CHTIF1        (1 << 10)
#define DMA_LIFCR_CHTIF1_SHIFT  10

#define DMA_LIFCR_CTEIF1        (1 << 9)
#define DMA_LIFCR_CTEIF1_SHIFT  9

#define DMA_LIFCR_CDMEIF1       (1 << 8)
#define DMA_LIFCR_CDMEIF1_SHIFT 8

#define DMA_LIFCR_CFEIF1        (1 << 6)
#define DMA_LIFCR_CFEIF1_SHIFT  6

#define DMA_LIFCR_CTCIF0        (1 << 5)
#define DMA_LIFCR_CTCIF0_SHIFT	5

#define DMA_LIFCR_CHTIF0        (1 << 4)
#define DMA_LIFCR_CHTIF0_SHIFT	4

#define DMA_LIFCR_CTEIF0        (1 << 3)
#define DMA_LIFCR_CTEIF0_SHIFT	3

#define DMA_LIFCR_CDMEIF0       (1 << 2)
#define DMA_LIFCR_CDMEIF0_SHIFT	2

#define DMA_LIFCR_CFEIF0        (1 << 0)
#define DMA_LIFCR_CFEIF0_SHIFT	0

/* --- DMA_HIFCR values ------------------------------------------------------ */

#define DMA_HIFCR_CTCIF7        (1 << 27)
#define DMA_HIFCR_CTCIF7_SHIFT	27

#define DMA_HIFCR_CHTIF7        (1 << 26)
#define DMA_HIFCR_CHTIF7_SHIFT	26

#define DMA_HIFCR_CTEIF7        (1 << 25)
#define DMA_HIFCR_CTEIF7_SHIFT	25

#define DMA_HIFCR_CDMEIF7       (1 << 24)
#define DMA_HIFCR_CDMEIF7_SHIFT	24

#define DMA_HIFCR_CFEIF7        (1 << 22)
#define DMA_HIFCR_CFEIF7_SHIFT	22

#define DMA_HIFCR_CTCIF6        (1 << 21)
#define DMA_HIFCR_CTCIF6_SHIFT	21

#define DMA_HIFCR_CHTIF6        (1 << 20)
#define DMA_HIFCR_CHTIF6_SHIFT	20

#define DMA_HIFCR_CTEIF6        (1 << 19)
#define DMA_HIFCR_CTEIF6_SHIFT	19

#define DMA_HIFCR_CDMEIF6       (1 << 18)
#define DMA_HIFCR_CDMEIF6_SHIFT	18

#define DMA_HIFCR_CFEIF6        (1 << 17)
#define DMA_HIFCR_CFEIF6_SHIFT	17

#define DMA_HIFCR_CTCIF5        (1 << 11)
#define DMA_HIFCR_CTCIF5_SHIFT	11

#define DMA_HIFCR_CHTIF5        (1 << 10)
#define DMA_HIFCR_CHTIF5_SHIFT	10

#define DMA_HIFCR_CTEIF5      	(1 << 9)
#define DMA_HIFCR_CTEIF5_SHIFT	9

#define DMA_HIFCR_CDMEI5F5       (1 << 8)
#define DMA_HIFCR_CDMEI5F5_SHIFT 8

#define DMA_HIFCR_CFEIF5        (1 << 6)
#define DMA_HIFCR_CFEIF5_SHIFT	6

#define DMA_HIFCR_CTCIF4        (1 << 5)
#define DMA_HIFCR_CTCIF4_SHIFT	5

#define DMA_HIFCR_CHTIF4        (1 << 4)
#define DMA_HIFCR_CHTIF4_SHIFT	4

#define DMA_HIFCR_CTEIF4       	(1 << 3)
#define DMA_HIFCR_CTEIF4_SHIFT	3

#define DMA_HIFCR_CDMEIF4       (1 << 2)
#define DMA_HIFCR_CDMEIF4_SHIFT	2

#define DMA_HIFCR_CFEIF4        (1 << 0)
#define DMA_HIFCR_CFEIF4_SHIFT	0


/* --- DMA_CR values ----------------------------------------------------- */
#define DMA_CR_CHSEL  		   			(1 << 25)
#define DMA_CR_CHSEL_LSB   				25

#define DMA_CR_MBURST     				(1 << 23)
#define DMA_CR_MBURST_SHIFT				23
#define DMA_CR_MBURST_SINGLETRANSFER	(0x0 << 23)
#define DMA_CR_MBURST_INCR4				(0x1 << 23)
#define DMA_CR_MBURST_INCR8				(0x2 << 23)
#define DMA_CR_MBURST_INCR16			(0x3 << 23)
#define DMA_CR_MBURST_MASK				(0x3 << 23)

#define DMA_CR_PBURST     				(1 << 21)
#define DMA_CR_PBURST_SHIFT				21
#define DMA_CR_PBURST_SINGLETRANSFER    (0x0 << 21)
#define DMA_CR_PBURST_INCR4		    	(0x1 << 21)
#define DMA_CR_PBURST_INCR8				(0x2 << 21)
#define DMA_CR_PBURST_INCR16		    (0x3 << 21)
#define DMA_CR_PBURST_MASK			    (0x3 << 21)

#define DMA_CR_ACK	   					 (1 << 20)
#define DMA_CR_ACK_SHIFT	   			 20

#define DMA_CR_CT         				(1 << 19)
#define DMA_CR_CT_SHIFT      			19

#define DMA_CR_DBM        				(1 << 18)
#define DMA_CR_DBM_SHIFT				18

#define DMA_CR_PL        				(1 << 16)
#define DMA_CR_PL_SHIFT	 				16
#define DMA_CR_PL_LOW	 				(0x0 << 16)
#define DMA_CR_PL_MEDIUM				(0x1 << 16)
#define DMA_CR_PL_HIGH	 				(0x2 << 16)
#define DMA_CR_PL_VERYHIGH 				(0x3 << 16)
#define DMA_CR_PL_MASK	 				(0x3 << 16)

#define DMA_CR_PINCOS    				(1 << 15)
#define DMA_CR_PINCOS_SHIFT  			15

#define DMA_CR_MSIZE      				(1 << 13)
#define DMA_CR_MSIZE_SHIFT 				13
#define DMA_CR_MSIZE_BYTE      			(0x0 << 13)
#define DMA_CR_MSIZE_HALFWORD      		(0x1 << 13)
#define DMA_CR_MSIZE_WORD      			(0x2 << 13)
#define DMA_CR_MSIZE_MASK      			(0x3 << 13)

#define DMA_CR_PSIZE      				(1 << 11)
#define DMA_CR_PSIZE_SHIFT 				11
#define DMA_CR_PSIZE_BYTE      			(0x0 << 11)
#define DMA_CR_PSIZE_HALFWORD      		(0x1 << 11)
#define DMA_CR_PSIZE_WORD      			(0x2 << 11)
#define DMA_CR_PSIZE_MASK      			(0x3 << 11)


#define DMA_CR_MINC      				(1 << 10)
#define DMA_CR_MINC_SHIFT		      	10


#define DMA_CR_PINC     				(1 << 9)
#define DMA_CR_PINC_SHIFT		       	9


#define DMA_CR_CIRC      				(1 << 8)
#define DMA_CR_CIRC_SHIFT		    	8

#define DMA_CR_DIR      				(1 << 6)
#define DMA_CR_DIR_SHIFT		      	6

#define DMA_CR_PFCTRL  					(1 << 5)
#define DMA_CR_PFCTRL_SHIFT     		5

#define DMA_CR_TCIE       				(1 << 4)
#define DMA_CR_TCIE_SHIFT		       	4

#define DMA_CR_HTIE     			  	(1 << 3)
#define DMA_CR_HTIE_SHIFT      			3

#define DMA_CR_TEIE      				(1 << 2)
#define DMA_CR_TEIE_SHIFT       		2

#define DMA_CR_DMEIE    			  	(1 << 1)
#define DMA_CR_DMEIE_SHIFT      		1

#define DMA_CR_EN         				(1 << 0)
#define DMA_CR_EN_SHIFT         		0


/* --- DMA_NDTR values ----------------------------------------------------- */
#define DMA_NDTR_NDT         			(1 << 0)
#define DMA_NDTR_NDT_SHIFT         		0
#define DMA_NDTR_NDT_MASK         		(0x10000 << 0)

/* --- DMA_FCR values ----------------------------------------------------- */

#define DMA_FCR_FEIE     				(1 << 7)
#define DMA_FCR_FEIE_SHIFT     			7

#define DMA_FCR_FS     					(1 << 3)
#define DMA_FCR_FS_SHIFT 				3
#define DMA_FCR_FS_0_TO_25     			(0x0 << 3)
#define DMA_FCR_FS_25_TO_50     		(0x1 << 3)
#define DMA_FCR_FS_50_TO_75     		(0x2 << 3)
#define DMA_FCR_FS_75_TO_100     		(0x3 << 3)
#define DMA_FCR_FS_EMPTY    			(0x4 << 3)
#define DMA_FCR_FS_FULL    				(0x5 << 3)
#define DMA_FCR_FS_MASK   				(0x5 << 3)


#define DMA_FCR_DMDIS	    			(1 << 2)
#define DMA_FCR_DMDIS_SHIFT	  			2

#define DMA_FCR_FTH     				(1 << 0)
#define DMA_FCR_FTH_SHIFT     			0
#define DMA_FCR_FTH_25     				(0x0 << 0)
#define DMA_FCR_FTH_50     				(0x1 << 0)
#define DMA_FCR_FTH_75     				(0x2 << 0)
#define DMA_FCR_FTH_FULL   				(0x3 << 0)

#endif
