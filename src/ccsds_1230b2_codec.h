#ifndef __CCSDS_1230B2__
#define __CCSDS_1230B2__

#include <stdbool.h>
#include "bit_output_stream.h"
#include "bit_input_stream.h"
#include "utilities.h"
#include "checker.h"
#include "tree_table.h"


#define D3(data, i, j, k, cp) data[(i)*cp->samples_per_band + (j)*cp->samples + (k)]
#define W2(data, i, j, cp) data[(i)*cp->weights_per_band+(j)]

////
//CONSTANTS
////


/////IMAGE PARAMETERS/////
#define MAX_DEPTH 32
#define DEFAULT_DEPTH 16
#define MIN_DEPTH 2
//////////////////////////


////////LOSSY PARAMETERS////////
#define MIN_ERROR_LIMIT_BIT_DEPTH 1
#define DEFAULT_ABSOLUTE_ERROR_LIMIT_BIT_DEPTH 14
#define DEFAULT_RELATIVE_ERROR_LIMIT_BIT_DEPTH 14
#define MAX_ERROR_LIMIT_BIT_DEPTH 16

#define DEFAULT_USE_ABS_ERR false
#define DEFAULT_USE_REL_ERR false

#define MIN_ABS_ERR_VALUE 0
#define DEFAULT_ABS_ERR_VALUE 0
#define get_MAX_ABS_ERR_VALUE(aelbd) ((1 << aelbd) - 1)

#define MIN_REL_ERR_VALUE 0
#define DEFAULT_REL_ERR_VALUE 0
#define get_MAX_REL_ERR_VALUE(relbd) ((1 << relbd) - 1)
/////////////////////////////


////////PREDICTOR FINE TUNING////////// (disable for fast lossless pipelining potential)
#define MIN_RESOLUTION 0
#define DEFAULT_RESOLUTION_VALUE 4
#define MAX_RESOLUTION 4

#define MIN_DAMPING 0
#define DEFAULT_DAMPING_VALUE 4
#define get_MAX_DAMPING(resolution) ((1 << resolution) - 1)

#define MIN_OFFSET 0
#define DEFAULT_OFFSET_VALUE 4
#define get_MAX_OFFSET(resolution) ((1 << resolution) - 1)
///////////////////////////////

///////COMPRESSION PARAMETERS////////////
enum LocalSumType {
	WIDE_NEIGHBOR_ORIENTED,
	NARROW_NEIGHBOR_ORIENTED,
	WIDE_COLUMN_ORIENTED,
	NARROW_COLUMN_ORIENTED
};

#define DEFAULT_FULL_PREDICTION_ENABLED true
#define DEFAULT_LOCAL_SUM_TYPE WIDE_NEIGHBOR_ORIENTED

#define MIN_OMEGA 4
#define DEFAULT_OMEGA 19
#define MAX_OMEGA 19

#define MIN_V -6
#define DEFAULT_V_MIN -1
#define DEFAULT_V_MAX 3
#define MAX_V 9

#define MIN_T_EXP 4
#define DEFAULT_T_EXP 6
#define MAX_T_EXP 11

#define MIN_P 0
#define DEFAULT_P 3
#define MAX_P 15

#define MIN_ACC_INIT_CONSTANT 0
#define DEFAULT_ACC_INIT_CONSTANT 5
#define get_MAX_ACC_INIT_CONSTANT(depth) min(depth - 2, 14)

#define MIN_WEIGHT_EXPONENT_OFFSET -6
#define DEFAULT_WEIGHT_EXPONENT_OFFSET 0
#define MAX_WEIGHT_EXPONENT_OFFSET 5

#define get_MIN_R(depth, omega) max(32, depth + omega + 2)
#define DEFAULT_R 64
#define MAX_R 64
////////////////////////////////////////

//////////ENCODER PARAMETERS///////////
#define MIN_U_MAX 8
#define DEFAULT_U_MAX 18
#define MAX_U_MAX 32

#define MIN_GAMMA_ZERO 1
#define DEFAULT_GAMMA_ZERO 1
#define MAX_GAMMA_ZERO 8

#define MIN_GAMMA_STAR 4
#define DEFAULT_GAMMA_STAR 6
#define MAX_GAMMA_STAR 11
///////////////////////////////////////


////
//COMPRESSION PARAMETERS
////

inline long get_s_mid(long depth) 	{ return 1l << (depth - 1); }
inline long get_s_min() 			{ return 0;}
inline long get_s_max(long depth) 	{ return (1l << depth) - 1; }
inline long get_w_min(long omega) 	{ return -(1 << (omega + 2)); }		//EQ 30
inline long get_w_max(long omega) 	{ return (1 << (omega + 2)) - 1; }	//EQ 30


typedef struct {
	enum LocalSumType local_sum_type;
	long depth;
	bool full_prediction_mode;
	long prediction_bands;
	long omega;
	long r;
	
	//simplified to a single value
	long abs_err;
	long rel_err;

	bool use_abs_err_limit;
	bool use_rel_err_limit; 
	long abs_err_limit_bit_depth;
	long rel_err_limit_bit_depth;
	
	//simplified to a single value
	long resolution;
	long damping;
	long offset;
	
	long t_inc_exp;
	long vmin;
	long vmax;

	//simplified to a single value
	long intra_band_weight_exponent_offset;
	long inter_band_weight_exponent_offset; 
	
	long u_max;
	long gamma_zero;
	long gamma_star;

	//simplified to a single value
	long accumulator_initialization_constant;

	//dimensions
	long bands;
	long samples;
	long lines;

	//Non base values that are calculated on the fly with setters
	long weights_per_band;
	long samples_per_image;
	long samples_per_band;

	long s_mid;
	long s_min;
	long s_max;
	long w_min;
	long w_max;

	//Encoder parameters
	/* Need one per band */
	long * accumulator;
	long counter_reset_value;
	TreeTable * active_tables[16];

	//statistics
	long stats_golombrem;
	long stats_golombunary;
	long stats_golombrem_max;
	long stats_golombunary_max;
	long stats_mqi;
	long stats_accbit;
	long stats_tablecwbits;
	long stats_tableflush;
	long stats_accflush;
	long stats_endbit;
} CompressionParameters;


void reset_stats(CompressionParameters * cp);
void recalc_encoder_params(CompressionParameters * cp);
void reset_tables(CompressionParameters * cp);
long set_dimensions(CompressionParameters * cp, long bands, long lines, long samples);
long set_defaults(CompressionParameters * cp);
void recalc_weight_vector_length(CompressionParameters * cp);

long set_full_prediction_mode(bool enable, CompressionParameters * cp);
long set_prediction_bands(long prediction_bands, CompressionParameters * cp);
long set_accumulator_initialization_constant(long accumulator_initialization_constant, CompressionParameters * cp);
long set_encoder_update_params(long u_max, long gamma_zero, long gamma_star, CompressionParameters * cp);
long set_weight_exponent_offset(long weight_exponent_offset, CompressionParameters * cp);
long set_weight_update_params(long t_inc_exp, long vmin, long vmax, CompressionParameters * cp);
long set_near_lossless_params(long resolution, long damping, long offset, CompressionParameters * cp);
long set_omega(long omega, CompressionParameters * cp);
long set_depth(long depth, CompressionParameters * cp);	
long set_r(long r, CompressionParameters * cp);
void set_local_sum_type(enum LocalSumType local_sum_type, CompressionParameters * cp);
long set_errors(long abs_err_limit_bit_depth, long rel_err_limit_bit_depth, long abs_err, long rel_err, bool use_abs_err_limit, bool use_rel_err_limit, CompressionParameters * cp);


void compress(long * block, CompressionParameters * cp, BitOutputStream * bos, Checker * checker_predictor, Checker * checker_encoder);
long * decompress(BitInputStream * bis, CompressionParameters * cp, Checker * checker_predictor, Checker * checker_encoder);


long calc_local_sum(long b, long l, long s, long * rep_block, long samples, CompressionParameters * cp);
long calc_central_local_diff(long b, long l, long s, long * rep_block, long local_sum, CompressionParameters * cp);
long calc_north_diff (long b, long l, long s, long * rep_block, long local_sum, CompressionParameters * cp);
long calc_west_diff (long b, long l, long s, long * rep_block, long local_sum, CompressionParameters * cp);
long calc_north_west_diff (long b, long l, long s, long * rep_block, long local_sum, CompressionParameters * cp);
long * get_initial_weights(CompressionParameters * cp);
long calc_predicted_central_diff (long b, long l, long s, long * weights, long north_diff, long west_diff, long north_west_diff, long * diff_block, CompressionParameters * cp);
long calc_high_resolution_predicted_sample_value(long predicted_central_diff, long local_sum, CompressionParameters * cp);
long calc_double_resolution_sample_value(long b, long l, long s, long high_resolution_predicted_sample_value, long * block, CompressionParameters * cp);
long calc_predicted_sample_value(long double_resolution_predicted_sample_value);
long calc_prediction_residual(long sample, long predicted_sample_value);
long calc_quantizer_index(long predition_residual, long maximum_error_value, long t);
long calc_max_err_val(long predicted_sample_value, long t, CompressionParameters * cp);
long calc_sample_representative(long l, long s, long double_resolution_sample_representative, long sample);
long calc_double_resolution_sample_representative(long clipped_quantizer_bin_center, long quantizer_index, long maximum_error_value, long high_resolution_predicted_sample_value, CompressionParameters * cp);
long calc_clipped_quantizer_bin_center(long predicted_sample_value, long quantizer_index, long maximum_error_value, CompressionParameters * cp);
long calc_double_resolution_prediction_error(long clipped_quantizer_bin_center, long double_resolution_predicted_sample_value);
long calc_weight_update_scaling_exponent(long t, long samples, CompressionParameters * cp);
long update_weight(long weight, long double_resolution_prediction_error, long diff, long weight_update_scaling_exponent, long weight_exponent_offset, CompressionParameters * cp);
long calc_mapped_quantizer_index(long quantizer_index, long theta, long double_resolution_predicted_sample_value);
long get_lower_theta(long t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long get_upper_theta(long t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long calc_theta(long t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long decalc_quantizer_index(long mapped_quantizer_index, long theta, long double_resolution_predicted_sample_value, long t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long decalc_sample(long prediction_residual, long predicted_sample_value);
long decalc_prediction_residual(long t, long quantizer_index, long maximum_error_value);

////
//ENTROPY CODING
////
	
long get_counter_value(long t, CompressionParameters * cp);
void length_limited_golomb_power_of_two_code(long u_int_val, long u_int_code_index,  BitOutputStream * bos, CompressionParameters * cp);
long length_limited_golomb_power_of_two_decode(long u_int_code_index, BitInputStream * bis, CompressionParameters * cp);
long get_u_int_code_index(long b, long c_value, CompressionParameters * cp);
void update_accumulator(long b, long mapped_quantizer_index, long c_value, CompressionParameters * cp);
void code(long mapped_quantizer_index, long t, long b, BitOutputStream * bos, CompressionParameters * cp, Checker * checker);
long decode(long t, long b, BitInputStream * bis, CompressionParameters * cp, Checker * checker);

long get_k(long counter, long accumulator, CompressionParameters * cp);
long get_code_index(long acc, long counter);
void code_hybrid(long mapped_quantizer_index, long t, long b, BitOutputStream * bos, CompressionParameters * cp, Checker * checker);
long decode_hybrid(long t, long b, BitInputStream * bis, CompressionParameters * cp, Checker * checker, long ** decoded_mqi);
long * fully_decode_hybrid(BitInputStream * bis, CompressionParameters * cp, Checker * checker);
void reverse_update_accumulator(long b, long mqi, long counter, CompressionParameters * cp);

////
//QUANTIZATION
////

long quantize(long value, long parameter);
long dequantize(long qvalue, long parameter);

////
//UTILITIES
////

long max(long a, long b);
long min(long a, long b);
long absl(long a);
long clamp(long value, long min, long max);
long clampi(long value, long min, long max);
long signum(long val);
long signum_plus(long val);
long mod_R(long value, long r);
long minus_one_to_the(long value);


#endif //__CCSDS_1230B2__