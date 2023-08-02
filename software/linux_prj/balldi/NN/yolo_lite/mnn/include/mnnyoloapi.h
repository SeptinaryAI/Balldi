/**
origin:https://github.com/ppogg/YOLOv5-Lite

modify it for Balldi project

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**/
#ifndef MNNYOLOAPI_H
#define MNNYOLOAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#define RS_STAT_SUCC           0
#define RS_STAT_ERR            1
#define RS_STAT_GIVEUP         10

#define FRAME_MAX_LEN   100 * 1024      //100K
#define RST_MAX_LEN     20              //max 20 result of reasoning

/* build mnnyoloapi use */
typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
    int label;
    int id;
    float score;
} t_result;

typedef struct {
    t_result results[RST_MAX_LEN];
} t_rslts;

typedef void (*fp_init_mnn_yolo)(void);
typedef void (*fp_close_mnn_yolo)(void);
typedef void (*fp_get_result_by_jpg_raw)(t_result *arr_rst, int *rst_len, unsigned char *jpg_raw, int jpg_len);

//callback
void init_mnn_yolo(void);
void close_mnn_yolo(void);
void get_result_by_jpg_raw(t_result *arr_rst, int *rst_len, unsigned char *jpg_raw, int jpg_len);

void set_reason_run_flg(int val);
int get_reason_run_flg(void);
void set_share_frame(unsigned char *addr, int len);

//usr api
int get_frame_reason(unsigned char **pp, int *len, t_rslts *rslts, int *rst_len);
void get_frame_reason_finish(void);

int cgo_get_x1(t_rslts *rslts, unsigned int index);
int cgo_get_y1(t_rslts *rslts, unsigned int index);
int cgo_get_x2(t_rslts *rslts, unsigned int index);
int cgo_get_y2(t_rslts *rslts, unsigned int index);
int cgo_get_label(t_rslts *rslts, unsigned int index);
int cgo_get_id(t_rslts *rslts, unsigned int index);
float cgo_get_score(t_rslts *rslts, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif
