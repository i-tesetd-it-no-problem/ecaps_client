#include "app/algorithm.h"

const unsigned short auw_hamm[31] = { 41, 276, 512, 276, 41 };
const unsigned char uch_spo2_table[184] = { 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98,
	98, 98, 99, 99, 99, 99, 99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	100, 100, 100, 100, 100, 100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98,
	98, 97, 97, 97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 90,
	90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 80, 80, 79, 78, 78,
	77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 66, 66, 65, 64, 63, 62, 62, 61, 60,
	59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37,
	36, 35, 34, 33, 31, 30, 29, 28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9,
	7, 6, 5, 3, 2, 1 };
static int an_dx[BUFFER_SIZE - MA4_SIZE];
static int an_x[BUFFER_SIZE];
static int an_y[BUFFER_SIZE];

void maxim_heart_rate_and_oxygen_saturation(unsigned int *pun_ir_buffer, int n_ir_buffer_length,
	unsigned int *pun_red_buffer, int *pn_spo2, int *pch_spo2_valid, int *pn_heart_rate,
	int *pch_hr_valid)
{
#if 1
	unsigned int un_ir_mean, un_only_once;
	int k, n_i_ratio_count;
	int i, s, m, n_exact_ir_valley_locs_count, n_middle_idx;
	int n_th1, n_npks, n_c_min;
	int an_ir_valley_locs[15];
	int an_exact_ir_valley_locs[15];
	int an_dx_peak_locs[15];
	int n_peak_interval_sum;
	int n_y_ac, n_x_ac;
	int n_spo2_calc;
	int n_y_dc_max, n_x_dc_max;
	int n_y_dc_max_idx, n_x_dc_max_idx;
	int an_ratio[5], n_ratio_average;
	int n_nume, n_denom;

	un_ir_mean = 0;
	for (k = 0; k < n_ir_buffer_length; k++)
		un_ir_mean += pun_ir_buffer[k];

	un_ir_mean = un_ir_mean / n_ir_buffer_length;

	for (k = 0; k < n_ir_buffer_length; k++) {
		an_x[k] = pun_ir_buffer[k] - un_ir_mean;
	}

	for (k = 0; k < BUFFER_SIZE - MA4_SIZE; k++) {
		n_denom = (an_x[k] + an_x[k + 1] + an_x[k + 2] + an_x[k + 3]);
		an_x[k] = n_denom / 4;
	}

	for (k = 0; k < BUFFER_SIZE - MA4_SIZE - 1; k++) {
		an_dx[k] = (an_x[k + 1] - an_x[k]);
	}

	for (k = 0; k < BUFFER_SIZE - MA4_SIZE - 2; k++) {
		an_dx[k] = (an_dx[k] + an_dx[k + 1]) / 2;
	}

	for (i = 0; i < BUFFER_SIZE - HAMMING_SIZE - MA4_SIZE - 2; i++) {
		s = 0;
		for (k = i; k < i + HAMMING_SIZE; k++) {
			s -= an_dx[k] * auw_hamm[k - i];
		}
		an_dx[i] = s / 1146;
	}

	n_th1 = 0;
	for (k = 0; k < BUFFER_SIZE - HAMMING_SIZE; k++) {
		n_th1 += (an_dx[k] > 0) ? an_dx[k] : (-an_dx[k]);
	}
	n_th1 = n_th1 / (BUFFER_SIZE - HAMMING_SIZE);

	maxim_find_peaks(an_dx_peak_locs, &n_npks, an_dx, BUFFER_SIZE - HAMMING_SIZE, n_th1, 8, 5);

	n_peak_interval_sum = 0;
	if (n_npks >= 2) {
		for (k = 1; k < n_npks; k++)
			n_peak_interval_sum += (an_dx_peak_locs[k] - an_dx_peak_locs[k - 1]);
		n_peak_interval_sum = n_peak_interval_sum / (n_npks - 1);
		*pn_heart_rate = (int)(6000 / n_peak_interval_sum);
		*pch_hr_valid = 1;
	} else {
		*pn_heart_rate = -999;
		*pch_hr_valid = 0;
	}

	for (k = 0; k < n_npks; k++)
		an_ir_valley_locs[k] = an_dx_peak_locs[k] + HAMMING_SIZE / 2;

	for (k = 0; k < n_ir_buffer_length; k++) {
		an_x[k] = pun_ir_buffer[k];
		an_y[k] = pun_red_buffer[k];
	}

	n_exact_ir_valley_locs_count = 0;
	for (k = 0; k < n_npks; k++) {
		un_only_once = 1;
		m = an_ir_valley_locs[k];
		n_c_min = 16777216;
		if (m + 5 < BUFFER_SIZE - HAMMING_SIZE && m - 5 > 0) {
			for (i = m - 5; i < m + 5; i++)
				if (an_x[i] < n_c_min) {
					if (un_only_once > 0) {
						un_only_once = 0;
					}
					n_c_min = an_x[i];
					an_exact_ir_valley_locs[k] = i;
				}
			if (un_only_once == 0)
				n_exact_ir_valley_locs_count++;
		}
	}
	if (n_exact_ir_valley_locs_count < 2) {
		*pn_spo2 = -999;
		*pch_spo2_valid = 0;
		return;
	}

	for (k = 0; k < BUFFER_SIZE - MA4_SIZE; k++) {
		an_x[k] = (an_x[k] + an_x[k + 1] + an_x[k + 2] + an_x[k + 3]) / 4;
		an_y[k] = (an_y[k] + an_y[k + 1] + an_y[k + 2] + an_y[k + 3]) / 4;
	}

	n_ratio_average = 0;
	n_i_ratio_count = 0;

	for (k = 0; k < 5; k++)
		an_ratio[k] = 0;
	for (k = 0; k < n_exact_ir_valley_locs_count; k++) {
		if (an_exact_ir_valley_locs[k] > BUFFER_SIZE) {
			*pn_spo2 = -999;
			*pch_spo2_valid = 0;
			return;
		}
	}

	for (k = 0; k < n_exact_ir_valley_locs_count - 1; k++) {
		n_y_dc_max = -16777216;
		n_x_dc_max = -16777216;
		if (an_exact_ir_valley_locs[k + 1] - an_exact_ir_valley_locs[k] > 10) {
			for (i = an_exact_ir_valley_locs[k]; i < an_exact_ir_valley_locs[k + 1]; i++) {
				if (an_x[i] > n_x_dc_max) {
					n_x_dc_max = an_x[i];
					n_x_dc_max_idx = i;
				}
				if (an_y[i] > n_y_dc_max) {
					n_y_dc_max = an_y[i];
					n_y_dc_max_idx = i;
				}
			}
			n_y_ac = (an_y[an_exact_ir_valley_locs[k + 1]] - an_y[an_exact_ir_valley_locs[k]]) *
				(n_y_dc_max_idx - an_exact_ir_valley_locs[k]);
			n_y_ac = an_y[an_exact_ir_valley_locs[k]] +
				n_y_ac / (an_exact_ir_valley_locs[k + 1] - an_exact_ir_valley_locs[k]);

			n_y_ac = an_y[n_y_dc_max_idx] - n_y_ac;
			n_x_ac = (an_x[an_exact_ir_valley_locs[k + 1]] - an_x[an_exact_ir_valley_locs[k]]) *
				(n_x_dc_max_idx - an_exact_ir_valley_locs[k]);
			n_x_ac = an_x[an_exact_ir_valley_locs[k]] +
				n_x_ac / (an_exact_ir_valley_locs[k + 1] - an_exact_ir_valley_locs[k]);
			n_x_ac = an_x[n_y_dc_max_idx] - n_x_ac;
			n_nume = (n_y_ac * n_x_dc_max) >> 7;
			n_denom = (n_x_ac * n_y_dc_max) >> 7;
			if (n_denom > 0 && n_i_ratio_count < 5 && n_nume != 0) {
				an_ratio[n_i_ratio_count] = (n_nume * 100) / n_denom;
				n_i_ratio_count++;
			}
		}
	}

	maxim_sort_ascend(an_ratio, n_i_ratio_count);
	n_middle_idx = n_i_ratio_count / 2;

	if (n_middle_idx > 1)
		n_ratio_average = (an_ratio[n_middle_idx - 1] + an_ratio[n_middle_idx]) / 2;
	else
		n_ratio_average = an_ratio[n_middle_idx];

	if (n_ratio_average > 2 && n_ratio_average < 184) {
		n_spo2_calc = uch_spo2_table[n_ratio_average];
		*pn_spo2 = n_spo2_calc;
		*pch_spo2_valid = 1;
	} else {
		*pn_spo2 = -999;
		*pch_spo2_valid = 0;
	}
#else
	unsigned int un_ir_mean, un_only_once;
	int k, n_i_ratio_count;
	int i, s, m, n_exact_ir_valley_locs_count, n_middle_idx;
	int n_th1, n_npks, n_c_min;
	int an_ir_valley_locs[15];
	int n_peak_interval_sum;

	int n_y_ac, n_x_ac;
	int n_spo2_calc;
	int n_y_dc_max, n_x_dc_max;
	int n_y_dc_max_idx, n_x_dc_max_idx;
	int an_ratio[5], n_ratio_average;
	int n_nume, n_denom;

	un_ir_mean = 0;
	for (k = 0; k < n_ir_buffer_length; k++)
		un_ir_mean += pun_ir_buffer[k];
	un_ir_mean = un_ir_mean / n_ir_buffer_length;

	for (k = 0; k < n_ir_buffer_length; k++)
		an_x[k] = -1 * (pun_ir_buffer[k] - un_ir_mean);

	for (k = 0; k < n_ir_buffer_length - MA4_SIZE; k++) {
		an_x[k] = (an_x[k] + an_x[k + 1] + an_x[k + 2] + an_x[k + 3]) / 4;
	}

	n_th1 = 0;
	for (k = 0; k < n_ir_buffer_length; k++) {
		n_th1 += an_x[k];
	}
	n_th1 = n_th1 / n_ir_buffer_length;
	if (n_th1 < 30)
		n_th1 = 30;
	if (n_th1 > 60)
		n_th1 = 60;

	for (k = 0; k < 15; k++)
		an_ir_valley_locs[k] = 0;

	maxim_find_peaks(an_ir_valley_locs, &n_npks, an_x, n_ir_buffer_length, n_th1, 4, 15);

	n_peak_interval_sum = 0;
	if (n_npks >= 2) {
		for (k = 1; k < n_npks; k++)
			n_peak_interval_sum += (an_ir_valley_locs[k] - an_ir_valley_locs[k - 1]);
		n_peak_interval_sum = n_peak_interval_sum / (n_npks - 1);
		*pn_heart_rate = (int)((FS * 60) / n_peak_interval_sum);
		*pch_hr_valid = 1;
	} else {
		*pn_heart_rate = -999;
		*pch_hr_valid = 0;
	}

	for (k = 0; k < n_ir_buffer_length; k++) {
		an_x[k] = pun_ir_buffer[k];
		an_y[k] = pun_red_buffer[k];
	}

	n_exact_ir_valley_locs_count = n_npks;

	n_ratio_average = 0;
	n_i_ratio_count = 0;
	for (k = 0; k < 5; k++)
		an_ratio[k] = 0;
	for (k = 0; k < n_exact_ir_valley_locs_count; k++) {
		if (an_ir_valley_locs[k] > n_ir_buffer_length) {
			*pn_spo2 = -999;
			*pch_spo2_valid = 0;
			return;
		}
	}

	for (k = 0; k < n_exact_ir_valley_locs_count - 1; k++) {
		n_y_dc_max = -16777216;
		n_x_dc_max = -16777216;
		if (an_ir_valley_locs[k + 1] - an_ir_valley_locs[k] > 3) {
			for (i = an_ir_valley_locs[k]; i < an_ir_valley_locs[k + 1]; i++) {
				if (an_x[i] > n_x_dc_max) {
					n_x_dc_max = an_x[i];
					n_x_dc_max_idx = i;
				}
				if (an_y[i] > n_y_dc_max) {
					n_y_dc_max = an_y[i];
					n_y_dc_max_idx = i;
				}
			}
			n_y_ac = (an_y[an_ir_valley_locs[k + 1]] - an_y[an_ir_valley_locs[k]]) *
				(n_y_dc_max_idx - an_ir_valley_locs[k]);
			n_y_ac = an_y[an_ir_valley_locs[k]] +
				n_y_ac / (an_ir_valley_locs[k + 1] - an_ir_valley_locs[k]);

			n_y_ac = an_y[n_y_dc_max_idx] - n_y_ac;
			n_x_ac = (an_x[an_ir_valley_locs[k + 1]] - an_x[an_ir_valley_locs[k]]) *
				(n_x_dc_max_idx - an_ir_valley_locs[k]);
			n_x_ac = an_x[an_ir_valley_locs[k]] +
				n_x_ac / (an_ir_valley_locs[k + 1] - an_ir_valley_locs[k]);
			n_x_ac = an_x[n_y_dc_max_idx] - n_x_ac;
			n_nume = (n_y_ac * n_x_dc_max) >> 7;
			n_denom = (n_x_ac * n_y_dc_max) >> 7;
			if (n_denom > 0 && n_i_ratio_count < 5 && n_nume != 0) {
				an_ratio[n_i_ratio_count] = (n_nume * 100) / n_denom;
				n_i_ratio_count++;
			}
		}
	}

	maxim_sort_ascend(an_ratio, n_i_ratio_count);
	n_middle_idx = n_i_ratio_count / 2;

	if (n_middle_idx > 1)
		n_ratio_average = (an_ratio[n_middle_idx - 1] + an_ratio[n_middle_idx]) / 2;
	else
		n_ratio_average = an_ratio[n_middle_idx];

	if (n_ratio_average > 2 && n_ratio_average < 184) {
		n_spo2_calc = uch_spo2_table[n_ratio_average];
		*pn_spo2 = n_spo2_calc;
		*pch_spo2_valid = 1;
	} else {
		*pn_spo2 = -999;
		*pch_spo2_valid = 0;
	}
#endif
}

void maxim_find_peaks(int *pn_locs, int *pn_npks, int *pn_x, int n_size, int n_min_height,
	int n_min_distance, int n_max_num)
{
	maxim_peaks_above_min_height(pn_locs, pn_npks, pn_x, n_size, n_min_height);
	maxim_remove_close_peaks(pn_locs, pn_npks, pn_x, n_min_distance);
	*pn_npks = min(*pn_npks, n_max_num);
}

void maxim_peaks_above_min_height(
	int *pn_locs, int *pn_npks, int *pn_x, int n_size, int n_min_height)
{
	int i = 1, n_width;
	*pn_npks = 0;

	while (i < n_size - 1) {
		if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i - 1]) {
			n_width = 1;

			while (i + n_width < n_size && pn_x[i] == pn_x[i + n_width])
				n_width++;
			if (pn_x[i] > pn_x[i + n_width] && (*pn_npks) < 15) {
				pn_locs[(*pn_npks)++] = i;

				i += n_width + 1;
			} else
				i += n_width;
		} else
			i++;
	}
}

void maxim_remove_close_peaks(int *pn_locs, int *pn_npks, int *pn_x, int n_min_distance)
{
	int i, j, n_old_npks, n_dist;
	/* 按从大到小的顺序排序峰值 */
	maxim_sort_indices_descend(pn_x, pn_locs, *pn_npks);
	for (i = -1; i < *pn_npks; i++) {
		n_old_npks = *pn_npks;
		*pn_npks = i + 1;
		for (j = i + 1; j < n_old_npks; j++) {
			n_dist = pn_locs[j] - (i == -1 ? -1 : pn_locs[i]);
			if (n_dist > n_min_distance || n_dist < -n_min_distance)
				pn_locs[(*pn_npks)++] = pn_locs[j];
		}
	}

	maxim_sort_ascend(pn_locs, *pn_npks);
}

void maxim_sort_ascend(int *pn_x, int n_size)
{
	int i, j, n_temp;
	for (i = 1; i < n_size; i++) {
		n_temp = pn_x[i];
		for (j = i; j > 0 && n_temp < pn_x[j - 1]; j--)
			pn_x[j] = pn_x[j - 1];
		pn_x[j] = n_temp;
	}
}

void maxim_sort_indices_descend(int *pn_x, int *pn_indx, int n_size)
{
	int i, j, n_temp;
	for (i = 1; i < n_size; i++) {
		n_temp = pn_indx[i];
		for (j = i; j > 0 && pn_x[n_temp] > pn_x[pn_indx[j - 1]]; j--)
			pn_indx[j] = pn_indx[j - 1];
		pn_indx[j] = n_temp;
	}
}
