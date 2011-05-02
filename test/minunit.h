/*
 * Very simple test "framework" based on http://www.jera.com/techinfo/jtns/jtn002.html
 */

#define mu_assert(message, test) do { if (!(test)) throw strdup(message); } while (0)
#define mu_run_test(test, testname) do { tests_run++; try { test(); printf("Test PASSED: %s\n", testname); } catch (const char* ex) { tests_failed++; printf("Test FAILED: %s\n\t%s\n", testname, ex); } } while (0)
extern int tests_run;
extern int tests_failed;
