#include "stdafx.h"

#include "../libnumeric/numericutil.h"
#include "fmtutil.h"

namespace
{
	void chk_strict_equal(const double x, const double frac_exp, const int expn_exp)
	{
		int exponent;
		const double frac = std::frexp(x, &exponent);
		EXPECT_EQ(frac_exp, frac);
		EXPECT_EQ(expn_exp, exponent);
	}

}	// namespace


TEST(numericTest, frexp) {
	double x;

	SCOPED_TRACE(__FUNCTION__);

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, true, 20);

	// 0.0 = 0.0*2^0
	x = 0.0;
	chk_strict_equal(x, 0.0, 0);

	// 0.5 = 0.5*2^0
	x = 0.5;
	chk_strict_equal(x, 0.5, 0);

	// 1.0 = 0.5*2^1
	x = 1.0;
	chk_strict_equal(x, 0.5, 1);

	// 2.0 = 0.5*2^2
	x = 2.0;
	chk_strict_equal(x, 0.5, 2);

	restore_format(sv_spec, cout);
}

TEST(numericTest, epsilon) {
	double x, y, val, err;

	SCOPED_TRACE(__FUNCTION__);

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, true, 20);

	const auto epsilon = std::numeric_limits<double>::epsilon();
	const auto digits = std::numeric_limits<double>::digits;

	// Case where error = epsilon
	x = 2.0;
	y = 0.002;
	val = (x + y) - y;
	err = std::abs(val - x);
	cout << "      x = " << x << endl;
	cout << "      y = " << y << endl;
	cout << "(x+y)-x = " << val << endl;
	cout << "|x-val| = " << err << endl;
	EXPECT_EQ(epsilon, err);

	// epsilon = 2^(-digits + 1)
	x = epsilon;
	y = std::ldexp(1.0, -digits + 1);
	EXPECT_EQ(x, y);

	// epsilon = 0.5^(-digits + 2)
	x = epsilon;
	y = std::ldexp(0.5, -digits + 2);
	EXPECT_EQ(x, y);

	restore_format(sv_spec, cout);
}

TEST(numericTest, ldexp_basic_behavior) {
	double x, y;

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, false, 20);

	// 0.0^0 = 0.0 (at std::ldexp())
	x = std::ldexp(0.0, 0);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));
	EXPECT_EQ(0.0, x);

	// 1.0^0 = 1.0
	x = std::ldexp(1.0, 0);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));
	EXPECT_EQ(1.0, x);

	// ldexp(1.0, 0) = ldexp(0.5, -1)
	x = ldexp(1.0, -1);
	y = ldexp(0.5, 0);
	EXPECT_EQ(x, y);
	EXPECT_EQ(0.5, x);

	restore_format(sv_spec, cout);
}

// In the following, "B'0.1{n}"  means       sum of 2^(-k  ) on (k=1, 2, ..., n).
// As above,         "B'1.1{n}"  means 1.0 + sum of 2^(-k  ) on (k=1, 2, ..., n),
//                   "B'1.01{n}" means 1.0 + sum of 2^(-k-1) on (k=1, 2, ..., n).
// Exanple:
//   B'0.1{2} = B'0.11   = 0.75
//   B'1.1{2} = B'1.11   = 1.75
//   B'1.1{4} = B'1.1111 = 1.75 + 2^(-3) + 2^(-4) = 1.9375
//   B'0.1{4} = B'0.1111 = 0.9375

TEST(numericTest, tool_check) {
	double x, y;

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, false, 20);

	const auto epsilon = std::numeric_limits<double>::epsilon();
	const auto digits = std::numeric_limits<double>::digits;

	// gen_consecutive_1s() test
	// gen_consecutive_1s(2, -1) = B'0.1{2}
	x = gen_consecutive_1s(2, -1);
	ASSERT_EQ(0.75, x);
	// gen_consecutive_1s(4, -1) = B'0.1{4}
	x = gen_consecutive_1s(4, -1);
	ASSERT_EQ(0.9375, x);
	// gen_consecutive_1s(3, 0) = 1.0 + gen_consecutive_1s(2, -1) = B'1.1{2}
	x = gen_consecutive_1s(3, 0);
	y = 1.0 + gen_consecutive_1s(2, -1);
	ASSERT_EQ(1.75, x);
	ASSERT_EQ(x, y);
	// gen_consecutive_1s(5, 0) = 1.0 + gen_consecutive_1s(4, -1) = B'1.1{4}
	x = gen_consecutive_1s(5, 0);
	y = 1.0 + gen_consecutive_1s(4, -1);
	ASSERT_EQ(1.9375, x);
	ASSERT_EQ(x, y);

	// ldexp() test
	// ldexp(1.0, -digits + 1) = 2^(-digits + 1) = epsilon
	ASSERT_EQ(epsilon, ldexp(1.0, -digits + 1));
	// ldexp(0.5, -digits) = 0.5^(-digits + 2) = epsilon
	ASSERT_EQ(epsilon, ldexp(0.5, -digits + 2));

	restore_format(sv_spec, cout);
}

namespace
{
	// Return a string like "B'0.1{4}".
	std::string val_str(const char* const frac, const int n)
	{
		std::ostringstream os;
		os << "B'" << frac << "{" << std::noshowpos << n << "}";
		return os.str();
	}

	// Return a string like "2^(-53)".
	std::string iv2_str(const int n)
	{
		std::ostringstream os;
		os << "2^(" << std::noshowpos << n << ")";
		return os.str();
	}

	void print_result(const char* const frac, const int digits, const int expn_to_add, const double x, const double y)
	{
		const std::string x_str = val_str(frac, digits);
		const std::string add_str = iv2_str(expn_to_add);
		cout << x_str << setw(10) << " " << ") = " << std::showpos << x << endl;
		cout << x_str << " + " << add_str << ") = " << std::showpos << y << endl;
	}

}	// namespace

TEST(numericTest, ldexp_around_1) {
	double x, y;
	int expn_to_add;

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, true, 20);

	const auto epsilon = std::numeric_limits<double>::epsilon();
	const auto digits = std::numeric_limits<double>::digits;

	// (B'0.1{digits}) + 2^(-digits + 2) > 1.0
	expn_to_add = -digits + 2;
	x = gen_consecutive_1s(digits, -1);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("0.1", digits, expn_to_add, x, y);
	EXPECT_NE(1.0, x);
	EXPECT_LT(x, 1.0);
	EXPECT_GT(y, 1.0);

	// (B'0.1{digits}) + 2^(-digits + 1) = 1.0 (match by carry and rounding (1))
	expn_to_add = -digits + 1;
	x = gen_consecutive_1s(digits, -1);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("0.1", digits, expn_to_add, x, y);
	EXPECT_NE(1.0, x);
	EXPECT_LT(x, 1.0);
	EXPECT_EQ(1.0, y);

	// (B'0.1{digits}) + 2^(-digits + 0) = 1.0 (match by carry and rounding (2))
	expn_to_add = -digits + 0;
	x = gen_consecutive_1s(digits, -1);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("0.1", digits, expn_to_add, x, y);
	EXPECT_NE(1.0, x);
	EXPECT_LT(x, 1.0);
	EXPECT_EQ(1.0, y);

	// (B'0.1{digits}) + 2^(-digits - 1) = 1.0 (match by carry and rounding (3))
	expn_to_add = -digits - 1;
	x = gen_consecutive_1s(digits, -1);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("0.1", digits, expn_to_add, x, y);
	EXPECT_NE(1.0, x);
	EXPECT_LT(x, 1.0);
	EXPECT_EQ(1.0, y);

	// (B'0.1{digits}) + 2^(-digits - 2) < 1.0
	expn_to_add = -digits - 2;
	x = gen_consecutive_1s(digits, -1);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("0.1", digits, expn_to_add, x, y);
	EXPECT_NE(1.0, x);
	EXPECT_LT(x, 1.0);
	EXPECT_LT(y, 1.0);

	restore_format(sv_spec, cout);
}

TEST(numericTest, ldexp_around_1_5) {
	double x, y;
	int expn_to_add;

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, true, 20);

	const auto epsilon = std::numeric_limits<double>::epsilon();
	const auto digits = std::numeric_limits<double>::digits;

	// (B'1.01{digits - 2}) + 2^(-digits + 2) > 1.5
	expn_to_add = -digits + 2;
	x = 1.0 + gen_consecutive_1s(digits - 2, -2);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("1.01", digits - 2, expn_to_add, x, y);
	EXPECT_NE(1.5, x);
	EXPECT_LT(x, 1.5);
	EXPECT_GT(y, 1.5);

	// (B'1.01{digits - 2}) + 2^(-digits + 1) = 1.5
	expn_to_add = -digits + 1;
	x = 1.0 + gen_consecutive_1s(digits - 2, -2);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("1.01", digits - 2, expn_to_add, x, y);
	EXPECT_NE(1.5, x);
	EXPECT_LT(x, 1.5);
	EXPECT_EQ(1.5, y);

	// (B'1.01{digits - 2}) + 2^(-digits) = 1.5 (match by rounding)
	expn_to_add = -digits;
	x = 1.0 + gen_consecutive_1s(digits - 2, -2);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("1.01", digits - 2, expn_to_add, x, y);
	EXPECT_NE(1.5, x);
	EXPECT_LT(x, 1.5);
	EXPECT_EQ(1.5, y);

	// (B'1.01{digits - 2}) + 2^(-digits - 1) < 1.5
	expn_to_add = -digits - 1;
	x = 1.0 + gen_consecutive_1s(digits - 2, -2);
	y = x + std::ldexp(1.0, expn_to_add);
	print_result("1.01", digits - 2, expn_to_add, x, y);
	EXPECT_NE(1.5, x);
	EXPECT_LT(x, 1.5);
	EXPECT_LT(y, 1.5);

	restore_format(sv_spec, cout);
}

TEST(numericTest, ldexp_around_INF) {
	double x, y;
	int expn_to_add;

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, false, 20);

	const auto max_expn = std::numeric_limits<double>::max_exponent;
	const auto digits = std::numeric_limits<double>::digits;

	// 0.5^(max_expn) != INF
	x = std::ldexp(0.5, max_expn);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));

	// 0.5^(max_expn + 1) = INF
	x = std::ldexp(0.5, max_expn + 1);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_TRUE(std::isinf(x));

	// (B'0.1{digits})^(max_expn) != INF
	x = gen_consecutive_1s(digits, max_expn - 1);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));

	// (B'0.1{digits})^(max_expn) + 0.5^(max_expn - digits) = INF
	expn_to_add = max_expn - digits;
	x = gen_consecutive_1s(digits, max_expn - 1);
	y = x + std::ldexp(0.5, expn_to_add);
	EXPECT_FALSE(std::isnan(y));
	EXPECT_TRUE(std::isinf(y));

	// (B'0.1{digits})^(max_expn) + 0.5^(max_expn - digits 0 1) != INF
	expn_to_add = max_expn - digits - 1;
	x = gen_consecutive_1s(digits, max_expn - 1);
	y = x + std::ldexp(0.5, expn_to_add);
	EXPECT_FALSE(std::isnan(y));
	EXPECT_FALSE(std::isinf(y));

	restore_format(sv_spec, cout);
}

TEST(numericTest, ldexp_around_0) {
	double x;

	FormatSpec sv_spec;
	save_and_change_real_num_format(cout, sv_spec, false, 20);

	const auto min_expn = std::numeric_limits<double>::min_exponent;
	const auto digits = std::numeric_limits<double>::digits;

	// 0.5^(min_expn - digits + 1) > 0.0
	x = std::ldexp(0.5, min_expn - digits + 1);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));
	EXPECT_GT(x, 0.0);

	// 0.5^(min_expn - digits + 1) / 2.0 = 0.0
	x = std::ldexp(0.5, min_expn - digits + 1);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));
	EXPECT_EQ(0.0, x / 2.0);

	// 0.5^(min_expn - digits) > 0.0 (Why?)
	x = std::ldexp(0.5, min_expn - digits);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));
	EXPECT_GT(x, 0.0);

	// 0.5^(min_expn - digits - 1) = 0.0
	x = std::ldexp(0.5, min_expn - digits - 1);
	EXPECT_FALSE(std::isnan(x));
	EXPECT_FALSE(std::isinf(x));
	EXPECT_EQ(0.0, x);

	restore_format(sv_spec, cout);
}
