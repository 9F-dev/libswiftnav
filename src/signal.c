/*
 * Copyright (c) 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swift-nav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <assert.h>

#include <swiftnav/array_tools.h>
#include <swiftnav/constants.h>
#include <swiftnav/glo_map.h>
#include <swiftnav/signal.h>

/** \defgroup signal GNSS signal identifiers (SID)
 * \{ */

/** Element in the code data table. */
typedef struct {
  constellation_t constellation;
  u16 sat_count;
  u16 me_sig_count;
  u16 sat_start;
  const char str[12];
  double carr_freq;
  u32 chip_count;
  double chip_rate;
  bool requires_direct_acq;
  u16 prn_period_ms;
  double carr_to_code;
  float sv_doppler_max;
  bool requires_data_decoder;
} code_table_element_t;

/** Table of useful data for each code. */
static const code_table_element_t code_table[CODE_COUNT] = {
        /** GPS */
        [CODE_GPS_L1CA] = {CONSTELLATION_GPS, NUM_SATS_GPS,
                           NUM_SIGNALS_GPS_L1CA, GPS_FIRST_PRN, "GPS L1CA",
                           GPS_L1_HZ, GPS_L1CA_CHIPS_NUM, GPS_CA_CHIPPING_RATE,
                           true, GPS_L1CA_PRN_PERIOD_MS, GPS_L1CA_CARR_TO_CODE,
                           GPS_L1_DOPPLER_MAX_HZ, true},
        [CODE_AUX_GPS] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L1CA,
                          GPS_FIRST_PRN, "GPS AUX", GPS_L1_HZ,
                          GPS_L1CA_CHIPS_NUM, GPS_CA_CHIPPING_RATE, false,
                          GPS_L1CA_PRN_PERIOD_MS, GPS_L1CA_CARR_TO_CODE,
                          GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L1CI] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L1C,
                           GPS_FIRST_PRN, "GPS L1CI", GPS_L1_HZ,
                           GPS_L1C_CHIPS_NUM, GPS_CA_CHIPPING_RATE, false,
                           GPS_L1C_PRN_PERIOD_MS, GPS_L1CA_CARR_TO_CODE,
                           GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L1CQ] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L1C,
                           GPS_FIRST_PRN, "GPS L1CQ", GPS_L1_HZ, 0, 0, false, 0,
                           0, GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L1CX] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L1C,
                           GPS_FIRST_PRN, "GPS L1C", GPS_L1_HZ, 0, 0, false, 0,
                           0, GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L2CM] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L2C,
                           GPS_FIRST_PRN, "GPS L2CM", GPS_L2_HZ,
                           GPS_L2CM_CHIPS_NUM, GPS_CA_CHIPPING_RATE, false,
                           GPS_L2CM_PRN_PERIOD_MS, GPS_L2C_CARR_TO_CODE,
                           GPS_L2_DOPPLER_MAX_HZ, true},
        [CODE_GPS_L2CL] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L2C,
                           GPS_FIRST_PRN, "GPS L2CL", GPS_L2_HZ,
                           GPS_L2CL_CHIPS_NUM, GPS_CA_CHIPPING_RATE, false,
                           GPS_L2CL_PRN_PERIOD_MS, GPS_L2C_CARR_TO_CODE,
                           GPS_L2_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L2CX] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L2C,
                           GPS_FIRST_PRN, "GPS L2C", GPS_L2_HZ, 0, 0, false, 0,
                           0, GPS_L2_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L5I] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L5,
                          GPS_FIRST_PRN, "GPS L5I", GPS_L5_HZ, GPS_L5_CHIPS_NUM,
                          GPS_L5_CHIPPING_RATE, false, GPS_L5_PRN_PERIOD_MS,
                          GPS_L5_CARR_TO_CODE, GPS_L5_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L5Q] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L5,
                          GPS_FIRST_PRN, "GPS L5Q", GPS_L5_HZ, 0, 0, false, 0,
                          0, GPS_L5_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L5X] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L5,
                          GPS_FIRST_PRN, "GPS L5", GPS_L5_HZ, 0, 0, false, 0, 0,
                          GPS_L5_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L1P] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L1P,
                          GPS_FIRST_PRN, "GPS L1P", GPS_L1_HZ, 0, 0, false, 0,
                          0, GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_GPS_L2P] = {CONSTELLATION_GPS, NUM_SATS_GPS, NUM_SIGNALS_GPS_L2P,
                          GPS_FIRST_PRN, "GPS L2P", GPS_L2_HZ, 0, 0, false, 0,
                          0, GPS_L2_DOPPLER_MAX_HZ, false},

        /** SBAS */
        [CODE_SBAS_L1CA] = {CONSTELLATION_SBAS, NUM_SATS_SBAS,
                            NUM_SIGNALS_SBAS_L1CA, SBAS_FIRST_PRN, "SBAS L1",
                            SBAS_L1_HZ, SBAS_L1CA_CHIPS_NUM,
                            SBAS_L1CA_CHIPPING_RATE, true,
                            SBAS_L1CA_PRN_PERIOD_MS, SBAS_L1CA_CARR_TO_CODE,
                            SBAS_L1_DOPPLER_MAX_HZ, true},
        [CODE_AUX_SBAS] = {CONSTELLATION_SBAS, NUM_SATS_SBAS,
                           NUM_SIGNALS_SBAS_L1CA, SBAS_FIRST_PRN, "SBAS AUX",
                           SBAS_L1_HZ, SBAS_L1CA_CHIPS_NUM,
                           SBAS_L1CA_CHIPPING_RATE, false,
                           SBAS_L1CA_PRN_PERIOD_MS, SBAS_L1CA_CARR_TO_CODE,
                           SBAS_L1_DOPPLER_MAX_HZ, false},
        [CODE_SBAS_L5I] = {CONSTELLATION_SBAS, NUM_SATS_SBAS,
                           NUM_SIGNALS_SBAS_L5, SBAS_FIRST_PRN, "SBAS L5I",
                           SBAS_L5_HZ, SBAS_L5_CHIPS_NUM, SBAS_L5_CHIPPING_RATE,
                           false, SBAS_L5_PRN_PERIOD_MS, SBAS_L5_CARR_TO_CODE,
                           SBAS_L5_DOPPLER_MAX_HZ, true},
        [CODE_SBAS_L5Q] = {CONSTELLATION_SBAS, NUM_SATS_SBAS,
                           NUM_SIGNALS_SBAS_L5, SBAS_FIRST_PRN, "SBAS L5Q",
                           SBAS_L5_HZ, SBAS_L5_CHIPS_NUM, SBAS_L5_CHIPPING_RATE,
                           false, SBAS_L5_PRN_PERIOD_MS, SBAS_L5_CARR_TO_CODE,
                           SBAS_L5_DOPPLER_MAX_HZ, false},
        [CODE_SBAS_L5X] = {CONSTELLATION_SBAS, NUM_SATS_SBAS,
                           NUM_SIGNALS_SBAS_L5, SBAS_FIRST_PRN, "SBAS L5",
                           SBAS_L5_HZ, SBAS_L5_CHIPS_NUM, SBAS_L5_CHIPPING_RATE,
                           false, SBAS_L5_PRN_PERIOD_MS, SBAS_L5_CARR_TO_CODE,
                           SBAS_L5_DOPPLER_MAX_HZ, false},

        /** Glonass  */
        [CODE_GLO_L1OF] = {CONSTELLATION_GLO, NUM_SATS_GLO, NUM_FREQ_GLO_L1OF,
                           GLO_FIRST_PRN, "GLO L1OF", GLO_L1_HZ,
                           GLO_CA_CHIPS_NUM, GLO_CA_CHIPPING_RATE, true,
                           GLO_PRN_PERIOD_MS, GLO_L1_CARR_TO_CODE(0),
                           GLO_L1_DOPPLER_MAX_HZ, true},
        [CODE_GLO_L2OF] = {CONSTELLATION_GLO, NUM_SATS_GLO, NUM_FREQ_GLO_L2OF,
                           GLO_FIRST_PRN, "GLO L2OF", GLO_L2_HZ,
                           GLO_CA_CHIPS_NUM, GLO_CA_CHIPPING_RATE, false,
                           GLO_PRN_PERIOD_MS, GLO_L2_CARR_TO_CODE(0),
                           GLO_L2_DOPPLER_MAX_HZ, true},
        [CODE_GLO_L1P] = {CONSTELLATION_GLO, NUM_SATS_GLO, NUM_FREQ_GLO_L1OF,
                          GLO_FIRST_PRN, "GLO L1P", GLO_L1_HZ, 0, 0, false, 0,
                          0, GLO_L1_DOPPLER_MAX_HZ, false},
        [CODE_GLO_L2P] = {CONSTELLATION_GLO, NUM_SATS_GLO, NUM_FREQ_GLO_L2OF,
                          GLO_FIRST_PRN, "GLO L2P", GLO_L2_HZ, 0, 0, false, 0,
                          0, GLO_L2_DOPPLER_MAX_HZ, false},

        /** Galileo  */
        [CODE_GAL_E1B] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E1,
                          GAL_FIRST_PRN, "GAL E1B", GAL_E1_HZ,
                          GAL_E1B_CHIPS_NUM, GAL_E1_CHIPPING_RATE, true,
                          GAL_E1B_PRN_PERIOD_MS, GAL_E1_CARR_TO_CODE,
                          GAL_E1_DOPPLER_MAX_HZ, true},
        [CODE_GAL_E1C] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E1,
                          GAL_FIRST_PRN, "GAL E1C", GAL_E1_HZ, 0, 0, false, 0,
                          0, GAL_E1_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E1X] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E1,
                          GAL_FIRST_PRN, "GAL E1", GAL_E1_HZ, 0, 0, false, 0, 0,
                          GAL_E1_DOPPLER_MAX_HZ, false},
        [CODE_AUX_GAL] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E1,
                          GAL_FIRST_PRN, "GAL AUX", GAL_E1_HZ,
                          GAL_E1B_CHIPS_NUM, GAL_E1_CHIPPING_RATE, false,
                          GAL_E1B_PRN_PERIOD_MS, GAL_E1_CARR_TO_CODE,
                          GAL_E1_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E6B] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E6,
                          GAL_FIRST_PRN, "GAL E6B", GAL_E6_HZ, GAL_E6_CHIPS_NUM,
                          GAL_E6_CHIPPING_RATE, false, GAL_E6B_PRN_PERIOD_MS,
                          GAL_E6_CARR_TO_CODE, GAL_E6_DOPPLER_MAX_HZ, true},
        [CODE_GAL_E6C] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E6,
                          GAL_FIRST_PRN, "GAL E6C", GAL_E6_HZ, 0, 0, false, 0,
                          0, GAL_E6_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E6X] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E6,
                          GAL_FIRST_PRN, "GAL E6", GAL_E6_HZ, 0, 0, false, 0, 0,
                          GAL_E6_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E7I] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E7,
                          GAL_FIRST_PRN, "GAL E5bI", GAL_E7_HZ,
                          GAL_E7_CHIPS_NUM, GAL_E7_CHIPPING_RATE, false,
                          GAL_E7I_PRN_PERIOD_MS, GAL_E7_CARR_TO_CODE,
                          GAL_E7_DOPPLER_MAX_HZ, true},
        [CODE_GAL_E7Q] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E7,
                          GAL_FIRST_PRN, "GAL E5bQ", GAL_E7_HZ, 0, 0, false, 0,
                          0, GAL_E7_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E7X] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E7,
                          GAL_FIRST_PRN, "GAL E5b", GAL_E7_HZ, 0, 0, false, 0,
                          0, GAL_E7_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E8I] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E8,
                          GAL_FIRST_PRN, "GAL E8I", GAL_E8_HZ, 0, 0, false, 0, 0,
                          GAL_E8_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E8Q] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E8,
                          GAL_FIRST_PRN, "GAL E8Q", GAL_E8_HZ, 0, 0, false, 0, 0,
                          GAL_E8_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E8X] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E8,
                          GAL_FIRST_PRN, "GAL E8", GAL_E8_HZ, 0, 0, false, 0, 0,
                          GAL_E8_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E5I] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E5,
                          GAL_FIRST_PRN, "GAL E5aI", GAL_E5_HZ,
                          GAL_E5_CHIPS_NUM, GAL_E5_CHIPPING_RATE, false,
                          GAL_E5I_PRN_PERIOD_MS, GAL_E5_CARR_TO_CODE,
                          GAL_E5_DOPPLER_MAX_HZ, true},
        [CODE_GAL_E5Q] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E5,
                          GAL_FIRST_PRN, "GAL E5aQ", GAL_E5_HZ, 0, 0, false, 0,
                          0, GAL_E5_DOPPLER_MAX_HZ, false},
        [CODE_GAL_E5X] = {CONSTELLATION_GAL, NUM_SATS_GAL, NUM_SIGNALS_GAL_E5,
                          GAL_FIRST_PRN, "GAL E5a", GAL_E5_HZ, 0, 0, false, 0,
                          0, GAL_E5_DOPPLER_MAX_HZ, false},

        /** Beidou */
        [CODE_BDS2_B1] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS2_B1,
                          BDS_FIRST_PRN, "BDS B1", BDS2_B11_HZ,
                          BDS2_B11_CHIPS_NUM, BDS2_B11_CHIPPING_RATE, true,
                          BDS2_B11_PRN_PERIOD_MS, BDS2_B11_CARR_TO_CODE,
                          BDS2_B11_DOPPLER_MAX_HZ, true},
        [CODE_AUX_BDS] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS2_B1,
                          BDS_FIRST_PRN, "BDS AUX", BDS2_B11_HZ,
                          BDS2_B11_CHIPS_NUM, BDS2_B11_CHIPPING_RATE, false,
                          BDS2_B11_PRN_PERIOD_MS, BDS2_B11_CARR_TO_CODE,
                          BDS2_B11_DOPPLER_MAX_HZ, false},
        [CODE_BDS2_B2] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS2_B2,
                          BDS_FIRST_PRN, "BDS B2", BDS2_B2_HZ,
                          BDS2_B2_CHIPS_NUM, BDS2_B2_CHIPPING_RATE, false,
                          BDS2_B2_PRN_PERIOD_MS, BDS2_B2_CARR_TO_CODE,
                          BDS2_B2_DOPPLER_MAX_HZ, true},
        [CODE_BDS3_B1CI] = {CONSTELLATION_BDS, NUM_SATS_BDS,
                            NUM_SIGNALS_BDS3_B1C, BDS_FIRST_PRN, "BDS3 B1CI",
                            BDS3_B1C_HZ, BDS3_B1C_CHIPS_NUM,
                            BDS3_B1C_CHIPPING_RATE, false,
                            BDS3_B1C_PRN_PERIOD_MS, BDS3_B1C_CARR_TO_CODE,
                            BDS3_B1C_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B1CQ] = {CONSTELLATION_BDS, NUM_SATS_BDS,
                            NUM_SIGNALS_BDS3_B1C, BDS_FIRST_PRN, "BDS3 B1CQ",
                            BDS3_B1C_HZ, 0, 0, false, 0, 0,
                            BDS3_B1C_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B1CX] = {CONSTELLATION_BDS, NUM_SATS_BDS,
                            NUM_SIGNALS_BDS3_B1C, BDS_FIRST_PRN, "BDS3 B1C",
                            BDS3_B1C_HZ, 0, 0, false, 0, 0,
                            BDS3_B1C_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B3I] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B3,
                           BDS_FIRST_PRN, "BDS3 B3I", BDS3_B3_HZ,
                           BDS3_B3_CHIPS_NUM, BDS3_B3_CHIPPING_RATE, false,
                           BDS3_B3_PRN_PERIOD_MS, BDS3_B3_CARR_TO_CODE,
                           BDS3_B3_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B3Q] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B3,
                           BDS_FIRST_PRN, "BDS3 B3Q", BDS3_B3_HZ, 0, 0, false,
                           0, 0, BDS3_B3_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B3X] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B3,
                           BDS_FIRST_PRN, "BDS3 B3", BDS3_B3_HZ, 0, 0, false,
                           0, 0, BDS3_B3_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B7I] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B7,
                           BDS_FIRST_PRN, "BDS3 B7I", BDS3_B7_HZ,
                           BDS3_B7_CHIPS_NUM, BDS3_B7_CHIPPING_RATE, false,
                           BDS3_B7_PRN_PERIOD_MS, BDS3_B7_CARR_TO_CODE,
                           BDS3_B7_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B7Q] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B7,
                           BDS_FIRST_PRN, "BDS3 B7Q", BDS3_B7_HZ, 0, 0, false,
                           0, 0, BDS3_B7_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B7X] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B7,
                           BDS_FIRST_PRN, "BDS3 B7", BDS3_B7_HZ,
                           BDS3_B7_CHIPS_NUM, BDS3_B7_CHIPPING_RATE, false,
                           BDS3_B7_PRN_PERIOD_MS, BDS3_B7_CARR_TO_CODE,
                           BDS3_B7_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B5I] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B5,
                           BDS_FIRST_PRN, "BDS3 B5I", BDS3_B3_HZ,
                           BDS3_B5_CHIPS_NUM, BDS3_B5_CHIPPING_RATE, false,
                           BDS3_B5_PRN_PERIOD_MS, BDS3_B5_CARR_TO_CODE,
                           BDS3_B5_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B5Q] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B5,
                           BDS_FIRST_PRN, "BDS3 B5Q", BDS3_B3_HZ, 0, 0, false,
                           0, 0, BDS3_B5_DOPPLER_MAX_HZ, false},
        [CODE_BDS3_B5X] = {CONSTELLATION_BDS, NUM_SATS_BDS, NUM_SIGNALS_BDS3_B5,
                           BDS_FIRST_PRN, "BDS3 B5", BDS3_B3_HZ, 0, 0, false,
                           0, 0, BDS3_B5_DOPPLER_MAX_HZ, false},

        /** QZS L1C/A has all the same characteristics as GPS L1 C/A */
        [CODE_QZS_L1CA] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L1,
                           QZS_FIRST_PRN, "QZS L1CA", QZS_L1_HZ,
                           QZS_L1CA_CHIPS_NUM, QZS_L1CA_CHIPPING_RATE, true,
                           QZS_L1CA_PRN_PERIOD_MS, QZS_L1CA_CARR_TO_CODE,
                           QZS_L1_DOPPLER_MAX_HZ, true},
        [CODE_AUX_QZS] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L1,
                          QZS_FIRST_PRN, "QZS AUX", QZS_L1_HZ,
                          QZS_L1CA_CHIPS_NUM, QZS_L1CA_CHIPPING_RATE, false,
                          QZS_L1CA_PRN_PERIOD_MS, QZS_L1CA_CARR_TO_CODE,
                          QZS_L1_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L1CI] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L1C,
                           GPS_FIRST_PRN, "QZSS L1CI", QZS_L1_HZ,
                           GPS_L1C_CHIPS_NUM, GPS_CA_CHIPPING_RATE, false,
                           GPS_L1C_PRN_PERIOD_MS, GPS_L1CA_CARR_TO_CODE,
                           GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L1CQ] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L1C,
                           GPS_FIRST_PRN, "QZSS L1CQ", QZS_L1_HZ, 0, 0, false,
                           0, 0, GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L1CX] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L1C,
                           GPS_FIRST_PRN, "QZSS L1CX", QZS_L1_HZ, 0, 0, false,
                           0, 0, GPS_L1_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L2CM] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L2C,
                           QZS_FIRST_PRN, "QZS L2CM", GPS_L2_HZ,
                           GPS_L2CM_CHIPS_NUM, QZS_L1CA_CHIPPING_RATE, false,
                           GPS_L2CM_PRN_PERIOD_MS, QZS_L2C_CARR_TO_CODE,
                           QZS_L2_DOPPLER_MAX_HZ, true},
        [CODE_QZS_L2CL] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L2C,
                           QZS_FIRST_PRN, "QZS L2CL", GPS_L2_HZ,
                           GPS_L2CL_CHIPS_NUM, QZS_L1CA_CHIPPING_RATE, false,
                           GPS_L2CL_PRN_PERIOD_MS, QZS_L2C_CARR_TO_CODE,
                           QZS_L2_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L2CX] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L2C,
                           QZS_FIRST_PRN, "QZS L2C", GPS_L2_HZ, 0, 0, false, 0,
                           0, QZS_L2_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L5I] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L5,
                          QZS_FIRST_PRN, "QZS L5I", QZS_L5_HZ, 0, 0, false, 0,
                          0, QZS_L5_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L5Q] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L5,
                          QZS_FIRST_PRN, "QZS L5Q", QZS_L5_HZ, 0, 0, false, 0,
                          0, QZS_L5_DOPPLER_MAX_HZ, false},
        [CODE_QZS_L5X] = {CONSTELLATION_QZS, NUM_SATS_QZS, NUM_SIGNALS_QZS_L5,
                          QZS_FIRST_PRN, "QZS L5", QZS_L5_HZ, 0, 0, false, 0, 0,
                          QZS_L5_DOPPLER_MAX_HZ, false},
};

typedef struct { u8 prn_list[MAX_SBAS_SATS_PER_SYSTEM]; } sbas_prn_table_t;

/** Active SBAS PRNs as of March 2018, sources:
 *  http://gpsworld.com/the-almanac/
 *  https://egnos-user-support.essp-sas.eu/new_egnos_ops/egnos_system_realtime
 */
static const sbas_prn_table_t sbas_prn_table[SBAS_COUNT] = {
        [SBAS_WAAS] = {{131, 135, 138}},  /* PRN 131 in test mode */
        [SBAS_EGNOS] = {{120, 123, 136}}, /* PRN 136 in test mode */
        [SBAS_GAGAN] = {{127, 128, 132}}, /* PRN 132 is in reserve */
        [SBAS_MSAS] = {{129, 137}},
};

/** String representation used for unknown code values. */
static const char *unknown_str = "?";

/** Construct a gnss_signal_t.
 *
 * \note This function does not check the validity of the resulting signal.
 *
 * \param code  Code to use.
 * \param sat   Satellite identifier to use.
 *
 * \return gnss_signal_t corresponding to the specified arguments.
 */
gnss_signal_t construct_sid(code_t code, u16 sat) {
  gnss_signal_t sid = {.code = code, .sat = sat};
  return sid;
}

/** Construct a me_gnss_signal_t.
 *
 * \note This function does not check the validity of the resulting signal.
 *
 * \param code  Code to use.
 * \param sat   Satellite identifier to use.
 *
 * \return me_gnss_signal_t corresponding to the specified arguments.
 */
me_gnss_signal_t construct_mesid(code_t code, u16 sat) {
  me_gnss_signal_t mesid = {.code = code, .sat = sat};
  return mesid;
}

/** Construct a gnss_signal_t from input me_gnss_signal_t.
 *
 * \param mesid        ME signal to use.
 * \param glo_slot_id  GLO orbital slot.
 *
 * \return gnss_signal_t corresponding to the specified argument.
 */
gnss_signal_t mesid2sid(const me_gnss_signal_t mesid, u16 glo_slot_id) {
  assert(mesid_valid(mesid));
  gnss_signal_t sid;
  if (IS_GLO(mesid)) {
    assert(glo_slot_id_is_valid(glo_slot_id));
    sid = construct_sid(mesid.code, glo_slot_id);
  } else {
    sid = construct_sid(mesid.code, mesid.sat);
  }
  return sid;
}

/** Print a string representation of a sat code.
 *
 * \param str_buf     Buffer of capacity str_buf_len,
 *                    to which the string will be written.
 * \param suffix_len  Length of suffix.
 * \param suffix      Suffix string to be printed.
 * \param sat         Sat identifier.
 * \param code        Code identifier.
 *
 * \return Number of characters written to s, excluding the terminating null.
 */
int sat_code_to_string(char *str_buf, size_t suffix_len, const char *suffix,
                       u16 sat, code_t code) {
  const char *code_str =
      ((code < 0) || (code >= CODE_COUNT)) ? unknown_str : code_table[code].str;
  int nchars = 0;

  /* Copy code string */
  for (u32 i = 0; code_str[i] != 0; i++) {
    str_buf[nchars++] = code_str[i];
  }

  /* Print suffix */
  for (u32 i = 0; i < suffix_len; i++) {
    str_buf[nchars++] = suffix[i];
  }

  /* Print sat value */
  bool started = false;
  u16 div = 10000;
  while (div > 0) {
    u8 digit = (sat / div) % 10;
    div /= 10;
    if (started || (digit != 0)) {
      str_buf[nchars++] = '0' + digit;
      started = true;
    }
  }
  /* Handle special case where sat = 0 */
  if (!started) {
    str_buf[nchars++] = '0';
  }
  str_buf[nchars] = 0;
  if (nchars >= SID_STR_LEN_MAX) {
    log_error("%d: %s", nchars, str_buf);
  }
  assert(nchars < SID_STR_LEN_MAX);
  return nchars;
}

/** Print a string representation of a gnss_signal_t.
 *
 * \param s   Buffer of capacity n to which the string will be written.
 * \param n   Capacity of buffer s.
 * \param sid gnss_signal_t to use.
 *
 * \return Number of characters written to s, excluding the terminating null.
 */
int sid_to_string(char *s, int n, const gnss_signal_t sid) {
  assert(n >= SID_STR_LEN_MAX);
  (void)n;
  return sat_code_to_string(s, SID_SUFFIX_LENGTH, /* suffix = */ " ", sid.sat,
                            sid.code);
}

/** Print a string representation of a me_gnss_signal_t.
 *
 * \param s     Buffer of capacity n to which the string will be written.
 * \param n     Capacity of buffer s.
 * \param mesid me_gnss_signal_t to use.
 *
 * \return Number of characters written to s, excluding the terminating null.
 */
int mesid_to_string(char *s, int n, const me_gnss_signal_t mesid) {
  assert(n >= MESID_STR_LEN_MAX);
  (void)n;
  return sat_code_to_string(s, MESID_SUFFIX_LENGTH, /* suffix = */ " ME ",
                            mesid.sat, mesid.code);
}

/** Determine if a gnss_signal_t corresponds to a known code and
 * satellite identifier.
 *
 * \param sid   gnss_signal_t to use.
 *
 * \return true if sid exists, false otherwise.
 */
bool sid_valid(gnss_signal_t sid) {
  if (!code_valid(sid.code)) {
    return false;
  }

  const code_table_element_t *e = &code_table[sid.code];
  if ((sid.sat < e->sat_start) || (sid.sat >= e->sat_start + e->sat_count)) {
    return false;
  }

  return true;
}

/** Determine if a me_gnss_signal_t corresponds to a known code and
 * ME satellite identifier.
 *
 * \param mesid   me_gnss_signal_t to use.
 *
 * \return true if mesid exists, false otherwise.
 */
bool mesid_valid(const me_gnss_signal_t mesid) {
  if (!code_valid(mesid.code)) {
    return false;
  }

  const code_table_element_t *e = &code_table[mesid.code];
  if ((mesid.sat < e->sat_start) ||
      (mesid.sat >= e->sat_start + e->me_sig_count)) {
    log_debug_mesid(mesid, "mesid.sat %u e->start %u e->me_sig_count %u",
                    mesid.sat, e->sat_start, e->me_sig_count);
    return false;
  }

  return true;
}

/** Determine if a constellation is valid.
 *
 * \param constellation   Constellation to use.
 *
 * \return true if constellation is valid, false otherwise
 */
bool constellation_valid(constellation_t constellation) {
  return ((constellation >= 0) && (constellation < CONSTELLATION_COUNT));
}

/** Convert a code-specific signal index to a gnss_signal_t.
 *
 * \param code          Code to use.
 * \param code_index    Code-specific signal index in
 *                      [0, SIGNAL_COUNT_\<code\>).
 *
 * \return gnss_signal_t corresponding to code and sat_index.
 */
gnss_signal_t sid_from_code_index(code_t code, u16 sat_index) {
  assert(code_valid(code));
  assert(sat_index < code_table[code].sat_count);
  return construct_sid(code, code_table[code].sat_start + sat_index);
}

/** Convert a code-specific ME signal index to a me_gnss_signal_t.
 *
 * \param code          Code to use.
 * \param me_code_index ME code-specific signal index in
 *                      [0, ACQ_TRACK_COUNT_\<code\>).
 *
 * \return me_gnss_signal_t corresponding to code and code_index.
 */
me_gnss_signal_t mesid_from_code_index(code_t code, u16 me_code_index) {
  assert(code_valid(code));
  assert(me_code_index < code_table[code].me_sig_count);
  return construct_mesid(code, code_table[code].sat_start + me_code_index);
}

/** Return the code-specific signal index for a gnss_signal_t.
 *
 * \param sid   gnss_signal_t to use.
 *
 * \return Code-specific signal index in [0, SIGNAL_COUNT_\<code\>).
 */
u16 sid_to_code_index(gnss_signal_t sid) {
  assert(sid_valid(sid));
  return sid.sat - code_table[sid.code].sat_start;
}

/** Return the code-specific signal index for a me_gnss_signal_t.
 *
 * \param mesid me_gnss_signal_t to use.
 *
 * \return Code-specific signal index in [0, SIGNAL_COUNT_\<code\>).
 */
u16 mesid_to_code_index(const me_gnss_signal_t mesid) {
  assert(mesid_valid(mesid));
  return mesid.sat - code_table[mesid.code].sat_start;
}

/** Get the constellation to which a gnss_signal_t belongs.
 *
 * \param sid   gnss_signal_t to use.
 *
 * \return Constellation to which sid belongs.
 */
constellation_t sid_to_constellation(gnss_signal_t sid) {
  return code_to_constellation(sid.code);
}

/** Get the constellation to which a me_gnss_signal_t belongs.
 *
 * \param mesid   me_gnss_signal_t to use.
 *
 * \return Constellation to which mesid belongs.
 */
constellation_t mesid_to_constellation(const me_gnss_signal_t mesid) {
  return code_to_constellation(mesid.code);
}

/** Return the carrier frequency for a mesid.
 *
 * \param mesid  me_gnss_signal_t to use.
 * \return carrier frequency
 */
double mesid_to_carr_freq(const me_gnss_signal_t mesid) {
  code_t code = mesid.code;
  assert(code_valid(code));
  /* Map GLO mesid.sat [1 - 14] -> GLO FCN [-7 - +6] */
  s8 fcn = mesid.sat - GLO_FCN_OFFSET;
  if (CODE_GLO_L1OF == code) {
    return GLO_L1_HZ + fcn * GLO_L1_DELTA_HZ;
  } else if (CODE_GLO_L2OF == code) {
    return GLO_L2_HZ + fcn * GLO_L2_DELTA_HZ;
  }
  /* there is no difference between mesid and sid for GPS */
  gnss_signal_t sid = construct_sid(mesid.code, mesid.sat);
  return sid_to_carr_freq(sid);
}

/** Get the constellation to which a code belongs.
 *
 * \param code  Code to use.
 *
 * \return Constellation to which code belongs.
 */
constellation_t code_to_constellation(code_t code) {
  assert(code_valid(code));
  return code_table[code].constellation;
}

/** Return the center carrier frequency for sid.
 *  NOTE: it's assumed GLO SV already mapped to frequency slot
 *
 * \param sid  SID to use.
 * \return center carrier frequency
 */
double sid_to_carr_freq(gnss_signal_t sid) {
  assert(code_valid(sid.code));
  /* Map GLO mesid.sat [1 - 14] -> GLO FCN [-7 - +6] */
  if (CODE_GLO_L1OF == sid.code) {
    assert(glo_map_valid(sid));
    return GLO_L1_HZ +
           (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) * GLO_L1_DELTA_HZ;
  } else if (CODE_GLO_L2OF == sid.code) {
    assert(glo_map_valid(sid));
    return GLO_L2_HZ +
           (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) * GLO_L2_DELTA_HZ;
  }
  /* for CDMA signals just take from table */
  return code_table[sid.code].carr_freq;
}

/** Return the wavelength of a carrier in a vacuum for a code_t
 *  NOTE: it's assumed GLO SV already mapped to frequency slot
 *
 * \param sid  SID to use.
 * \return center carrier frequency lambda [m]
 */
double sid_to_lambda(gnss_signal_t sid) {
  assert(code_valid(sid.code));
  /* Map GLO mesid.sat [1 - 14] -> GLO FCN [-7 - +6] */
  if (CODE_GLO_L1OF == sid.code) {
    assert(glo_map_valid(sid));
    return GPS_C / (GLO_L1_HZ +
                    (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) * GLO_L1_DELTA_HZ);
  } else if (CODE_GLO_L2OF == sid.code) {
    assert(glo_map_valid(sid));
    return GPS_C / (GLO_L2_HZ +
                    (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) * GLO_L2_DELTA_HZ);
  }
  /* for GPS just take from table */
  return GPS_C / code_table[sid.code].carr_freq;
}

/** Return the chips count for a code_t.
 *
 * \param code  code_t to use.
 * \return chips count
 */
u32 code_to_chip_count(code_t code) {
  assert(code_valid(code));
  return code_table[code].chip_count;
}

/** Return the chip rate for a code_t.
 *
 * \param code  code_t to use.
 * \return chip rate
 */
double code_to_chip_rate(code_t code) {
  assert(code_valid(code));
  return code_table[code].chip_rate;
}

/** Return the [carrier freq / code chip rate] for a code_t.
 *
 * \param mesid  me_gnss_signal_t to use.
 * \return [carrier freq / code chip rate]
 */
double mesid_to_carr_to_code(const me_gnss_signal_t mesid) {
  code_t code = mesid.code;
  assert(code_valid(code));
  /* Map GLO mesid.sat [1 - 14] -> GLO FCN [-7 - +6] */
  s8 fcn = mesid.sat - GLO_FCN_OFFSET;
  if (CODE_GLO_L1OF == code) {
    return GLO_L1_CARR_TO_CODE(fcn);
  } else if (CODE_GLO_L2OF == code) {
    return GLO_L2_CARR_TO_CODE(fcn);
  } else {
    return code_table[mesid.code].carr_to_code;
  }
}

/** Return the PRN period for a code_t.
 *
 * \param code  code_t to use.
 * \return code period in ms
 */
u16 code_to_prn_period_ms(code_t code) {
  assert(code_valid(code));
  return code_table[code].prn_period_ms;
}
/** Checks if the code requires direct acquisition.
 *
 * An example of non-direct acquisition is the L1C/A
 * handover to L2CM, which eliminates the direct acquisition
 * for L2CM signal.
 *
 * \param code  code_t to check.
 * \retval true Direct acquisition is required
 * \retval false Direct acquisition is not required
 */
bool code_requires_direct_acq(code_t code) {
  assert(code_valid(code));
  return code_table[code].requires_direct_acq;
}

/** Return the minimum Doppler value for a code_t induced by satellite motion.
 *
 * \param code  code_t to use.
 * \return Minimum Doppler value [Hz]
 */
float code_to_sv_doppler_min(code_t code) {
  assert(code_valid(code));

  return -code_table[code].sv_doppler_max;
}

/** Return the maximum Doppler value for a code_t induced by satellite motion.
 *
 * \param code  code_t to use.
 * \return Maximum Doppler value [Hz]
 */
float code_to_sv_doppler_max(code_t code) {
  assert(code_valid(code));

  return +code_table[code].sv_doppler_max;
}

/** Checks if code needs a data decoder.
 *
 *  For example GPS L2CL is pilot component, and does NOT need data decoder.
 *
 * \param  code  code_t to check.
 * \retval true  Data decoder is needed
 * \retval false Data decoder is not needed
 */
bool code_requires_decoder(code_t code) {
  assert(code_valid(code));
  return code_table[code].requires_data_decoder;
}

/**
 * Returns number of SV belongs to a constellation
 * \param gnss GNSS constellation
 * \return number of SV
 */
u16 constellation_to_sat_count(constellation_t gnss) {
  code_t code = constellation_to_l1_code(gnss);
  assert(code_valid(code));
  return code_table[code].sat_count;
}

/** Returns a pointer to the list of PRNs for the given SBAS provider */
const u8 *get_sbas_prn_list(sbas_system_t sbas_system) {
  assert(SBAS_NONE != sbas_system && SBAS_COUNT > sbas_system);
  return sbas_prn_table[sbas_system].prn_list;
}

/** Which system a given SBAS PRN belongs to */
sbas_system_t get_sbas_system(const gnss_signal_t sid) {
  assert(IS_SBAS(sid));
  for (sbas_system_t sbas_system = (sbas_system_t)0; sbas_system < SBAS_COUNT;
       sbas_system = (sbas_system_t)(sbas_system + 1)) {
    if (is_value_in_array(sbas_prn_table[sbas_system].prn_list,
                          MAX_SBAS_SATS_PER_SYSTEM, sid.sat)) {
      return sbas_system;
    }
  }
  return SBAS_NONE;
}

/* \} */
