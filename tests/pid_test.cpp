#include "CppUTest/TestHarness.h"

extern "C" {
#include "../pid.h"
}

TEST_GROUP(PIDTestGroup)
{
    pid_filter_t pid;

    /** Processes a given input in the PID filter and compares it against
     * expected output. */
    void process_and_expect(float input, float expected_output)
    {
        float output;
        output = pid_process(&pid, input);
        DOUBLES_EQUAL(expected_output, output, 1e-3);
    }

    void setup(void)
    {
        pid_init(&pid);
    }
};

TEST(PIDTestGroup, IntegralValueIsZeroAtInit)
{
    CHECK_EQUAL(0, pid_get_integral(&pid));
}

TEST(PIDTestGroup, PControllerAtInit)
{
    CHECK_EQUAL(1., pid_get_kp(&pid));
    CHECK_EQUAL(0., pid_get_ki(&pid));
    CHECK_EQUAL(0., pid_get_kd(&pid));
}

TEST(PIDTestGroup, CanSetGains)
{
    pid_set_gains(&pid, 10., 20., 30.);
    CHECK_EQUAL(10., pid_get_kp(&pid));
    CHECK_EQUAL(20., pid_get_ki(&pid));
    CHECK_EQUAL(30., pid_get_kd(&pid));
}

TEST(PIDTestGroup, ZeroErrorMakesForZeroOutput)
{
    process_and_expect(0., 0.);
}

TEST(PIDTestGroup, ByDefaultIsIdentity)
{
    process_and_expect(42., 42.);
}

TEST(PIDTestGroup, KpHasInfluenceOnOutput)
{
    pid_set_gains(&pid, 2., 0., 0.);
    process_and_expect(21., 42.);
}

TEST(PIDTestGroup, IntegratorIsChangedByProcess)
{
    pid_process(&pid, 42.);
    CHECK_EQUAL(42., pid_get_integral(&pid));

    pid_process(&pid, 42.);
    CHECK_EQUAL(84., pid_get_integral(&pid));
}

TEST(PIDTestGroup, AddingKiMakesOutputIncrease)
{
    pid_set_gains(&pid, 0., 2., 0.);
    process_and_expect(42., 84.);
    process_and_expect(42., 168.);
}

TEST(PIDTestGroup, IntegratorWorksInNegativeToo)
{
    pid_set_gains(&pid, 0., 1., 0.);
    process_and_expect(-42., -42.);
    process_and_expect(-42., -84.);
}

TEST(PIDTestGroup, AddingKdCreatesDerivativeAction)
{
    pid_set_gains(&pid, 0., 0., 2.);
    process_and_expect(42., 84.);
    process_and_expect(42., 0);
}

TEST(PIDTestGroup, IntegratorMaxValueIsRespected)
{
    pid_set_integral_limit(&pid, 20.);
    pid_set_gains(&pid, 0., 1., 0.);
    process_and_expect(20., 20.);
    process_and_expect(20., 20.);
}

TEST(PIDTestGroup, IntegratorMaxValueWorksInNegativeToo)
{
    pid_set_integral_limit(&pid, 20.);
    pid_set_gains(&pid, 0., 1., 0.);

    process_and_expect(-20., -20.);
    process_and_expect(-20., -20.);
}

TEST(PIDTestGroup, CanResetIntegrator)
{
    pid_set_gains(&pid, 0., 1., 0.);
    process_and_expect(20., 20.);
    process_and_expect(20., 40.);

    pid_reset_integral(&pid);
    process_and_expect(20., 20.);
}

TEST(PIDTestGroup, FrequencyChangeIntegrator)
{
    pid_set_frequency(&pid, 10.);
    pid_set_gains(&pid, 0., 1., 0.);
    process_and_expect(20., 2.);
    process_and_expect(20., 4.);
}

TEST(PIDTestGroup, FrequencyChangeDerivative)
{
    pid_set_frequency(&pid, 10.);
    pid_set_gains(&pid, 0., 0., 1.);
    process_and_expect(20., 200.);
    process_and_expect(20., 0.);
}

