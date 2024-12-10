#ifndef ALGORITHM_H_
#define ALGORITHM_H_
#define true 1
#define false 0
#define FS 100
#define BUFFER_SIZE (FS * 5)
//#define BUFFER_SIZE (FS* 2)
#define HR_FIFO_SIZE 7
#define MA4_SIZE 4	   // DO NOT CHANGE
#define HAMMING_SIZE 5 // DO NOT CHANGE
#define min(x, y) ((x) < (y) ? (x) : (y))

void maxim_heart_rate_and_oxygen_saturation(unsigned int *pun_ir_buffer, int n_ir_buffer_length,
	unsigned int *pun_red_buffer, int *pn_spo2, int *pch_spo2_valid, int *pn_heart_rate,
	int *pch_hr_valid);
void maxim_find_peaks(int *pn_locs, int *pn_npks, int *pn_x, int n_size, int n_min_height,
	int n_min_distance, int n_max_num);
void maxim_peaks_above_min_height(
	int *pn_locs, int *pn_npks, int *pn_x, int n_size, int n_min_height);
void maxim_remove_close_peaks(int *pn_locs, int *pn_npks, int *pn_x, int n_min_distance);
void maxim_sort_ascend(int *pn_x, int n_size);
void maxim_sort_indices_descend(int *pn_x, int *pn_indx, int n_size);
#endif /* ALGORITHM_H_ */