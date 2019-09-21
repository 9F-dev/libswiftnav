#include <check.h>
#include <inttypes.h>

#include <swiftnav/almanac.h>
#include <swiftnav/ephemeris.h>
#include <swiftnav/linear_algebra.h>
#include <swiftnav/shm.h>
#include "check_suites.h"

/* Set thresholds so high that the unit tests
 * for ephemeris - almanac cross checks actually pass.
 * These should be modified for actual tests. */
#define VALID_ALM_ACCURACY (500000)
#define VALID_EPH_ACCURACY (500000)

/* GPS ephemeris for tests.
 * See scenario ME-45. */
static const ephemeris_t gps_eph = {
    .sid = {.code = CODE_GPS_L1CA, .sat = 1},
    .toe = {.wn = 1916, .tow = 14400},
    .ura = 2.0,
    .fit_interval = 14400,
    .valid = 1,
    .health_bits = 0,
    .kepler = {.tgd.gps_s = {5.122274160385132E-9, 0.0},
               .crc = 198.9375,
               .crs = 10.28125,
               .cuc = 5.327165126800537E-7,
               .cus = 9.521842002868652E-6,
               .cic = -2.3655593395233154E-7,
               .cis = -3.91155481338501E-8,
               .dn = 4.5637615275575705E-9,
               .m0 = 2.167759779416001,
               .ecc = 0.005649387603625655,
               .sqrta = 5153.644334793091,
               .omega0 = 1.8718410336467348,
               .omegadot = -7.896400345341237E-9,
               .w = 0.4837085715349947,
               .inc = 0.9649728717477063,
               .inc_dot = 6.078824636017362E-10,
               .af0 = 2.5494489818811417E-5,
               .af1 = 1.2505552149377763E-12,
               .af2 = 0.0,
               .toc = {.wn = 1916, .tow = 14400},
               .iodc = 2,
               .iode = 2}};

  /** From table 4 in Thompson, Lewis & Brown (2019) */
  static const ephemeris_t gps_eph2 = {
    .sid = {.code = CODE_GPS_L1CA, .sat = 11},
    .toe = {.wn = 1983, .tow = 0},
    .ura = 2.0,
    .fit_interval = 14400,
    .valid = 1,
    .health_bits = 0,
    .kepler = {.tgd.gps_s = {5.122274160385132E-9, 0.0},
               .crc = 0.293218750000E+03,
               .crs = -0.965625000000E+01,
               .cuc = -0.379979610443E-06,
               .cus = 0.277347862720E-05,
               .cic = 0.199303030968E-06,
               .cis = 0.173225998878E-06,
               .dn = 0.583845748090E-08,
               .m0 = -0.286954703389E+01,
               .ecc = 0.167867515702E-01,
               .sqrta = 0.515375480270E+04,
               .omega0 = -0.657960408566E+00,
               .omegadot = -0.868929051526E-08,
               .w = 0.173129682312E+01,
               .inc = 0.903782727230E+00,
               .inc_dot = 0.789318592573E-10,
               .af0 = 2.5494489818811417E-5,
               .af1 = 1.2505552149377763E-12,
               .af2 = 0.0,
               .toc = {.wn = 1983, .tow = 0},
               .iodc = 2,
               .iode = 2}};

/* GPS almanac for tests.
 * See scenario ME-45. */
static const almanac_t gps_alm = {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
                                  .toa = {.wn = 1916, .tow = 53248},
                                  .ura = 900,
                                  .fit_interval = 504000,
                                  .valid = 1,
                                  .health_bits = 0,
                                  .kepler = {.m0 = 1.5509826579560628,
                                             .ecc = 0.005649566650390625,
                                             .sqrta = 5153.64453125,
                                             .omega0 = 1.8715344586823712,
                                             .omegadot = -7.897471818543825E-9,
                                             .w = 0.4837084091510879,
                                             .inc = 0.964996154674105,
                                             .af0 = 2.574920654296875E-5,
                                             .af1 = 0.0}};

START_TEST(test_ephemeris_almanac_divergence) {
  /* See scenario ME-45 description. */

  /* Initially just copy the original ephemeris. */
  ephemeris_t gps_eph_diverged;
  memcpy(&gps_eph_diverged, &gps_eph, sizeof(gps_eph_diverged));

  fail_unless(ephemeris_equal(&gps_eph, &gps_eph_diverged),
              "Ephemerides should be equal");

  /* Next, let's modify and diverge the ephemeris.
   * Further modify these if the test case changes. */
  gps_eph_diverged.kepler.dn = 10.4154338446262e-009;
  gps_eph_diverged.kepler.m0 = 2.16970122385066e+000;

  fail_unless(!ephemeris_equal(&gps_eph, &gps_eph_diverged),
              "Ephemerides should not be equal");

  /* First check point: start of the position test interval */
  gps_time_t t_start = gps_eph.toe;
  t_start.tow += -(double)gps_eph.fit_interval / 2.0;
  normalize_gps_time(&t_start);

  gps_time_t t = t_start;

  for (u8 i = 0; i < 3;
       ++i, t.tow += gps_eph.fit_interval / 2.0, normalize_gps_time(&t)) {
    double d;
    double _[3];
    double alm_sat_pos[3];
    double eph_sat_pos[3];
    double div_sat_pos[3];
    u8 iode;
    u16 iodc;

    bool calc_alm_ok =
        (0 == calc_sat_state_almanac(&gps_alm, &t, alm_sat_pos, _, _, _, _));
    bool calc_eph_ok =
        (0 ==
         calc_sat_state_n(&gps_eph, &t, eph_sat_pos, _, _, _, _, &iodc, &iode));
    bool calc_eph_div_ok =
        (0 ==
         calc_sat_state_n(
             &gps_eph_diverged, &t, div_sat_pos, _, _, _, _, &iodc, &iode));

    /* Check successful sat state calculation. */
    fail_unless(calc_alm_ok && calc_eph_ok && calc_eph_div_ok,
                "Failure computing sat state! \n"
                "Alm success: %" PRIu8
                " \n"
                "Eph success: %" PRIu8
                " \n"
                "Eph diverged success: %" PRIu8 " \n",
                calc_alm_ok,
                calc_eph_ok,
                calc_eph_div_ok);

    /* Almanac vs. diverged ephemeris comparison. */

    /* Compute distance [m] */
    d = fabs(vector_distance(3, alm_sat_pos, div_sat_pos));

    fail_unless(d <= VALID_ALM_ACCURACY,
                "Almanac vs. diverging  check failed: \n"
                "Iteration: %" PRIu8
                " \n"
                "Distance: %lf \n"
                "Alm_sat_pos_x: %lf, Diverged_sat_pos_x: %lf \n"
                "Alm_sat_pos_y: %lf, Diverged_sat_pos_y: %lf \n"
                "Alm_sat_pos_z: %lf, Diverged_sat_pos_z: %lf",
                i,
                d,
                alm_sat_pos[0],
                div_sat_pos[0],
                alm_sat_pos[1],
                div_sat_pos[1],
                alm_sat_pos[2],
                div_sat_pos[2]);

    /* Ephemeris vs. diverged ephemeris comparison. */

    /* Compute distance [m] */
    d = vector_distance(3, eph_sat_pos, div_sat_pos);

    fail_unless(d <= VALID_EPH_ACCURACY,
                "Ephemeris vs. diverging ephemeris check failed: \n"
                "Iteration: %" PRIu8
                " \n"
                "Distance: %lf \n"
                "Eph_sat_pos_x: %lf, Diverged_sat_pos_x: %lf \n"
                "Eph_sat_pos_y: %lf, Diverged_sat_pos_y: %lf \n"
                "Eph_sat_pos_z: %lf, Diverged_sat_pos_z: %lf",
                i,
                d,
                eph_sat_pos[0],
                div_sat_pos[0],
                eph_sat_pos[1],
                div_sat_pos[1],
                eph_sat_pos[2],
                div_sat_pos[2]);
  }
}
END_TEST

START_TEST(test_ephemeris_equal) {
  ephemeris_t a;
  ephemeris_t b;

  memset(&a, 0, sizeof(a));
  memset(&b, 0, sizeof(b));

  fail_unless(ephemeris_equal(&a, &b), "Ephemerides should be equal");

  a.valid = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (valid)");
  memset(&a, 0, sizeof(a));

  a.health_bits = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (healthy)");
  memset(&a, 0, sizeof(a));

  a.sid.sat = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (sid.sat)");
  memset(&a, 0, sizeof(a));

  a.sid.code = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (sid.band)");
  memset(&a, 0, sizeof(a));

  a.sid.code = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (sid.constellation)");
  memset(&a, 0, sizeof(a));

  a.toe.wn = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (toe.wn)");
  memset(&a, 0, sizeof(a));

  a.toe.tow = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (toe.tow)");
  memset(&a, 0, sizeof(a));

  a.ura = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (ura)");
  memset(&a, 0, sizeof(a));

  a.fit_interval = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (fit_interval)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.tgd.gps_s[0] = 1.0;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.tgd_gps_s)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.crs = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.crs)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.crc = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.crc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.cuc = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.cuc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.cus = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.cus)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.cic = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.cic)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.cis = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.cis)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.dn = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.dn)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.m0 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.m0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.ecc = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.ecc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.sqrta = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.sqrta)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.omega0 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.omega0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.omegadot = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.omegadot)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.w = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.w)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.inc = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.inc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.inc_dot = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.inc_dot)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.af0 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.af0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.af1 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.af1)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.af2 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.af2)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.iode = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.iode)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.iodc = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.iodc)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.toc.wn = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.toc.wn)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.kepler.toc.tow = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (kepler.toc.tow)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.pos[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.pos[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.pos[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.pos[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.pos[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.pos[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.vel[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.vel[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.vel[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.vel[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.vel[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.vel[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.acc[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.acc[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.acc[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.acc[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.acc[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.acc[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.a_gf0 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.a_gf0)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.xyz.a_gf1 = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (xyz.a_gf1)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.gamma = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.gamma)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.tau = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.tau)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.d_tau = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.d_tau)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.iod = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.iod)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.fcn = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.fcn)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.pos[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.pos[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.pos[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.pos[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.pos[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.pos[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.vel[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.vel[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.vel[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.vel[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.vel[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.vel[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.acc[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.acc[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.acc[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.acc[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.glo.acc[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.acc[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.gamma = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.gamma)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.tau = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.tau)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.d_tau = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.d_tau)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.iod = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.iod)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.fcn = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.fcn)");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.pos[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.pos[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.pos[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.pos[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.pos[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.pos[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.vel[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.vel[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.vel[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.vel[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.vel[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.vel[2])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.acc[0] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.acc[0])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.acc[1] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.acc[1])");
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.glo.acc[2] = 1;
  fail_unless(!ephemeris_equal(&a, &b),
              "Ephemerides should not be equal (glo.acc[2])");
  memset(&a, 0, sizeof(a));
}
END_TEST

START_TEST(test_ephemeris_health) {
  const struct test_case {
    ephemeris_t e;
    bool healthy;
  } test_cases[] = {
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
             .health_bits = 0,
             .ura = 2.0,
             .valid = true},
       .healthy = true},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
             .health_bits = 0,
             .ura = 200.0,
             .valid = false},
       .healthy = true},
      {.e = {.sid = {.code = CODE_GPS_L2CM, .sat = 32},
             .health_bits = 0,
             .ura = 2000.0,
             .valid = true},
       .healthy = true},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
             .health_bits = 0,
             .ura = 33333.0,
             .valid = true},
       .healthy = false},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
             .health_bits = 0,
             .ura = INVALID_URA_VALUE,
             .valid = true},
       .healthy = false},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
             .health_bits = 0,
             .ura = -100.0,
             .valid = true},
       .healthy = false},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 11},
             .health_bits = 0x3F,
             .ura = 1.0,
             .valid = true},
       .healthy = false},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 12},
             .health_bits = 0x2A,
             .ura = 1.0,
             .valid = true},
       .healthy = false},
      {.e = {.sid = {.code = CODE_GPS_L2CM, .sat = 13},
             .health_bits = 0x2E,
             .ura = 4000.0,
             .valid = true},
       .healthy = false},
      {.e = {.sid = {.code = CODE_GPS_L2CM, .sat = 1},
             .health_bits = 0x2A,
             .ura = 4000.0,
             .valid = true},
       .healthy = true},
      {.e = {.sid = {.code = CODE_GPS_L1CA, .sat = 22},
             .health_bits = 0x2E,
             .ura = 10.0,
             .valid = true},
       .healthy = false},
  };

  for (u8 i = 0; i < (sizeof(test_cases) / sizeof(test_cases[0])); i++) {
    const struct test_case *t = &test_cases[i];
    fail_unless(t->healthy == ephemeris_healthy(&t->e, t->e.sid.code),
                "test signal %d healthy incorrect",
                i);
  }
}
END_TEST

START_TEST(test_6bit_health_word) {
  /*
   * u8 gps_healthy(u8 health_bits, code_t code)
   */

  const struct test_case {
    u8 health_bits;
    code_t code;
    u8 result;
  } test_cases[] = {
      {.health_bits = 0, .code = CODE_GPS_L1CA, .result = 1},
      {.health_bits = 0, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x2B, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x2B, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x0B, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x0B, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x0B, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x0B, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x2E, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x2E, .code = CODE_GPS_L2CM, .result = 0},
      {.health_bits = 0x0E, .code = CODE_GPS_L1CA, .result = 1},
      {.health_bits = 0x0E, .code = CODE_GPS_L2CM, .result = 0},
      {.health_bits = 0x04, .code = CODE_GPS_L1P, .result = 0},
      {.health_bits = 0x07, .code = CODE_GPS_L2P, .result = 0},
      {.health_bits = 0x20, .code = CODE_GPS_L1P, .result = 0},
      {.health_bits = 0x20, .code = CODE_GPS_L1P, .result = 0},
      {.health_bits = 0x01, .code = CODE_GPS_L2P, .result = 0},
      {.health_bits = 0, .code = CODE_GPS_L5I, .result = 1},
  };
  for (u32 i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
    const struct test_case *t = &test_cases[i];
    fail_unless(t->result == check_6bit_health_word(t->health_bits, t->code),
                "check_6bit_health_word(%d, %d) test failed (%d)",
                t->health_bits,
                t->code,
                t->result);
  }
}
END_TEST

// BNC_IODE was calculated using the code in
// https://github.com/swift-nav/PPP_Wizard14/blob/b05025517fa3f5ee4334171b97ab7475db319215/RTRover/rtrover_broadcast.cpp#L391
START_TEST(test_bds_iode) {
  ephemeris_t bds_eph = gps_eph;
  bds_eph.sid.code = CODE_BDS2_B1;
  u32 our_IODE = get_ephemeris_iod_or_iodcrc(&bds_eph);
  u32 BNC_IODE = 14700972;
  fail_unless(our_IODE == BNC_IODE, "test_bds_iode test failed");
}
END_TEST

START_TEST(test_gps_eph) {
  double d;
  double sat_pos[3];
  double sat_vel[3];
  double sat_acc[3];
  double clock_err;
  double clock_rate_err;
  u8 iode;
  u16 iodc;

  // TODO also test clocks
  gps_time_t t = {.wn = 1983, .tow = 2100};
  bool calc_eph_ok =
      (0 ==
       calc_sat_state_n(&gps_eph2, &t, sat_pos, sat_vel, sat_acc, &clock_err,
                        &clock_rate_err, &iodc, &iode));
  fail_unless(calc_eph_ok, "Failure computing sat state! \n");

  fail_unless(iode == 2);
  fail_unless(iodc == 2);

  double truth_pos[3] = {3166192.017, -21511945.818, -15899623.697};
  d = fabs(vector_distance(3, sat_pos, truth_pos));
  printf("result %f %f %f = %f\n", sat_pos[0], sat_pos[1], sat_pos[2], d);
  //fail_unless(d <= 0.001);


  double truth_vel[3] = {1533.973749, -1209.904136, 2000.871636};
  d = fabs(vector_distance(3, sat_vel, truth_vel));
  printf("result %f %f %f = %f\n", sat_vel[0], sat_vel[1], sat_vel[2], d);
  //fail_unless(d <= 0.001);


  double truth_acc[3] = {-0.224186, 0.100579, 0.324295};
  d = fabs(vector_distance(3, sat_acc, truth_acc));
  printf("result %f %f %f = %f\n", sat_acc[0], sat_acc[1], sat_acc[2], d);
  //fail_unless(d <= 0.001);
}
END_TEST

Suite *ephemeris_suite(void) {
  Suite *s = suite_create("Ephemeris");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_ephemeris_almanac_divergence);
  tcase_add_test(tc_core, test_ephemeris_equal);
  tcase_add_test(tc_core, test_ephemeris_health);
  tcase_add_test(tc_core, test_6bit_health_word);
  tcase_add_test(tc_core, test_bds_iode);
  tcase_add_test(tc_core, test_gps_eph);
  suite_add_tcase(s, tc_core);

  return s;
}
