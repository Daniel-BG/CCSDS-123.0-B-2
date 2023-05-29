#ifndef __CCSDS_1230B2__
#define __CCSDS_1230B2__

#include <stdbool.h>
#include "bit_output_stream.h"
#include "bit_input_stream.h"
#include "utilities.h"


#define D3(data, i, j, k, cp) data[i*cp->samples_per_band + j*cp->samples + k]
#define W2(data, i, j, cp) data[i*cp->weights_per_band+j]

////
//CONSTANTS
////


/////IMAGE PARAMETERS/////
const int MAX_DEPTH = 32;
const int DEFAULT_DEPTH = 16;
const int MIN_DEPTH = 2;
//////////////////////////


////////LOSSY PARAMETERS////////
const int MIN_ERROR_LIMIT_BIT_DEPTH = 1;
const int DEFAULT_ABSOLUTE_ERROR_LIMIT_BIT_DEPTH = 14;
const int DEFAULT_RELATIVE_ERROR_LIMIT_BIT_DEPTH = 14;
const int MAX_ERROR_LIMIT_BIT_DEPTH = 16;

const bool DEFAULT_USE_ABS_ERR = true;
const bool DEFAULT_USE_REL_ERR = true;

const int MIN_ABS_ERR_VALUE = 0;
const int DEFAULT_ABS_ERR_VALUE = 0;
#define get_MAX_ABS_ERR_VALUE(aelbd) ((1 << aelbd) - 1)

const int MIN_REL_ERR_VALUE = 0;
const int DEFAULT_REL_ERR_VALUE = 0;
#define get_MAX_REL_ERR_VALUE(relbd) ((1 << relbd) - 1)
/////////////////////////////


////////PREDICTOR FINE TUNING////////// (disable for fast lossless pipelining potential)
const int MIN_RESOLUTION = 0;
const int DEFAULT_RESOLUTION_VALUE = 4;
const int MAX_RESOLUTION = 4;

const int MIN_DAMPING = 0;
const int DEFAULT_DAMPING_VALUE = 4;
#define get_MAX_DAMPING(resolution) ((1 << resolution) - 1)

const int MIN_OFFSET = 0;
const int DEFAULT_OFFSET_VALUE = 4;
#define get_MAX_OFFSET(resolution) ((1 << resolution) - 1)
///////////////////////////////

///////COMPRESSION PARAMETERS////////////
enum LocalSumType {
	WIDE_NEIGHBOR_ORIENTED,
	NARROW_NEIGHBOR_ORIENTED,
	WIDE_COLUMN_ORIENTED,
	NARROW_COLUMN_ORIENTED
};

const bool DEFAULT_FULL_PREDICTION_ENABLED = true;
const enum LocalSumType DEFAULT_LOCAL_SUM_TYPE = WIDE_NEIGHBOR_ORIENTED;

const int MIN_OMEGA = 4;
const int DEFAULT_OMEGA = 19;
const int MAX_OMEGA = 19;

const int MIN_V = -6;
const int DEFAULT_V_MIN = -1;
const int DEFAULT_V_MAX = 3;
const int MAX_V = 9;

const int MIN_T_EXP = 4;
const int DEFAULT_T_EXP = 6;
const int MAX_T_EXP = 11;

const int MIN_P = 0;
const int DEFAULT_P = 3;
const int MAX_P = 15;

const int MIN_ACC_INIT_CONSTANT = 0;
const int DEFAULT_ACC_INIT_CONSTANT = 5;
#define get_MAX_ACC_INIT_CONSTANT(depth) min(depth - 2, 14)

const int MIN_WEIGHT_EXPONENT_OFFSET = -6;
const int DEFAULT_WEIGHT_EXPONENT_OFFSET = 0;
const int MAX_WEIGHT_EXPONENT_OFFSET = 5;

#define get_MIN_R(depth, omega) max(32, depth + omega + 2)
const int DEFAULT_R = 64;
const int MAX_R = 64;
////////////////////////////////////////

//////////ENCODER PARAMETERS///////////
const int MIN_U_MAX = 8;
const int DEFAULT_U_MAX = 18;
const int MAX_U_MAX = 32;

const int MIN_GAMMA_ZERO = 1;
const int DEFAULT_GAMMA_ZERO = 1;
const int MAX_GAMMA_ZERO = 8;

const int MIN_GAMMA_STAR = 4;
const int DEFAULT_GAMMA_STAR = 6;
const int MAX_GAMMA_STAR = 11;
///////////////////////////////////////


////
//COMPRESSION PARAMETERS
////

inline long get_s_mid(int depth) 	{ return 1l << (depth - 1); }
inline long get_s_min() 			{ return 0;}
inline long get_s_max(int depth) 	{ return (1l << depth) - 1; }
inline int get_w_min(int omega) 	{ return -(1 << (omega + 2)); }		//EQ 30
inline int get_w_max(int omega) 	{ return (1 << (omega + 2)) - 1; }	//EQ 30


typedef struct {
	enum LocalSumType local_sum_type;
	int depth;
	bool full_prediction_mode;
	int prediction_bands;
	int omega;
	int r;
	
	//simplified to a single value
	int abs_err;
	int rel_err;

	bool use_abs_err_limit;
	bool use_rel_err_limit; 
	int abs_err_limit_bit_depth;
	int rel_err_limit_bit_depth;
	
	//simplified to a single value
	int resolution;
	int damping;
	int offset;
	
	int t_inc_exp;
	int vmin;
	int vmax;

	//simplified to a single value
	int intra_band_weight_exponent_offset;
	int inter_band_weight_exponent_offset; 
	
	int u_max;
	int gamma_zero;
	int gamma_star;

	//simplified to a single value
	int accumulator_initialization_constant;

	//dimensions
	int bands;
	int samples;
	int lines;



	//Non base values that are calculated on the fly with setters
	int weights_per_band;
	int samples_per_image;
	int samples_per_band;

	int s_mid;
	int s_min;
	int s_max;
	int w_min;
	int w_max;


	//Encoder parameters
	int * accumulator; //need one per band
	int counter_reset_value;
} CompressionParameters;


void recalc_encoder_params(CompressionParameters * cp);
int set_dimensions(CompressionParameters * cp, int bands, int lines, int samples);
int set_defaults(CompressionParameters * cp);
void recalc_weight_vector_length(CompressionParameters * cp);

int set_full_prediction_mode(bool enable, CompressionParameters * cp);
int set_prediction_bands(int prediction_bands, CompressionParameters * cp);
int set_accumulator_initialization_constant(int accumulator_initialization_constant, CompressionParameters * cp);
int set_encoder_update_params(int u_max, int gamma_zero, int gamma_star, CompressionParameters * cp);
int set_weight_exponent_offset(int weight_exponent_offset, CompressionParameters * cp);
int set_weight_update_params(int t_inc_exp, int vmin, int vmax, CompressionParameters * cp);
int set_near_lossless_params(int resolution, int damping, int offset, CompressionParameters * cp);
int set_omega(int omega, CompressionParameters * cp);
int set_depth(int depth, CompressionParameters * cp);	
int set_r(int r, CompressionParameters * cp);
void set_local_sum_type(enum LocalSumType local_sum_type, CompressionParameters * cp);
int set_errors(int abs_err_limit_bit_depth, int rel_err_limit_bit_depth, int abs_err, int rel_err, bool use_abs_err_limit, bool use_rel_err_limit, CompressionParameters * cp);


void compress(int * block, CompressionParameters * cp, BitOutputStream * bos);
int * decompress(BitInputStream * bis, CompressionParameters * cp);


long calc_local_sum(int b, int l, int s, int * rep_block, int samples, CompressionParameters * cp);
long calc_central_local_diff(int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp);
long calc_north_diff (int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp);
long calc_west_diff (int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp);
long calc_north_west_diff (int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp);
int * get_initial_weights(CompressionParameters * cp);
long calc_predicted_central_diff (int b, int l, int s, int * weights, long north_diff, long west_diff, long north_west_diff, int * diff_block, CompressionParameters * cp);
long calc_high_resolution_predicted_sample_value(long predicted_central_diff, long local_sum, CompressionParameters * cp);
long calc_double_resolution_sample_value(int b, int l, int s, long high_resolution_predicted_sample_value, int * block, CompressionParameters * cp);
long calc_predicted_sample_value(long double_resolution_predicted_sample_value);
long calc_prediction_residual(long sample, long predicted_sample_value);
long calc_quantizer_index(long predition_residual, long maximum_error_value, int t);
long calc_max_err_val(int b, long predicted_sample_value, CompressionParameters * cp);
long calc_sample_representative(int l, int s, long double_resolution_sample_representative, int sample);
long calc_double_resolution_sample_representative(int b, long clipped_quantizer_bin_center, long quantizer_index, long maximum_error_value, long high_resolution_predicted_sample_value, CompressionParameters * cp);
long calc_clipped_quantizer_bin_center(long predicted_sample_value, long quantizer_index, long maximum_error_value, CompressionParameters * cp);
long calc_double_resolution_prediction_error(long clipped_quantizer_bin_center, long double_resolution_predicted_sample_value);
long calc_weight_update_scaling_exponent(int t, int samples, CompressionParameters * cp);
int update_weight(int weight, long double_resolution_prediction_error, long diff, long weight_update_scaling_exponent, int weight_exponent_offset, int t, CompressionParameters * cp);
long calc_mapped_quantizer_index(long quantizer_index, long theta, long double_resolution_predicted_sample_value);
long get_lower_theta(int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long get_upper_theta(int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long calc_theta(int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long decalc_quantizer_index(long mapped_quantizer_index, long theta, long double_resolution_predicted_sample_value, int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp);
long decalc_sample(long prediction_residual, long predicted_sample_value);
long decalc_prediction_residual(int t, long quantizer_index, long maximum_error_value);

////
//ENTROPY CODING
////
	
int get_counter_value(int t, CompressionParameters * cp);
void length_limited_golomb_power_of_two_code(int u_int_val, int u_int_code_index,  BitOutputStream * bos, CompressionParameters * cp);
int length_limited_golomb_power_of_two_decode(int u_int_code_index, BitInputStream * bis, CompressionParameters * cp);
int get_u_int_code_index(int b, int t, int c_value, CompressionParameters * cp);
void update_accumulator(int b, int t, int mapped_quantizer_index, int c_value, CompressionParameters * cp);
void code(int mapped_quantizer_index, int t, int b, BitOutputStream * bos, CompressionParameters * cp);
int decode(int t, int b, BitInputStream * bis, CompressionParameters * cp);

////
//QUANTIZATION
////

long quantize(long value, long parameter);
long dequantize(long qvalue, long parameter);



#endif //__CCSDS_1230B2__