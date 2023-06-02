#include "ccsds_1230b2_codec.h"
#include "debug.h"
#include "ccsds_hybrid_coder.h"



//Call set_dimensions and set_defaults


void recalc_encoder_params(CompressionParameters * cp) {
	if (cp->accumulator != NULL)
		free(cp->accumulator);
	cp->accumulator = calloc(cp->bands, sizeof(int));
	cp->counter_reset_value = 1 << cp->gamma_zero;

	int b;
	for (b = 0; b < cp->bands; b++) {
		cp->accumulator[b] = cp->accumulator_initialization_constant;
	}
}	

int set_dimensions(CompressionParameters * cp, int bands, int lines, int samples) {
	if (bands < 0 || lines < 0 || samples < 0)
		return -1;
	cp->bands = bands;
	cp->lines = lines;
	cp->samples = samples;

	cp->samples_per_image = bands * lines * samples;
	cp->samples_per_band  = lines * samples;

	recalc_encoder_params(cp);
	return 0;
}

int set_defaults(CompressionParameters * cp) {
	int ret = 0;
	ret |= set_dimensions(cp, 1, 1, 1);
		   set_local_sum_type(DEFAULT_LOCAL_SUM_TYPE, cp);
	ret |= set_errors(DEFAULT_ABSOLUTE_ERROR_LIMIT_BIT_DEPTH, DEFAULT_RELATIVE_ERROR_LIMIT_BIT_DEPTH, DEFAULT_ABS_ERR_VALUE, DEFAULT_REL_ERR_VALUE, DEFAULT_USE_ABS_ERR, DEFAULT_USE_REL_ERR, cp);
	ret |= set_depth(DEFAULT_DEPTH, cp);
	ret |= set_near_lossless_params(DEFAULT_RESOLUTION_VALUE, DEFAULT_DAMPING_VALUE, DEFAULT_OFFSET_VALUE, cp);
	ret |= set_weight_update_params(DEFAULT_T_EXP, DEFAULT_V_MIN, DEFAULT_V_MAX, cp);
	ret |= set_omega(DEFAULT_OMEGA, cp);
	ret |= set_r(DEFAULT_R, cp);
	ret |= set_encoder_update_params(DEFAULT_U_MAX, DEFAULT_GAMMA_ZERO, DEFAULT_GAMMA_STAR, cp);
	ret |= set_accumulator_initialization_constant(DEFAULT_ACC_INIT_CONSTANT, cp);
	ret |= set_prediction_bands(DEFAULT_P, cp);
	ret |= set_full_prediction_mode(DEFAULT_FULL_PREDICTION_ENABLED, cp);
	ret |= set_weight_exponent_offset(DEFAULT_WEIGHT_EXPONENT_OFFSET, cp);
	return ret;
}


void recalc_weight_vector_length(CompressionParameters * cp) {
	if (cp->full_prediction_mode) 
		cp->weights_per_band = cp->prediction_bands + 3;
	else
		cp->weights_per_band = cp->prediction_bands;
}

int set_full_prediction_mode(bool enable, CompressionParameters * cp) {
	cp->full_prediction_mode = enable;
	recalc_weight_vector_length(cp);
	return 0;
}

int set_prediction_bands(int prediction_bands, CompressionParameters * cp) {
	if (prediction_bands < MIN_P || prediction_bands > MAX_P)
		return -1;
	
	cp->prediction_bands = prediction_bands;
	recalc_weight_vector_length(cp);
	return 0;
}
	
int set_accumulator_initialization_constant(int accumulator_initialization_constant, CompressionParameters * cp) {
	if (accumulator_initialization_constant < MIN_ACC_INIT_CONSTANT || accumulator_initialization_constant > get_MAX_ACC_INIT_CONSTANT(cp->depth))
		return -1;
	cp->accumulator_initialization_constant = accumulator_initialization_constant;		
	recalc_encoder_params(cp);
	return 0;
}
	
int set_encoder_update_params(int u_max, int gamma_zero, int gamma_star, CompressionParameters * cp) {
	if (u_max < MIN_U_MAX || u_max > MAX_U_MAX)
		return -1;
	cp->u_max = u_max;
	if (gamma_zero < MIN_GAMMA_ZERO || gamma_zero > MAX_GAMMA_ZERO) 
		return -1;
	cp->gamma_zero = gamma_zero;
	if (gamma_star < max(MIN_GAMMA_STAR, cp->gamma_zero + 1) || gamma_star > MAX_GAMMA_STAR)
		return -1;
	cp->gamma_star = gamma_star;
	return 0;
}
	
int set_weight_exponent_offset(int weight_exponent_offset, CompressionParameters * cp) {
	if (weight_exponent_offset < MIN_WEIGHT_EXPONENT_OFFSET || weight_exponent_offset > MAX_WEIGHT_EXPONENT_OFFSET)
		return -1;
	cp->intra_band_weight_exponent_offset = weight_exponent_offset;
	cp->inter_band_weight_exponent_offset = weight_exponent_offset;
	return 0;
}
	
int set_weight_update_params(int t_inc_exp, int vmin, int vmax, CompressionParameters * cp) {
	if (t_inc_exp < MIN_T_EXP || t_inc_exp > MAX_T_EXP)
		return -1;
	cp->t_inc_exp = t_inc_exp;
	if (vmin < MIN_V || vmax > MAX_V || vmin >= vmax)
		return -1;
	cp->vmax = vmax;
	cp->vmin = vmin;
	return 0;
}
	
int set_near_lossless_params(int resolution, int damping, int offset, CompressionParameters * cp) {
	if (resolution < MIN_RESOLUTION || resolution > MAX_RESOLUTION)
		return -1;
	cp->resolution = resolution;
	if (damping < MIN_DAMPING || damping > get_MAX_DAMPING(cp->resolution))
		return -1;
	cp->damping = damping;
	if (offset < MIN_OFFSET || offset > get_MAX_OFFSET(cp->resolution))
		return -1;
	cp->offset = offset;
	return 0;
}

int set_omega(int omega, CompressionParameters * cp) {
	if (omega < MIN_OMEGA || omega > MAX_OMEGA)
		return -1;

	cp->omega = omega;
	cp->w_min = -(1 << (omega + 2));
	cp->w_max = (1 << (omega + 2)) - 1;
	//need to recheck r
	return set_r(cp->r, cp);
}

int set_depth(int depth, CompressionParameters * cp) {
	if (depth < MIN_DEPTH || depth > MAX_DEPTH)
		return -1;

	cp->depth = depth;
	cp->s_mid = 1l << (depth - 1);
	cp->s_min = 0;
	cp->s_max = (1l << depth) - 1;
	//need to recheck r
	return set_r(cp->r, cp);
}

	
int set_r(int r, CompressionParameters * cp) {
	if (r < get_MIN_R(cp->depth, cp->omega) || r > MAX_R) 
		return -1;
	cp->r = r;
	return 0;
}
	
void set_local_sum_type(enum LocalSumType local_sum_type, CompressionParameters * cp) {
	cp->local_sum_type = local_sum_type;
}
	
int set_errors(int abs_err_limit_bit_depth, int rel_err_limit_bit_depth, int abs_err, int rel_err, bool use_abs_err_limit, bool use_rel_err_limit, CompressionParameters * cp) {
	cp->abs_err_limit_bit_depth = abs_err_limit_bit_depth;
	cp->rel_err_limit_bit_depth = rel_err_limit_bit_depth;
	if (abs_err < MIN_ABS_ERR_VALUE || abs_err > get_MAX_ABS_ERR_VALUE(cp->abs_err_limit_bit_depth))
		return -1;
	if (rel_err < MIN_REL_ERR_VALUE || rel_err > get_MAX_REL_ERR_VALUE(cp->rel_err_limit_bit_depth))
		return -1;
	cp->abs_err = abs_err;
	cp->rel_err = rel_err;
	cp->use_abs_err_limit = use_abs_err_limit;
	cp->use_rel_err_limit = use_rel_err_limit;
	return 0;
}
	
	


void compress(int * block, CompressionParameters * cp, BitOutputStream * bos, Checker * checker) {
	//allocate the representative and difference blocks
	int * rep_block 	= calloc(cp->samples_per_image, sizeof(int));
	int * diff_block	= calloc(cp->samples_per_image, sizeof(int));
	//allocate the weight vector
	int * weights 		= get_initial_weights(cp);

	
	//LOOPS are interchangeable as long as samples come after lines
	//BIP: lines>samples>bands
	//BIL: lines>bands>sample
	//BSQ: bands>lines>samples
	int l, s, b;
	for (l = 0; l < cp->lines; l++) {
		for (s = 0; s < cp->samples; s++) {
			for (b = 0; b < cp->bands; b++) {
				int t = l*cp->samples + s;

				////LOCAL SUM BEGIN 4.4
				long local_sum 										= calc_local_sum(b, l, s, rep_block, cp->samples, cp);
				////LOCAL SUM END

				////LOCAL DIFF BEGIN 4.5
				long north_diff 									= calc_north_diff(b, l, s, rep_block, local_sum, cp);
				long west_diff 										= calc_west_diff(b, l, s, rep_block, local_sum, cp);
				long north_west_diff								= calc_north_west_diff(b, l, s, rep_block, local_sum, cp);
				////LOCAL DIFF END
				
				//PREDICTED CENTRAL LOCAL DIFFERENCE 4.7.1
				long predicted_central_diff 						= calc_predicted_central_diff(b, l, s, weights, north_diff, west_diff, north_west_diff, diff_block, cp);
				//PREDICTED CENTRAL LOCAL DIFFERENCE END
			
				//HR PREDICTED SAMPLE VALUE 4.7.2
				long high_resolution_predicted_sample_value 		= calc_high_resolution_predicted_sample_value(predicted_central_diff, local_sum, cp);
				//DR PREDICTED SAMPLE VALUE 4.7.3
				long double_resolution_predicted_sample_value 		= calc_double_resolution_sample_value(b, l, s, high_resolution_predicted_sample_value, block, cp);
				//PREDICTED SAMMPLE VALUE 4.7.4
				long predicted_sample_value 						= calc_predicted_sample_value(double_resolution_predicted_sample_value);
				//PRED SAMPLE VALUE END
					
				//PRED RES 4.8.1 + 4.8.2.1
				long prediction_residual 							= calc_prediction_residual(D3(block, b, l, s, cp), predicted_sample_value);
				long maximum_error_value 							= calc_max_err_val(predicted_sample_value, cp);
				long quantizer_index 								= calc_quantizer_index(prediction_residual, maximum_error_value, t);
				
				//DR SAMPLE REPRESENTATIVE AND SAMPLE REPRESENTATIVE 4.9
				long clipped_quantizer_bin_center 					= calc_clipped_quantizer_bin_center(predicted_sample_value, quantizer_index, maximum_error_value, cp);
				long double_resolution_sample_representative 		= calc_double_resolution_sample_representative(clipped_quantizer_bin_center, quantizer_index, maximum_error_value, high_resolution_predicted_sample_value, cp);
				
				//DR PRED ERR 4.10.1
				long double_resolution_prediction_error 			= calc_double_resolution_prediction_error(clipped_quantizer_bin_center, double_resolution_predicted_sample_value);
				//WEIGHT UPDATE SCALING EXPONENT 4.10.2
				long weight_update_scaling_exponent 				= calc_weight_update_scaling_exponent(t, cp->samples, cp);
				//WEIGHT UPDATE 4.10.3
				if (t > 0) {
					int windex = 0;
					if (cp->full_prediction_mode) {
						int weight_exponent_offset = cp->intra_band_weight_exponent_offset;
						//north, west, northwest
						W2(weights, b, 0, cp) 							= update_weight(W2(weights, b, 0, cp), double_resolution_prediction_error, north_diff, weight_update_scaling_exponent, weight_exponent_offset, cp);
						W2(weights, b, 1, cp) 							= update_weight(W2(weights, b, 1, cp), double_resolution_prediction_error, west_diff, weight_update_scaling_exponent, weight_exponent_offset, cp);
						W2(weights, b, 2, cp) 							= update_weight(W2(weights, b, 2, cp), double_resolution_prediction_error, north_west_diff, weight_update_scaling_exponent, weight_exponent_offset, cp);
						windex = 3;
					}
					for (int p = 0; p < cp->prediction_bands; p++) {
						if (b - p > 0) 
							W2(weights, b, windex+p, cp) 				= update_weight(W2(weights, b, windex+p, cp) , double_resolution_prediction_error, D3(diff_block, b-p-1, l, s, cp), weight_update_scaling_exponent, cp->inter_band_weight_exponent_offset, cp);
					}
				}
				
				//MAPPED QUANTIZER INDEX 4.11
				long theta 											= calc_theta(t, predicted_sample_value, maximum_error_value, cp);
				long mapped_quantizer_index 						= calc_mapped_quantizer_index(quantizer_index, theta, double_resolution_predicted_sample_value);
				
				//Send to coder to generate the binary output stream
				//TODO
				code((int) mapped_quantizer_index, t, b, bos, cp, checker);
				
				long sample_representative 							= calc_sample_representative(l, s, double_resolution_sample_representative, D3(block, b, l, s, cp));
				D3(rep_block, b, l, s, cp) 							= (int) sample_representative;
				
				long central_local_diff 							= calc_central_local_diff(b, l, s, rep_block, local_sum, cp);
				D3(diff_block, b, l, s, cp) 						= (int) central_local_diff;

				#ifdef CHECK_VALUES
					addl(checker, local_sum);
					addl(checker, north_diff);
					addl(checker, west_diff);
					addl(checker, north_west_diff);
					addl(checker, predicted_central_diff);
					addl(checker, high_resolution_predicted_sample_value);
					addl(checker, double_resolution_predicted_sample_value);
					addl(checker, predicted_sample_value);
					addl(checker, prediction_residual);
					addl(checker, maximum_error_value);
					addl(checker, quantizer_index);
					addl(checker, clipped_quantizer_bin_center);
					addl(checker, double_resolution_sample_representative);
					addl(checker, double_resolution_prediction_error);
					addl(checker, weight_update_scaling_exponent);
					//WEIGHTS??
					//
					addl(checker, theta);
					addl(checker, mapped_quantizer_index);
					addl(checker, sample_representative);
					addl(checker, central_local_diff);
				#endif
			}
		}
	}

	flush(bos);
}


int * decompress(BitInputStream * bis, CompressionParameters * cp, Checker * checker) {
	//image will be output here
	int * block 		= calloc(cp->samples_per_image, sizeof(int));
	//allocate the representative and difference blocks
	int * rep_block 	= calloc(cp->samples_per_image, sizeof(int));
	int * diff_block	= calloc(cp->samples_per_image, sizeof(int));
	//allocate the weight vector
	int * weights 		= get_initial_weights(cp);
	
	for (int l = 0; l < cp->lines; l++) {
		for (int s = 0; s < cp->samples; s++) {
			for (int b = 0; b < cp->bands; b++) {
				int t = l*cp->samples + s;

				#ifdef CHECK_VALUES
					set_position(checker, b, l, s);
				#endif
				
				long mapped_quantizer_index 						= (long) decode(t, b, bis, cp, checker);
				
				////LOCAL SUM BEGIN 4.4
				long local_sum 										= calc_local_sum(b, l, s, rep_block, cp->samples, cp);
				////LOCAL SUM END

				////LOCAL DIFF BEGIN 4.5
				long north_diff 									= calc_north_diff(b, l, s, rep_block, local_sum, cp);
				long west_diff 										= calc_west_diff(b, l, s, rep_block, local_sum, cp);
				long north_west_diff 								= calc_north_west_diff(b, l, s, rep_block, local_sum, cp);
				////LOCAL DIFF END
				
				//PREDICTED CENTRAL LOCAL DIFFERENCE 4.7.1
				long predicted_central_diff 						= calc_predicted_central_diff(b, l, s, weights, north_diff, west_diff, north_west_diff, diff_block, cp);
				//PREDICTED CENTRAL LOCAL DIFFERENCE END
			
				
				//HR PREDICTED SAMPLE VALUE 4.7.2
				long high_resolution_predicted_sample_value 		= calc_high_resolution_predicted_sample_value(predicted_central_diff, local_sum, cp);
				//DR PREDICTED SAMPLE VALUE 4.7.3
				long double_resolution_predicted_sample_value 		= calc_double_resolution_sample_value(b, l, s, high_resolution_predicted_sample_value, block, cp); //?
				//PREDICTED SAMMPLE VALUE 4.7.4
				long predicted_sample_value 						= calc_predicted_sample_value(double_resolution_predicted_sample_value);
				//PRED SAMPLE VALUE END
				
				//UNDO COMPRESSION
				long maximum_error_value 							= calc_max_err_val(predicted_sample_value, cp);
				long theta 											= calc_theta(t, predicted_sample_value, maximum_error_value, cp);
				
				long quantizer_index 								= decalc_quantizer_index(mapped_quantizer_index, theta, double_resolution_predicted_sample_value, t, predicted_sample_value, maximum_error_value, cp);
				long prediction_residual 							= decalc_prediction_residual(t, quantizer_index, maximum_error_value);
				long sample 										= decalc_sample(prediction_residual, predicted_sample_value);
				D3(block, b, l, s, cp) = (int) sample;
				//UNDO COMPRESSION END
								
				//DR SAMPLE REPRESENTATIVE AND SAMPLE REPRESENTATIVE 4.9
				long clipped_quantizer_bin_center 					= calc_clipped_quantizer_bin_center(predicted_sample_value, quantizer_index, maximum_error_value, cp);
				long double_resolution_sample_representative 		= calc_double_resolution_sample_representative(clipped_quantizer_bin_center, quantizer_index, maximum_error_value, high_resolution_predicted_sample_value, cp);
		
				//DR PRED ERR 4.10.1
				long double_resolution_prediction_error 			= calc_double_resolution_prediction_error(clipped_quantizer_bin_center, double_resolution_predicted_sample_value);
				//WEIGHT UPDATE SCALING EXPONENT 4.10.2
				long weight_update_scaling_exponent 				= calc_weight_update_scaling_exponent(t, cp->samples, cp);
				//WEIGHT UPDATE 4.10.3
				if (t > 0) {
					int windex = 0;
					if (cp->full_prediction_mode) {
						int weight_exponent_offset = cp->intra_band_weight_exponent_offset;
						//north, west, northwest
						W2(weights, b, 0, cp) 						= update_weight(W2(weights, b, 0, cp), double_resolution_prediction_error, north_diff, weight_update_scaling_exponent, weight_exponent_offset, cp);
						W2(weights, b, 1, cp) 						= update_weight(W2(weights, b, 1, cp), double_resolution_prediction_error, west_diff, weight_update_scaling_exponent, weight_exponent_offset, cp);
						W2(weights, b, 2, cp) 						= update_weight(W2(weights, b, 2, cp), double_resolution_prediction_error, north_west_diff, weight_update_scaling_exponent, weight_exponent_offset, cp);
						windex = 3;
					}
					for (int p = 0; p < cp->prediction_bands; p++) {
						if (b - p > 0) 
							W2(weights, b, windex + p, cp) 			= update_weight(W2(weights, b, windex + p, cp), double_resolution_prediction_error, D3(diff_block, b-p-1, l, s, cp), weight_update_scaling_exponent, cp->inter_band_weight_exponent_offset, cp);
					}
				}
					

				long sample_representative 							= calc_sample_representative(l, s, double_resolution_sample_representative, (int) sample);
				D3(rep_block, b, l, s, cp) 							= (int) sample_representative;

				long central_local_diff 							= calc_central_local_diff(b, l, s, rep_block, local_sum, cp);
				D3(diff_block, b, l, s, cp) 						= (int) central_local_diff;


				#ifdef CHECK_VALUES
					chkl(set_message(checker, "LS"), local_sum);
					chkl(set_message(checker, "ND"), north_diff);
					chkl(set_message(checker, "WD"), west_diff);
					chkl(set_message(checker, "NWD"), north_west_diff);
					chkl(set_message(checker, "PCD"), predicted_central_diff);
					chkl(set_message(checker, "HRPSV"), high_resolution_predicted_sample_value);
					chkl(set_message(checker, "DRPSV"), double_resolution_predicted_sample_value);
					chkl(set_message(checker, "PSV"), predicted_sample_value);
					if (maximum_error_value == 0) {
						chkl(set_message(checker, "PR"), prediction_residual);
					} else { //pr might change
						burn_bytes(checker, sizeof(long));
					}
					chkl(set_message(checker, "MEV"), maximum_error_value);
					chkl(set_message(checker, "QI"), quantizer_index);
					chkl(set_message(checker, "CQBC"), clipped_quantizer_bin_center);
					chkl(set_message(checker, "DRSR"), double_resolution_sample_representative);
					chkl(set_message(checker, "DRPE"), double_resolution_prediction_error);
					chkl(set_message(checker, "WUSE"), weight_update_scaling_exponent);
					//WEIGHTS??
					//
					chkl(set_message(checker, "THETA"), theta);
					chkl(set_message(checker, "MQI"), mapped_quantizer_index);
					chkl(set_message(checker, "SR"), sample_representative);
					chkl(set_message(checker, "CLD"), central_local_diff);
					//
					exit_if_failed(checker);
				#endif
			}
		}
	}
	
	return block;
}
	

long calc_local_sum(int b, int l, int s, int * rep_block, int samples, CompressionParameters * cp) { //EQ 20, 21, 22, 23		
	long local_sum = 0;
	switch (cp->local_sum_type) {
		case WIDE_NEIGHBOR_ORIENTED: { //EQ 20
			if (l > 0 && s > 0 && s < samples - 1) {
				local_sum = D3(rep_block, b, l, s-1, cp) + D3(rep_block, b, l-1, s-1, cp) + D3(rep_block, b, l-1, s, cp) + D3(rep_block, b, l-1, s+1, cp);
			} else if (l == 0 && s > 0) {
				local_sum = D3(rep_block, b, l, s-1, cp) << 2;
			} else if (l > 0 && s == 0) {
				local_sum = (D3(rep_block, b, l-1, s, cp) + D3(rep_block, b, l-1, s+1, cp)) << 1;
			} else if (l > 0 && s == samples - 1) {
				local_sum = D3(rep_block, b, l, s-1, cp) + D3(rep_block, b, l-1, s-1, cp) + (D3(rep_block, b, l-1, s, cp) << 1);
			} 
			break;
		}
		case NARROW_NEIGHBOR_ORIENTED: { //EQ 21
			if (l > 0 && s > 0 && s < samples - 1) {
				local_sum = D3(rep_block, b, l-1, s-1, cp) + (D3(rep_block, b, l-1, s, cp) << 1) + D3(rep_block, b, l-1, s+1, cp);
			} else if (l == 0 && s > 0 && b > 0) {
				local_sum = D3(rep_block, b-1, l, s-1, cp);
			} else if (l > 0 && s == 0) {
				local_sum = (D3(rep_block, b, l-1, s, cp) + D3(rep_block, b, l-1, s+1, cp)) << 1;
			} else if (l > 0 && s == samples - 1) {
				local_sum = (D3(rep_block, b, l-1, s-1, cp) + D3(rep_block, b, l-1, s, cp)) << 1;
			} else if (l == 0 && s > 0 && b == 0) {
				local_sum = cp->s_mid << 2;
			} 
			break;
		}
		case WIDE_COLUMN_ORIENTED: { //EQ 22
			if (l > 0) {
				local_sum = D3(rep_block, b, l-1, s, cp) << 2;
			} else if (l == 0 && s > 0) {
				local_sum = D3(rep_block, b, l, s-1, cp) << 2;
			} 
			break;
		}
		case NARROW_COLUMN_ORIENTED: { //EQ 23
			if (l > 0) {
				local_sum = D3(rep_block, b, l-1, s, cp) << 2;
			} else if (l == 0 && s > 0 && b > 0) {
				local_sum = D3(rep_block, b-1, l, s-1, cp) << 2;
			} else if (l == 0 && s > 0 && b == 0) {
				local_sum = cp->s_mid << 2;
			} 
			break;
		}
		default:
			break;
	}
	return local_sum;
}
	
long calc_central_local_diff(int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp) { //EQ 24
	long res = (D3(rep_block, b, l, s, cp) << 2) - local_sum; 
	return res;
}
	
long calc_north_diff (int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp) { //EQ 25		
	if (cp->full_prediction_mode && (s != 0 || l != 0))
		if (l > 0) 
			return (D3(rep_block, b, l-1, s, cp) << 2) - local_sum;
	return 0;
}
	
long calc_west_diff (int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp) { //EQ 26
	if (cp->full_prediction_mode && (s != 0 || l != 0)) {
		if (s > 0 && l > 0) {
			return (D3(rep_block, b, l, s-1, cp) << 2) - local_sum;
		} else if (s == 0 && l > 0) {
			return (D3(rep_block, b, l-1, s, cp) << 2) - local_sum;
		}
	}
	return 0;
}
	
long calc_north_west_diff (int b, int l, int s, int * rep_block, long local_sum, CompressionParameters * cp) { //EQ 27
	if (cp->full_prediction_mode && (s != 0 || l != 0)) {
		if (s > 0 && l > 0) {
			return (D3(rep_block, b, l-1, s-1, cp) << 2) - local_sum;
		} else if (s == 0 && l > 0) {
			return (D3(rep_block, b, l-1, s, cp) << 2) - local_sum;
		}
	}
	return 0;
}

int * get_initial_weights(CompressionParameters * cp) { //EQ 31, 32, 33, 34
	int * weights = (int *) calloc(cp->bands*cp->weights_per_band, sizeof(int));

	int b;
	for (b = 0; b < cp->bands; b++) {
		int windex = 0;
		if (cp->full_prediction_mode) {
			W2(weights, b, 0, cp) = 0;
			W2(weights, b, 1, cp) = 0;
			W2(weights, b, 2, cp) = 0;
			windex = 3;
		}
		int p;
		for (p = 0; p < cp->prediction_bands; p++) {
			if (p == 0) {
				W2(weights, b, p + windex, cp) = (7*(1 << cp->omega)) >> 3;
			} else {
				W2(weights, b, p + windex, cp) = W2(weights, b, p + windex - 1, cp) >> 3;
			}
		}
	}

	return weights;
}
	
	
long calc_predicted_central_diff (int b, int l, int s, int * weights, long north_diff, long west_diff, long north_west_diff, int * diff_block, CompressionParameters * cp) { //EQ 36

	long predicted_central_diff = 0;
	if (b != 0 || cp->full_prediction_mode) {
		int windex = 0;
		if (cp->full_prediction_mode) {
			predicted_central_diff += W2(weights, b, 0, cp) * north_diff;
			predicted_central_diff += W2(weights, b, 1, cp) * west_diff;
			predicted_central_diff += W2(weights, b, 2, cp) * north_west_diff;
			windex = 3;
		}
		for (int p = 0; p < cp->prediction_bands; p++) {
			if (b - p > 0) 
				predicted_central_diff += W2(weights, b, p + windex, cp) * D3(diff_block, b-p-1, l, s, cp);
		}
	}
	
	return predicted_central_diff;						
}
	
long calc_high_resolution_predicted_sample_value(long predicted_central_diff, long local_sum, CompressionParameters * cp) { // EQ 37
	long high_resolution_predicted_sample_value = 
			mod_R(predicted_central_diff + ((local_sum - (cp->s_mid << 2)) << cp->omega), cp->r)
			+ (cp->s_mid << (cp->omega + 2))
			+ (1 << (cp->omega + 1));
	return clamp(
			high_resolution_predicted_sample_value, 
			cp->s_min << (cp->omega + 2), 
			(cp->s_max << (cp->omega + 2)) + (1 << (cp->omega + 1)));
}
	
long calc_double_resolution_sample_value(int b, int l, int s, long high_resolution_predicted_sample_value, int * block, CompressionParameters * cp) { //EQ 38
	long double_resolution_predicted_sample_value = 0;
	if (s > 0 || l > 0) {
		double_resolution_predicted_sample_value = high_resolution_predicted_sample_value >> (cp->omega + 1);
	} else if (cp->prediction_bands == 0 || b == 0) {
		double_resolution_predicted_sample_value = cp->s_mid << 1;
	} else {
		double_resolution_predicted_sample_value = D3(block, b-1, l, s, cp) << 1;
	}
	return double_resolution_predicted_sample_value;
}

long calc_predicted_sample_value(long double_resolution_predicted_sample_value) { //EQ 9
	return double_resolution_predicted_sample_value >> 1;
}
	
long calc_prediction_residual(long sample, long predicted_sample_value) { //EQ 40
	 return sample - predicted_sample_value;
}
	
long calc_quantizer_index(long predition_residual, long maximum_error_value, int t) { //EQ 41
	if (t == 0)
		return predition_residual;
	return quantize(predition_residual, maximum_error_value);
}
	
long calc_max_err_val(long predicted_sample_value, CompressionParameters * cp) { //EQ 42, 43, 44, 45
	long maximum_error_value = 0;
	if (cp->use_abs_err_limit && cp->use_rel_err_limit) {
		maximum_error_value = min(cp->abs_err, (cp->rel_err*absl(predicted_sample_value)) >> cp->depth); //EQ 45
	} else if (cp->use_rel_err_limit) {
		maximum_error_value = (cp->rel_err*absl(predicted_sample_value)) >> cp->depth; //EQ 44
	} else if (cp->use_abs_err_limit) {
		maximum_error_value = cp->abs_err; //EQ 43
	} else { //no errors
		maximum_error_value = 0; //EQ 42
	}
	return maximum_error_value;
}

long calc_sample_representative(int l, int s, long double_resolution_sample_representative, int sample) { //EQ 46
	if (l > 0 || s > 0) {
		return (double_resolution_sample_representative + 1) >> 1;
	} else {
		return sample;
	}
}

long calc_double_resolution_sample_representative(long clipped_quantizer_bin_center, long quantizer_index, long maximum_error_value, long high_resolution_predicted_sample_value, CompressionParameters * cp) { //EQ 47
	/*long fm = (1 << cp->resolution) - cp->damping;
	long sm = (clipped_quantizer_bin_center << cp->omega) - ((signum(quantizer_index)*maximum_error_value*cp->offset) << (cp->omega - cp->resolution));
	long add = cp->damping*high_resolution_predicted_sample_value - (cp->damping << (cp->omega + 1)); 
	long sby = cp->omega + cp->resolution + 1; 
	return (((fm * sm) << 2) + add) >> sby;*/
	long fm = (1 << cp->resolution) - cp->damping;
	long omega_minus_resolution = (cp->omega - cp->resolution);
	long cqbc_shifted_by_omega = (clipped_quantizer_bin_center << cp->omega);
	long damping_shifted_by_omega_plus_1 = (cp->damping << (cp->omega + 1));
	long omega_plus_res_plus_1 = cp->omega + cp->resolution + 1;
	
	long mev_times_offset = maximum_error_value*cp->offset;
	long hrpsv_times_damping = cp->damping*high_resolution_predicted_sample_value;
	long mev_qui_signed = signum(quantizer_index)*mev_times_offset;
	long mev_qi_shifted = ((mev_qui_signed) << omega_minus_resolution);
	
	long sm = cqbc_shifted_by_omega - mev_qi_shifted;
	long fm_times_sm_sb_2 = (fm * sm) << 2;
	
	long final_unshifted = fm_times_sm_sb_2 +  hrpsv_times_damping - damping_shifted_by_omega_plus_1;
	
	long output = final_unshifted >> omega_plus_res_plus_1;
	return output;
}
	
long calc_clipped_quantizer_bin_center(long predicted_sample_value, long quantizer_index, long maximum_error_value, CompressionParameters * cp) { //EQ 48
	return clamp(predicted_sample_value + quantizer_index*(2*maximum_error_value + 1), cp->s_min, cp->s_max);
}
	
long calc_double_resolution_prediction_error(long clipped_quantizer_bin_center, long double_resolution_predicted_sample_value) { //EQ 49
	return (clipped_quantizer_bin_center << 1) - double_resolution_predicted_sample_value;
}
	
long calc_weight_update_scaling_exponent(int t, int samples, CompressionParameters * cp) { //EQ 50
	return clamp(cp->vmin + ((t - samples) >> cp->t_inc_exp), cp->vmin, cp->vmax) + cp->depth - cp->omega;
}
	
int update_weight(int weight, long double_resolution_prediction_error, long diff, long weight_update_scaling_exponent, int weight_exponent_offset, CompressionParameters * cp) { //EQ 51,52,53,54
	//first of all calculate the exponent above to see if its positive or negative
	int exponent = (int) weight_update_scaling_exponent + weight_exponent_offset;
	int result = weight;
	if (exponent > 0) {
		result += ((((signum_plus((int) double_resolution_prediction_error)*diff) >> exponent) + 1) >> 1);
	} else {
		result += ((((signum_plus((int) double_resolution_prediction_error)*diff) << (-exponent)) + 1) >> 1);
	}
	return clamp(result, cp->w_min, cp->w_max);
}
	
long calc_mapped_quantizer_index(long quantizer_index, long theta, long double_resolution_predicted_sample_value) { //EQ 55
	if (absl( quantizer_index) > theta) {
		return absl( quantizer_index) + theta;
	} else {
		long val;
		if (double_resolution_predicted_sample_value % 2 == 0) {
			val = quantizer_index;
		} else {
			val = -quantizer_index;
		}
		if (val >= 0 && theta >= val) {
			return absl( quantizer_index) << 1;
		} else {
			return (absl( quantizer_index) << 1) - 1;
		}
	}
}
	
long get_lower_theta(int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp) { //EQ 56 a
	if (t == 0) 
		return predicted_sample_value - cp->s_min;
	else 
		return (predicted_sample_value - cp->s_min + maximum_error_value) / (2*maximum_error_value + 1);
}

long get_upper_theta(int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp) { //EQ 56 b
	if (t == 0) 
		return (int) cp->s_max - predicted_sample_value;
	else 
		return (cp->s_max - predicted_sample_value + maximum_error_value) / (2*maximum_error_value + 1);
}

long calc_theta(int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp) { //EQ 56
	return min(get_lower_theta(t, predicted_sample_value, maximum_error_value, cp), get_upper_theta(t, predicted_sample_value, maximum_error_value, cp));
}


////
//Reverse operations
////

long decalc_quantizer_index(long mapped_quantizer_index, long theta, long double_resolution_predicted_sample_value, int t, long predicted_sample_value, long maximum_error_value, CompressionParameters * cp) { //INVERSE OF EQ 55
	if (mapped_quantizer_index > 2*theta) {
		long abs_signed_quantizer_index = mapped_quantizer_index - theta;
		//dunno if positive or negative¿?¿?¿? its collapsed
		//positive or negative depending on which limit theta triggered
		long lTheta = get_lower_theta(t, predicted_sample_value, maximum_error_value, cp);
		long uTheta = get_upper_theta(t, predicted_sample_value, maximum_error_value, cp);
		if (lTheta <= uTheta) {
			return abs_signed_quantizer_index;
		} else {
			return -abs_signed_quantizer_index;
		}
	} else {
		long abs_signed_quantizer_index = (mapped_quantizer_index + 1) >> 1;
		//assume it is possitive and check if not
		if (mapped_quantizer_index % 2 == 0) {
			//in this case it went through the first equation. if it holds, it its positive, otherwise
			//it is negative
			if (minus_one_to_the(double_resolution_predicted_sample_value)*abs_signed_quantizer_index >= 0
					&& minus_one_to_the(double_resolution_predicted_sample_value)*abs_signed_quantizer_index <= theta)
				return abs_signed_quantizer_index;
			return -abs_signed_quantizer_index;
		} else {
			//in this case it went through the second one
			//if the equation holds it is negative, otherwise positive
			if (minus_one_to_the(double_resolution_predicted_sample_value)*abs_signed_quantizer_index >= 0
					&& minus_one_to_the(double_resolution_predicted_sample_value)*abs_signed_quantizer_index <= theta)
				return -abs_signed_quantizer_index;
			return abs_signed_quantizer_index;
		}
	}
}
	
long decalc_sample(long prediction_residual, long predicted_sample_value) {
	return prediction_residual + predicted_sample_value;
}

long decalc_prediction_residual(int t, long quantizer_index, long maximum_error_value) {
	if (t == 0)
		return quantizer_index;
	return dequantize(quantizer_index, maximum_error_value);
}
	



////
//ENTROPY CODING
////


	
	
int get_counter_value(int t, CompressionParameters * cp) {
	int c_thresh = (1 << cp->gamma_star) - (1 << cp->gamma_zero);
	int c_oflow  = (t - ((1 << cp->gamma_star) - (1 << cp->gamma_zero) + 1)) % (1 << (cp->gamma_star - 1));
	int c_value = t <= c_thresh ? (1 << cp->gamma_zero) - 1 + t : ((1 << (cp->gamma_star - 1)) + c_oflow);
			
	return c_value;
}
	
void length_limited_golomb_power_of_two_code(int u_int_val, int u_int_code_index,  BitOutputStream * bos, CompressionParameters * cp) {
	int threshold = u_int_val >> u_int_code_index;
	if (threshold < cp->u_max) {
		//threshold zeroes + 1 + u_int_code_index lsbs of u_int_val
		write_bits(bos, 0, threshold);
		write_bit(bos, 1);
		write_bits(bos, u_int_val, u_int_code_index);
	} else {
		write_bits(bos, 0, cp->u_max);
		write_bits(bos, u_int_val, cp->depth);
	}
}

void reverse_length_limited_golomb_power_of_two_code(int u_int_val, int u_int_code_index,  BitOutputStream * bos, CompressionParameters * cp) {
	int threshold = u_int_val >> u_int_code_index;
	if (threshold < cp->u_max) {
		write_bits(bos, u_int_val, u_int_code_index);
		write_bit(bos, 1);
		write_bits(bos, 0, threshold);
	} else {
		write_bits(bos, u_int_val, cp->depth);
		write_bits(bos, 0, cp->u_max);
	}
}
	
int length_limited_golomb_power_of_two_decode(int u_int_code_index, BitInputStream * bis, CompressionParameters * cp) {		
	int threshold = 0;
	do {
		char bit = read_bit(bis);
		if (bit)
			break;
		threshold++;
	} while (threshold < cp->u_max);
	
	if (threshold == cp->u_max) {
		int res = (int) read_bits(bis, cp->depth);
		return res;
	} else {
		return (threshold << u_int_code_index) | (int) read_bits(bis, u_int_code_index);
	}
}

int reverse_length_limited_golomb_power_of_two_decode(int u_int_code_index, BitInputStream * bis, CompressionParameters * cp) {		
	int threshold = 0;
	do {
		char bit = read_bit(bis);
		if (bit)
			break;
		threshold++;
	} while (threshold < cp->u_max);
	
	if (threshold == cp->u_max) {
		int res = (int) reverse_read_bits(bis, cp->depth);
		return res;
	} else {
		return (threshold << u_int_code_index) | (int) reverse_read_bits(bis, u_int_code_index);
	}
}	
	
int get_u_int_code_index(int b, int c_value, CompressionParameters * cp) {
	int u_int_code_index;
	if (2*c_value > cp->accumulator[b] + ((49*c_value) >> 7)) {
		u_int_code_index = 0;
	} else {
		u_int_code_index = 0;
		while (c_value << (u_int_code_index + 1) <= cp->accumulator[b] + ((49*c_value) >> 7)) {
			u_int_code_index++;
		}
		u_int_code_index = min(u_int_code_index, cp->depth-2);
	}
	return u_int_code_index;
}
	
void update_accumulator(int b, int mapped_quantizer_index, int c_value, CompressionParameters * cp) {
	cp->accumulator[b] += mapped_quantizer_index;
	if (c_value == (1 << cp->gamma_star) - 1) {
		cp->accumulator[b] = (cp->accumulator[b] + 1) >> 1;
	}
}
	
void code(int mapped_quantizer_index, int t, int b, BitOutputStream * bos, CompressionParameters * cp, Checker * checker) {
	if (t == 0) {
		write_bits(bos, mapped_quantizer_index, cp->depth);
	} else {
		int c_value 			= get_counter_value(t, cp);
		int u_int_code_index 	= get_u_int_code_index(b, c_value, cp);
		int u_int_val 			= mapped_quantizer_index;
		//code
		length_limited_golomb_power_of_two_code(u_int_val, u_int_code_index, bos, cp);
		update_accumulator(b, mapped_quantizer_index, c_value, cp);

		#ifdef CHECK_VALUES
			addi(checker, c_value);
			addi(checker, u_int_code_index);
			addi(checker, u_int_val);
		#endif
	}
}

int decode(int t, int b, BitInputStream * bis, CompressionParameters * cp, Checker * checker) {
	if (t == 0) {
		int res = read_bits(bis, cp->depth);
		return res;
	} else {
		int c_value 			= get_counter_value(t, cp);
		int u_int_code_index 	= get_u_int_code_index(b, c_value, cp);
		int u_int_val 			= length_limited_golomb_power_of_two_decode(u_int_code_index, bis, cp);
		//update accumulator
		update_accumulator(b, u_int_val, c_value, cp);

		#ifdef CHECK_VALUES
			chki(set_message(checker, "CVAL"), c_value);
			chki(set_message(checker, "UICI"), u_int_code_index);
			chki(set_message(checker, "UIV"), u_int_val);
		#endif
		return u_int_val;
	}
}
	


int get_k(long counter, long accumulator, CompressionParameters * cp) {
	int k = 1;
	int kMax = max(cp->depth - 2, 2);
	while (counter*(1l << (k+1+2)) <= accumulator + ((49*counter) >> 5) && k < kMax)
		k++;
	
	return min(k, kMax);
}

int get_code_index(long acc, long counter) {
	int code_index = 0;
	for (int i = 0; i < 16; i++) {
		if (acc<<14 < (long) counter * (long) THRESHOLD[i])
			code_index = i;
		else
			break;
	}
	return code_index;
}

void code_hybrid(int mapped_quantizer_index, int t, int b, BitOutputStream * bos, CompressionParameters * cp, Checker * checker) {
    //generate counter for current iteration
    long counter_t = get_counter_value(t, cp);
    long counter_t_p_1 = get_counter_value(t+1, cp);
    long acc_t;
    //debug(mapped_quantizer_index, this.cp.depth, "Coding mqi");

    if (t == 0) {
        //code raw mqi value
		write_bits(bos, mapped_quantizer_index, cp->depth);
        acc_t = cp->accumulator[b];
    } else {
        //output last acc bit if we are losing it
        int flush_bit = (int) (cp->accumulator[b] & 0x1);
        if (counter_t == ((1l<<cp->gamma_star) - 1)) {
			write_bit(bos, flush_bit);
        }
        
        //update accumulator for current iteration
		update_accumulator(b, mapped_quantizer_index, counter_t, cp);
		acc_t = cp->accumulator[b];

        bool is_high_entropy = acc_t<<14 >= (long) THRESHOLD[0] * counter_t_p_1;
        int k = get_k(counter_t_p_1, acc_t, cp);
        int code_index = get_code_index(acc_t, counter_t_p_1);
        int input_symbol = mapped_quantizer_index <= INPUTSYMBOLLIMIT[code_index] ? mapped_quantizer_index : CODE_X_VAL;
        int code_quant = mapped_quantizer_index - INPUTSYMBOLLIMIT[code_index] - 1;
		
		TreeTable * current_table = cp->active_tables[code_index];
		TreeTable * entry = current_table->children[input_symbol];

        bool is_tree = treetable_is_tree(entry);
		TreeTable * next_table;
		next_table = is_tree ? entry : BASETABLES[code_index];
        
		CodeWord * code_word = (CodeWord *) entry->object;
        int cw_value = code_word->cw_value;
        int cw_bits = code_word->cw_bits;


        //perform high or low entropy coding
        if (is_high_entropy) {
            //high entropy
			reverse_length_limited_golomb_power_of_two_code(mapped_quantizer_index, k, bos, cp);
        } else {
            //low entropy
            if (input_symbol == CODE_X_VAL) {
				reverse_length_limited_golomb_power_of_two_code(code_quant, 0, bos, cp);
            }
            if (!is_tree) { 	//output final code and reset table
				write_bits(bos, cw_value, cw_bits);
            }
			cp->active_tables[code_index] = next_table;
        }
    }
    
    if (t == cp->samples_per_band - 1 && b == cp->bands - 1) {//last sample, flush things
        //flush all active tables with their flush codes
        for (int i = 0; i < 16; i++) {
			CodeWord * code_word = (CodeWord *) cp->active_tables[i]->object;
			write_bits(bos, code_word->cw_value, code_word->cw_bits);
        }
        //flush accumulators
        for (int i = 0; i < cp->bands; i++) {
			write_bits(bos, cp->accumulator[i], 2 + cp->depth + cp->gamma_star);
        }
		write_bit(bos, 1l);
    }
}


int decode_hybrid(int t, int b, BitInputStream * bis, CompressionParameters * cp, Checker * checker, int ** decoded_mqi) {
	if (t == 0 && b == 0)
		*decoded_mqi = fully_decode_hybrid(bis, cp);
	return (*decoded_mqi)[b*cp->samples_per_band + t];
}
	
int * fully_decode_hybrid(BitInputStream * bis, CompressionParameters * cp) {
	reverse_bis(bis); //this puts it from end to beginning, flipping bits

	//initialize decoding things
	int * decoded_mqi = calloc(cp->samples_per_image, sizeof(int));
	
	//first read until the first one
	while (!read_bit(bis));
	//debug(0, 1, "Read input padding");
	//debug(1, 1, "Read end of input padding");
	//read the accumulators
	for (int i = cp->bands - 1; i >= 0; i--) {
		cp->accumulator[i] = read_bits(bis, 2 + cp->depth + cp->gamma_star);
	}
	//read the flush tables
	for (int i = 15; i >= 0; i--) {
		TreeTable * rft = REVERSEFLUSHTABLES[i];
		while (treetable_is_tree(rft)) {
			rft = rft->children[(int) read_bit(bis)];
		}
		cp->active_tables[i] = (TreeTable * ) rft->object;
	}
	
	//now we invert the coding operation (always BIP mode)
	for (int t = cp->samples_per_band - 1; t >= 0; t--) {
		for (int b = cp->bands - 1; b >= 0; b--) {
			//generate counter for current iteration
			long counter_t = get_counter_value(t, cp);
			long counter_t_p_1 = get_counter_value(t+1, cp);
			long acc_t = cp->accumulator[b];

			int mqi;
			if (t > 0) { //reverse accumulator calculation for next iteration
				//perform high or low entropy decoding
				if (acc_t*(1l<<14) >= (long) THRESHOLD[0] * counter_t_p_1) {
					//was coded on high entropy
					int k = get_k(counter_t_p_1, acc_t, cp);
					mqi = reverse_length_limited_golomb_power_of_two_decode(k, bis, cp);
					//debug(mqi, k, "Read high entropy mqi (" + accT + "," + counterTp1 + ")");
				} else {
					//low entropy
					int code_index = get_code_index(acc_t, counter_t_p_1);
					//if current table is root, we need to read a new table
					if (treetable_is_root(cp->active_tables[code_index])) {
						TreeTable * rt = REVERSETABLES[code_index];
						while (treetable_is_tree(rt)) {
							rt = rt->children[(int) read_bit(bis)];
						}
						cp->active_tables[code_index] = (TreeTable *) rt->object;
					}
					//get symbol and update current table
					int input_symbol = cp->active_tables[code_index]->parent_index;
					cp->active_tables[code_index] = cp->active_tables[code_index]->parent; 
					if (input_symbol == CODE_X_VAL) {
						int difference = reverse_length_limited_golomb_power_of_two_decode(0, bis, cp);
						mqi = difference + INPUTSYMBOLLIMIT[code_index] + 1;
					} else { //inputSymbol is mqi
						mqi = input_symbol;
					}	
				}
				

				//update accumulator for previous iteration
				reverseUpdateAcc(b, mqi, (int) counter_t, cp);
				//recover lost bit if renormalized
				if (counter_t == ((1l<<cp->gamma_star) - 1)) {
					if (!read_bit(bis)) //if bit is zero
						cp->accumulator[b] += 1;
				}
				
			} else { //raw value is encoded
				mqi = reverse_read_bits(bis, cp->depth);
			}
			decoded_mqi[b*cp->samples_per_band + t] = mqi;
		}
	}

	return decoded_mqi;
}

void reverseUpdateAcc(int b, int mqi, int counter, CompressionParameters * cp) {
	if (counter < ((1<<cp->gamma_star) - 1)) {
		cp->accumulator[b] = cp->accumulator[b] - 4*mqi;
	} else {
		cp->accumulator[b] = cp->accumulator[b]*2 - 4*mqi - 1;
	}
}










////
//QUANTIZATION
////

long quantize(long value, long parameter) {
	long sign = signum(value);
	long magnitude = (absl(value) + parameter) / (2*parameter + 1);
	return sign*magnitude;
}


long dequantize(long qvalue, long parameter) {
	long sign = signum(qvalue);
	qvalue = absl(qvalue) * (2*parameter + 1) - parameter;
	return qvalue*sign;
}




////
//UTILITIES
////
long min(long a, long b) {
	return a > b ? b : a;
}

long max(long a, long b) {
	return a > b ? a : b;
}

long absl(long a) {
	return a > 0 ? a : -a;
}

long clamp(long value, long min, long max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

int clampi(int value, int min, int max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}


long signum(long val) {
	if (val > 0)
		return 1;
	else if (val == 0)
		return 0;
	return -1;
}

int signum_plus(int val) {
	if (val >= 0)
		return 1;
	return -1;
}

long mod_R(long value, int r) {
	if (r == 64)
		return value;
	
	long offset = 1 << (r - 1);
	long modulus = 1 << r;
	return ((value + offset) % modulus) - offset;
}

long minus_one_to_the(long value) {
	if (value % 2 == 0)
		return 1;
	return -1;
}

