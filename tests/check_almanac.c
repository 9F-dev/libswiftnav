#include <check.h>

#include <swiftnav/almanac.h>
#include "check_suites.h"

START_TEST(test_almanac_equal) {
  almanac_t a;
  almanac_t b;

  memset(&a, 0, sizeof(a));
  memset(&b, 0, sizeof(b));

  fail_unless(almanac_equal(&a, &b), "Almanacs should be equal");

  a.valid = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (valid)");
  memset(&a, 0, sizeof(a));

  a.health_bits = 0x3f;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (health_bits)");
  memset(&a, 0, sizeof(a));

  a.sid.sat = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (sid.sat)");
  memset(&a, 0, sizeof(a));

  a.sid.code = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (sid.band)");
  memset(&a, 0, sizeof(a));

  a.sid.code = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (sid.constellation)");
  memset(&a, 0, sizeof(a));

  a.toa.wn = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (toa.wn)");
  memset(&a, 0, sizeof(a));

  a.toa.tow = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (toa.tow)");
  memset(&a, 0, sizeof(a));

  a.ura = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (ura)");
  memset(&a, 0, sizeof(a));

  a.fit_interval = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (fit_interval)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.m0 = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.m0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.ecc = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.ecc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.sqrta = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.sqrta)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.omega0 = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.omega0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.omegadot = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.omegadot)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.w = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.w)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.inc = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.inc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.af0 = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.af0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.af1 = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (kepler.af1)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.pos[0] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.pos[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.pos[1] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.pos[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.pos[2] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.pos[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.vel[0] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.vel[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.vel[1] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.vel[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.vel[2] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.vel[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.acc[0] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.acc[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.acc[1] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.acc[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.acc[2] = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (xyz.acc[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.lambda = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (glo.lambda)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.t_lambda = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (glo.t_lambda)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.i = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (glo.i)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.t = 1;
  fail_unless(!almanac_equal(&a, &b), "Almanacs should not be equal (glo.t)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.t_dot = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (glo.t_dot)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.epsilon = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (glo.epsilon)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.omega = 1;
  fail_unless(!almanac_equal(&a, &b),
              "Almanacs should not be equal (glo.omega)");
  memset(&a, 0, sizeof(a));
}
END_TEST

START_TEST(test_doppler_hint) {
  /* load in the YUMA almanac for PRN 13 from NAVCEN on 10/18/2019
   * https://www.navcen.uscg.gov/?pageName=currentAlmanac&format=yuma-txt
   ********* Week 28 almanac for PRN-13 ********
      ID:                         13
      Health:                     000
      Eccentricity:               0.3928661346E-002
      Time of Applicability(s):  61440.0000
      Orbital Inclination(rad):   0.9682378875
      Rate of Right Ascen(r/s):  -0.7783181344E-008
      SQRT(A)  (m 1/2):           5153.612793
      Right Ascen at Week(rad):   0.2887932677E+001
      Argument of Perigee(rad):   1.194395791
      Mean Anom(rad):            -0.1298138732E+000
      Af0(s):                    -0.3814697266E-004
      Af1(s/s):                   0.3637978807E-011
      week:                        28
   **/
  almanac_kepler_t sat;
  almanac_t a;
  sat.m0 = -0.1298138732 / M_PI;  // radians to semi-circles
  sat.ecc = 0.3928661346E-002;
  sat.sqrta = 5153.612793;
  sat.omega0 = 0.2887932677E+001 / M_PI;
  sat.omegadot = -0.7783181344E-008 / M_PI;  // radians to semicircles
  sat.w = 1.194395791 / M_PI;
  sat.inc = 0.9682378875 / M_PI;
  sat.af0 = -0.3814697266E-004;
  sat.af1 = 0.3637978807E-011;
  a.sid.code = CODE_GPS_L1CA;
  a.sid.sat = 13;
  a.toa.wn = 2075;  // 28th after 2 rollovers 2047 + 28
  a.toa.tow = 61440;
  a.ura = 100;
  a.fit_interval = 2 * 7 * 24 * 60 * 60;  // two weeks
  a.valid = true;
  a.health_bits = 0;
  a.kepler = sat;

  /* choose reference as center of earth and current week time 2000*/
  gps_time_t calc_time;
  calc_time.wn = a.toa.wn;
  calc_time.tow = 495453;
  double ref_pos[3] = {0, 0, 0};
  double doppler;
  int ret;
  ret = calc_sat_doppler_almanac(&a, &calc_time, ref_pos, &doppler);
  /* doppler hint should be pretty darn close to 0 */
  fail_unless(ret == 0, "Doppler hint return should be valid");
  fail_unless(fabs(doppler) < 50,
              "Doppler hint at center of earth should be small");

  /* choose reference at SF office */
  ref_pos[0] = -2705105.947;
  ref_pos[1] = -4262044.267;
  ref_pos[2] = 3885382.625;
  ret = calc_sat_doppler_almanac(&a, &calc_time, ref_pos, &doppler);
  /* doppler hint should be around -2000 Hz at this time */
  fail_unless(ret == 0, "Doppler hint return should be valid");
  fail_unless(doppler < -2000 && doppler > -2500,
              "Doppler hint should be about -2000 Hz");
  double doppler2;
  double ref_vel[3];
  double sat_pos[3];
  double sat_vel[3];
  double sat_acc[3];
  double clock_err = 0;
  double clock_rate_err = 0;
  ret = calc_sat_state_almanac(
      &a, &calc_time, sat_pos, sat_vel, sat_acc, &clock_err, &clock_rate_err);
  fail_unless(ret == 0, "calc_sat_state return should be valid");
  /* make a unit vector for line of site vector to sat */
  double los_vect[3];
  los_vect[0] = sat_pos[0] - ref_pos[0];
  los_vect[1] = sat_pos[1] - ref_pos[1];
  los_vect[2] = sat_pos[2] - ref_pos[2];
  double mag_los = sqrt(los_vect[0] * los_vect[0] + los_vect[1] * los_vect[1] +
                        los_vect[2] * los_vect[2]);
  /* Add reference non-zero velocity opposite LOS vector that causes 100Hz of
   * doppler */
  ref_vel[0] = -los_vect[0] / mag_los * 100 / sid_to_carr_freq(a.sid) * GPS_C;
  ref_vel[1] = -los_vect[1] / mag_los * 100 / sid_to_carr_freq(a.sid) * GPS_C;
  ref_vel[2] = -los_vect[2] / mag_los * 100 / sid_to_carr_freq(a.sid) * GPS_C;
  ret = calc_sat_doppler_alm_vel(&a, &calc_time, ref_pos, ref_vel, &doppler2);
  fail_unless(ret == 0, "Doppler hint return should be valid");
  double doppler_change = doppler2 - doppler;
  /* Assert that the doppler has increased by the expected amount */
  fail_unless(fabs((doppler2 - doppler) - 100) < 0.000001,
              "L1 Doppler hint should have increased by exactly 100 Hz");
}
END_TEST

Suite *almanac_suite(void) {
  Suite *s = suite_create("Almanac");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_almanac_equal);
  tcase_add_test(tc_core, test_doppler_hint);
  suite_add_tcase(s, tc_core);

  return s;
}
