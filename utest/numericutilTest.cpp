#include "stdafx.h"

#include "../libnumeric/numericutil.h"
#include "fmtutil.h"

namespace
{
	int get_cnt_exp(const int sx, const int ex, const int cyc_x)
	{
		int cnt = 0;
		for (int x = sx; x < ex; x += cyc_x) {
			cnt++;
		}
		return cnt;
	}

}	// namespace

TEST(numericutilTest, get_num_grid_points)
{
	int sx, ex, cyc_x, cntx, cntExp;

	sx = 0;
	ex = 100;
	cyc_x = 10;
	cntx = get_num_grid_points(ex - sx, cyc_x);
	cntExp = get_cnt_exp(sx, ex, cyc_x);
	EXPECT_EQ(cntExp, cntx);

	sx = 1;
	ex = 100;
	cyc_x = 10;
	cntx = get_num_grid_points(ex - sx, cyc_x);
	cntExp = get_cnt_exp(sx, ex, cyc_x);
	EXPECT_EQ(cntExp, cntx);

	sx = 0;
	ex = 9;
	cyc_x = 10;
	cntx = get_num_grid_points(ex - sx, cyc_x);
	cntExp = get_cnt_exp(sx, ex, cyc_x);
	EXPECT_EQ(cntExp, cntx);
}

// In the following, "B'0.1{n}"  means       sum of 2^(-k  ) on (k=1, 2, ..., n).
// As above,         "B'1.1{n}"  means 1.0 + sum of 2^(-k  ) on (k=1, 2, ..., n),
//                   "B'1.01{n}" means 1.0 + sum of 2^(-k-1) on (k=1, 2, ..., n).
// Exanple:
//   B'0.1{2} = B'0.11   = 0.75
//   B'1.1{2} = B'1.11   = 1.75
//   B'1.1{4} = B'1.1111 = 1.75 + 2^(-3) + 2^(-4) = 1.9375
//   B'0.1{4} = B'0.1111 = 0.9375

TEST(numericutilTest, can_equal_basic_behavior) {
	double x, y;

	const auto digits = std::numeric_limits<double>::digits;

	// 0.0 = 0.0
	x = 0.0;
	y = 0.0;
	EXPECT_TRUE(can_equal(x, y));
	EXPECT_TRUE(can_equal(0.0, x - y));

	// 1.0 = 1.0
	x = 1.0;
	y = 1.0;
	EXPECT_TRUE(can_equal(x, y));
	EXPECT_TRUE(can_equal(0.0, x - y));

	// 1.1{digits - 1} == 1.1{digits - 1}
	x = gen_consecutive_1s(digits, 0);
	y = gen_consecutive_1s(digits, 0);
	EXPECT_TRUE(can_equal(x, y));
	EXPECT_TRUE(can_equal(0.0, x - y));
}

TEST(numericutilTest, can_equal_around_1) {
	double x, y;

	// 0.0 = 0.0
	x = 0.0;
	y = 0.0;
	EXPECT_TRUE(can_equal(x, y));

	// 1.0 = 1.0
	x = 1.0;
	y = 1.0;
	EXPECT_TRUE(can_equal(x, y));

	const auto min_expn = std::numeric_limits<double>::min_exponent;
	const auto digits = std::numeric_limits<double>::digits;
	const auto epsilon = std::numeric_limits<double>::epsilon();

	// B'1.01{digits - 2} != B'1.01{digits - 2} + 2^(-digits + 2)
	x = 1.0 + gen_consecutive_1s(digits, -2);
	y = x + std::ldexp(1.0, -digits + 2);		// > epsilon
	EXPECT_FALSE(can_equal(x, y));
	EXPECT_FALSE(can_equal(epsilon, y - x));

	// B'1.01{digits - 2} != B'1.01{digits - 2} + 2^(-digits + 1)
	x = 1.0 + gen_consecutive_1s(digits, -2);
	y = x + std::ldexp(1.0, -digits + 1);		// = epsilon
	EXPECT_FALSE(can_equal(x, y));
	EXPECT_TRUE(can_equal(epsilon, y - x));

	// B'1.01{digits - 2} == B'1.01{digits - 2} + 2^(-digits + 0)
	x = 1.0 + gen_consecutive_1s(digits, -2);
	y = x + std::ldexp(1.0, -digits + 0);		// < epsilon
	EXPECT_TRUE(can_equal(x, y));
	EXPECT_FALSE(can_equal(epsilon, y - x));
}

TEST(numericutilTest, can_equal_around_0_a) {
	double x, y;

	const auto min_expn = std::numeric_limits<double>::min_exponent;
	const auto digits = std::numeric_limits<double>::digits;

	// x != x + 0.5^(min_expn - digits + 3)
	// where x = B'1.1{digits - 1} * 2^(min_expn - 1)		// 2^(min_expn - 1) = 0.5^(min_expn)
	x = gen_consecutive_1s(digits, min_expn - 1);
	ASSERT_NE(x, (x / 2.0) * 2.0);
	y = x + std::ldexp(0.5, min_expn - digits + 3);
	EXPECT_FALSE(can_equal(x, y));

	// x == x + 0.5^(min_expn - digits + 2)  (match by carry and rounding)
	// where x = B'1.1{digits - 1} * 2^(min_expn - 1)		// 2^(min_expn - 1) = 0.5^(min_expn)
	x = gen_consecutive_1s(digits, min_expn - 1);
	ASSERT_NE(x, (x / 2.0) * 2.0);
	y = x + std::ldexp(0.5, min_expn - digits + 2);
	EXPECT_TRUE(can_equal(x, y));

	// x == x + 0.5^(min_expn - digits + 1)
	// where x = B'1.1{digits - 1} * 2^(min_expn - 1)		// 2^(min_expn - 1) = 0.5^(min_expn)
	x = gen_consecutive_1s(digits, min_expn - 1);
	ASSERT_NE(x, (x / 2.0) * 2.0);
	y = x + std::ldexp(0.5, min_expn - digits + 1);
	EXPECT_TRUE(can_equal(x, y));
}

TEST(numericutilTest, can_equal_around_0_b) {
	double x, y;

	const auto min_val = std::numeric_limits<double>::min();
	const auto min_expn = std::numeric_limits<double>::min_exponent;
	const auto digits = std::numeric_limits<double>::digits;

	// min_val is minimum value of normal number.
	ASSERT_EQ(std::ldexp(0.5, min_expn), min_val);

	// Minimum value of subnormal number is 0.5^(min_expn - digits + 1).
	ASSERT_NE(0.0, std::ldexp(0.5, min_expn - digits + 1));
	ASSERT_EQ(0.0, std::ldexp(0.5, min_expn - digits + 1) / 2.0);

	// x != x + 0.5^(min_expn - digits + 2)
	// where x = 2^(min_expn - 1)							// 2^(min_expn - 1) = 0.5^(min_expn)
	x = std::ldexp(0.5, min_expn);
	y = x + std::ldexp(0.5, min_expn - digits + 2);
	EXPECT_FALSE(can_equal(x, y));

	// x == x + 0.5^(min_expn - digits + 1)
	// where x = 2^(min_expn - 1)							// 2^(min_expn - 1) = 0.5^(min_expn)
	x = std::ldexp(0.5, min_expn);
	y = x + std::ldexp(0.5, min_expn - digits + 1);
	EXPECT_TRUE(can_equal(x, y));
}

TEST(numericutilTest, can_equal_around_0_subnormal) {
	double x, y;

	const auto min_val = std::numeric_limits<double>::min();
	const auto min_expn = std::numeric_limits<double>::min_exponent;
	const auto digits = std::numeric_limits<double>::digits;

	// min_val is minimum value of normal number.
	ASSERT_EQ(std::ldexp(0.5, min_expn), min_val);

	// Minimum value of subnormal number is 0.5^(min_expn - digits + 1).
	ASSERT_NE(0.0, std::ldexp(0.5, min_expn - digits + 1));
	ASSERT_EQ(0.0, std::ldexp(0.5, min_expn - digits + 1) / 2.0);

	// x != x + 0.5^(min_expn - digits + 2)
	// where x = 2^(min_expn - digits + 4)
	x = std::ldexp(1.0, min_expn - digits + 4);
	ASSERT_LT(x, min_val);
	y = x + std::ldexp(0.5, min_expn - digits + 2);
	EXPECT_FALSE(can_equal(x, y));

	// x == x + 0.5^(min_expn - digits + 1)
	// where x = 2^(min_expn - digits + 4)
	x = std::ldexp(1.0, min_expn - digits + 4);
	ASSERT_LT(x, min_val);
	y = x + std::ldexp(0.5, min_expn - digits + 1);
	EXPECT_TRUE(can_equal(x, y));
}
