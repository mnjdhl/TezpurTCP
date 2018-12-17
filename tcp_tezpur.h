/*TCP Tezpur Functions; Author: Manoj Dahal; Dated:05/10/2004 */

//Insert/change a segment seq no. with sent time;12/09/2004
extern struct sentseg* tezpur_ins_seg(struct sentseg *segs,__u32 seq,__u32 bytes,long stime) ;
//extern struct sentseg* tezpur_ins_seg(struct sentseg *segs,__u32 seq,long stime) ;
extern void tezpur_disp_segs(struct sentseg *segs);
//Delete a segment
extern struct sentseg* tezpur_del_segs(struct sentseg* tmpseg,__u32 seq); 
extern struct sentseg* tezpur_del_segs_le(struct sentseg* tmpseg,__u32 seq); 
extern struct sentseg * tezpur_del_segs_lt(struct sentseg * tmpseg,__u32 seq);
extern long tezpur_find_seg(struct sentseg* tmpseg,__u32 seq,__u32 * bytes);
//extern __u32 tezpur_find_seg(struct sentseg* tmpseg,__u32 seq);
extern long tezpur_find_seg_lt(struct sentseg * tmpseg,__u32 seq,__u32 * bytes);
//extern __u32 tezpur_find_seg_lt(struct sentseg * tmpseg,__u32 seq,__u32 * bytes);
/* Returns the current time in second +(microsecond/1000000.0) 
   11/09/2004 */

extern long tezpur_now(void);
//extern double tezpur_now(struct tcp_opt *tp);
extern void tezpur_init(struct tcp_opt *tp);
extern void tezpur_update_rtt(struct tcp_opt *tp,__u32 seq);
/* To distinguish loss due to corruption and congestion 
 * 02/06/2004 */
extern __u8 tezpur_congested(struct tcp_opt *tp);
extern double tezpur_get_cwnd_incr(struct tcp_opt *tp);
/* Previously as void tz_opencwnd(struct sock *sk) */
extern void tezpur_cong_avoid(struct tcp_opt *tp);
extern void tezpur_threshold_rtt_estimator(struct tcp_opt *tp);
/*End of TCP Tezpur Functions*/
