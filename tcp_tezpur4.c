//#include <net/tcp_tezpur.h>
#include <net/tcp.h>
#include <linux/tcp.h>
#include <linux/time.h>
//#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <stdarg.h>
#define TEZPUR_RTO_MIN (TCP_RTO_MIN*1000)

/*TCP Tezpur Functions; Author: Manoj Dahal; Dated:05/10/2004 */

//Insert/change a segment seq no. with sent time;17/11/2004
struct sentseg* tezpur_ins_seg(struct sentseg *segs,__u32 seq,long stime) 
{ 

    struct sentseg *tmpseg=segs;

	//printk(KERN_ALERT "\nTCP Tezpur Inserting a Sent Segment!");

    if (sysctl_tcp_tezpur)
      {

       if (segs == NULL) {
	   	segs = (struct sentseg*)kmalloc(sizeof(struct sentseg),GFP_ATOMIC);
	   	//segs = (struct sentseg*)kmalloc(sizeof(struct sentseg),GFP_KERNEL);
		if (segs == NULL) {
                  printk(KERN_ALERT "\n TCP Tezpur: Memory Allocation Error!");  
		  return segs;	
		}
   		segs->seqno= seq;
   		segs->sent_time = stime;
		segs->next = NULL;
		return segs;
       }
       else {
          	while (segs->next != NULL) {
       	     	     if (segs->seqno == seq) {
   			segs->sent_time = stime;
			return tmpseg;
                      }
			segs = segs->next;
		}

       	     	if (segs->seqno == seq) {
   		  segs->sent_time = stime;
	 	  return tmpseg;
                }

	   	segs->next = (struct sentseg*)kmalloc(sizeof(struct sentseg),GFP_ATOMIC);
	   	//segs->next = (struct sentseg*)kmalloc(sizeof(struct sentseg),GFP_KERNEL);
		if (segs->next == NULL) {
                  printk(KERN_ALERT "\n TCP Tezpur: Memory Allocation Error!");  
		  return tmpseg;
		}
       	     	segs = segs->next;
   		segs->seqno= seq;
   		segs->sent_time = stime;
		segs->next = NULL;
		return tmpseg;

       }
      }
	return tmpseg;

 }

void tezpur_disp_segs(struct sentseg *segs) {
 
    struct sentseg* tmpseg;

	for (tmpseg=segs;tmpseg!=NULL;tmpseg=tmpseg->next){

	     printk(KERN_ALERT "\nSequence No.=%u, Sent At=%u!",tmpseg->seqno,(__u32)tmpseg->sent_time);
	}
}

//Delete a segment
struct sentseg * tezpur_del_segs(struct sentseg * tmpseg,__u32 seq) {
	struct sentseg * tmpseg1, *tmpseg2;

      if (tmpseg != NULL) {
	tmpseg1=NULL;
	tmpseg2 = tmpseg;
	while (tmpseg2!=NULL) {
           
   	    if (tmpseg2->seqno==seq){
		        if (tmpseg1 == NULL)
			   tmpseg = tmpseg2->next;
			else
			   tmpseg1->next = tmpseg2->next;
			kfree(tmpseg2);
	         break;
          	}

		tmpseg1=tmpseg2;
		tmpseg2 = tmpseg2->next;
      }
     }
      return tmpseg;

}
//Delete segments whose seq # is less than equal to the given one
//17/11/2004
struct sentseg * tezpur_del_segs_le(struct sentseg * tmpseg,__u32 seq) {
	struct sentseg * tmpseg1, *tmpseg2;

      if (tmpseg != NULL) {
	tmpseg1=NULL;      
	tmpseg2=tmpseg;
	while (tmpseg2!=NULL) {
           
   	    if (tmpseg2->seqno<=seq){
		        if (tmpseg1 == NULL){
			  tmpseg = tmpseg2->next;
			  kfree(tmpseg2);
			  tmpseg2=tmpseg;
			}
			else {
			  tmpseg1->next = tmpseg2->next;
			  kfree(tmpseg2);
			  tmpseg2 = tmpseg1->next;
			}
          	}
	    else {
		tmpseg1=tmpseg2;
		tmpseg2 = tmpseg2->next;
	    }
	}
     }
      return tmpseg;

}

//Delete segments whose seq # is less than(lt) the given one
//02/12/2004
struct sentseg * tezpur_del_segs_lt(struct sentseg * tmpseg,__u32 seq) {
	struct sentseg * tmpseg1, *tmpseg2;

      if (tmpseg != NULL) {
	tmpseg1=NULL;      
	tmpseg2=tmpseg;
	while (tmpseg2!=NULL) {
           
   	    if (tmpseg2->seqno<seq){
		        if (tmpseg1 == NULL){
			  tmpseg = tmpseg2->next;
			  kfree(tmpseg2);
			  tmpseg2=tmpseg;
			}
			else {
			  tmpseg1->next = tmpseg2->next;
			  kfree(tmpseg2);
			  tmpseg2 = tmpseg1->next;
			}
          	}
	    else {
		tmpseg1=tmpseg2;
		tmpseg2 = tmpseg2->next;
	    }
	}
     }
      return tmpseg;

}
long tezpur_find_seg(struct sentseg * tmpseg,__u32 seq) {
	long tmp=-1;
	struct sentseg *tmpseg1=tmpseg;

	while (tmpseg1!=NULL) {
   	   if (tmpseg1->seqno==seq) {
	       	tmp =tmpseg1->sent_time;
	        break;
	   }
	  tmpseg1 = tmpseg1->next;
      }

      return tmp;
}
/* Find a sequence number which is less than(lt) the given one and the smallest;02/12/2004 */
long tezpur_find_seg_lt(struct sentseg * tmpseg,__u32 seq,__u32 * bytes) {
	long tmp=-1;
	struct sentseg *tmpseg1=tmpseg;

	while (tmpseg1!=NULL) {
   	   if (tmpseg1->seqno<seq) {
	       	tmp =tmpseg1->sent_time;
		*bytes=seq-tmpseg1->seqno;
	        break;
	   }
	  tmpseg1 = tmpseg1->next;
      }

      return tmp;
}



/* Returns the current time in (seconds*1000000) +microseconds 
   11/09/2004 */

long tezpur_now(void)
{
 struct timeval tv;
 
   do_gettimeofday(&tv);

   return ((tv.tv_sec*1000000)+tv.tv_usec) ;
   //return (tv.tv_sec+(tv.tv_usec/1000000.0)) ;
}

void tezpur_init(struct tcp_opt *tp)
{
      	tp->tezpur.crtt_ = 0;
      	tp->tezpur.prtt_ = 0;
      	//tp->tezpur.sent_time_ = 0;
      	tp->tezpur.pprtt_= 0;
	//tp->tezpur.cwndincr_=2;
	tp->tezpur.min_rtt_= 0;
	tp->tezpur.max_rtt_= 0;
	tp->tezpur.rtt_cnt_= 0;
	tp->tezpur.prev_cwnd1_= 0;
	tp->tezpur.prev_cwnd2_= 0;
	//tp->tezpur.avg_tot_= 0;
	tp->tezpur.cwnd_ = 1;
	tp->tezpur.cwnd_max_=1;
	tp->tezpur.mxcwnd_rtt_= 0;
	//tp->tezpur.id_= 0;
	//tp->tezpur.sldown_= 0;
	tp->tezpur.rtt_thper_ = 50;
	tp->tezpur.rtt_thin_= 40;
	tp->tezpur.count_= 0;
	tp->tezpur.rtt_tot_= 0;
	tp->tezpur.artt_= 0;
	tp->tezpur.rtt_thresh_= 0;
	tp->tezpur.edge_= 0;
	tp->tezpur.base_rtt_= 0;
	tp->tezpur.thrtt_const_=1;
	//tp->tezpur.max_qdelay_= 0;
	//tp->tezpur.qdelay_= 0;
	//tp->tezpur.delay_= 0;
	tp->tezpur.cwndincr_= 0;
	//tp->tezpur.alpha_=0.02;
	tp->tezpur.cong_flag_=1;
	tp->tezpur.max_cwndincr_=1.5;
	tp->tezpur.WinIncrOpt_= 3;
	tp->tezpur.rtt_incr_per_= 0;
	tp->tezpur.rtt_decr_per_= 0; 
	tp->tezpur.rtt_incr_per_tot_= 0;
	tp->tezpur.rtt_decr_per_tot_= 0;
	tp->tezpur.rtt_incr_per_cnt_= 0;
	tp->tezpur.rtt_decr_per_cnt_= 0;
	tp->tezpur.rtt_var_= 0;
	tp->tezpur.rfrtt_= 0;
	tp->tezpur.max_artt_= 0;
	tp->tezpur.mxrtt_cwnd_= 0;
	tp->tezpur.rtt_abrupt_=0;
	tp->tezpur.sentsegs_=NULL;
	tp->tezpur.init_time_ = tezpur_now();
	tp->tezpur.init_flag_ = 1;
	printk(KERN_ALERT "\nTCP Tezpur Initialized!");
}

void tezpur_update_rtt(struct tcp_opt *tp,__u32 seq)
{
        
	if (tp->tezpur.init_flag_ != 1)
            tezpur_init(tp);

	printk(KERN_ALERT "\nTCP Tezpur Updating RTT for Seq# %u !",seq);

         tezpur_disp_segs(tp->tezpur.sentsegs_);
	/* Get the Current, Previous RTTs and RTT Variance */
	tp->tezpur.pprtt_ = tp->tezpur.prtt_;
	tp->tezpur.prtt_ = tp->tezpur.crtt_;
	long mnow = tezpur_now()-tp->tezpur.init_time_;
	__u32 bytes=0;
        long stm  = tezpur_find_seg_lt(tp->tezpur.sentsegs_,seq,&bytes);
        if (stm>0) {
 	   tp->tezpur.crtt_ = mnow-stm;
 	   //__u32 crtt = (__u32) (1000000.0*tp->tezpur.crtt_) ;
	   tp->tezpur.sentsegs_= tezpur_del_segs_lt(tp->tezpur.sentsegs_,seq);
	   printk(KERN_ALERT "\nTCP Tezpur AT Update RTT Current RTT=%u Bytes ACKed=%u !",(__u32)tp->tezpur.crtt_,bytes);
	}
        else
          {
	   printk(KERN_ALERT "\nTCP Tezpur Update RTT; Search Failed Exiting at STM=-1!");
           return;
         }

	tezpur_threshold_rtt_estimator(tp);
	//tp->tezpur.rtt_var_ = tp->tezpur.crtt_ - tp->tezpur.prtt_;


//Test
/*
        // Calculate the RTT Increase or Decrease %ge; used to calculate refine/projected RTT  
	int k=5;
	if (tp->tezpur.crtt_>tp->tezpur.prtt_ && tp->tezpur.prtt_>0)
	{	
		tp->tezpur.rtt_incr_pers_[(tp->tezpur.rtt_incr_per_cnt_ % k)]= (tp->tezpur.crtt_ - tp->tezpur.prtt_)/tp->tezpur.prtt_;
		tp->tezpur.rtt_incr_per_tot_ = tp->tezpur.rtt_incr_per_tot_+(tp->tezpur.crtt_-tp->tezpur.prtt_)/tp->tezpur.prtt_;
		tp->tezpur.rtt_incr_per_cnt_ = tp->tezpur.rtt_incr_per_cnt_ + 1;
		if (tp->tezpur.rtt_incr_per_cnt_>k) {
			double tmp=0;
			int j;
			for(j=0;j<k;j++)
				tmp=tmp+tp->tezpur.rtt_incr_pers_[j];
			tp->tezpur.rtt_incr_per_ = tmp/k;
		}
		else	
		tp->tezpur.rtt_incr_per_ = tp->tezpur.rtt_incr_per_tot_/tp->tezpur.rtt_incr_per_cnt_;

	}
	else
        if (tp->tezpur.crtt_<tp->tezpur.prtt_ && tp->tezpur.prtt_>0)
	{
		tp->tezpur.rtt_decr_pers_[tp->tezpur.rtt_decr_per_cnt_ % k]= (tp->tezpur.prtt_ - tp->tezpur.crtt_)/tp->tezpur.prtt_;
		tp->tezpur.rtt_decr_per_tot_ = tp->tezpur.rtt_decr_per_tot_+(tp->tezpur.prtt_-tp->tezpur.crtt_)/tp->tezpur.prtt_;
		tp->tezpur.rtt_decr_per_cnt_ = tp->tezpur.rtt_decr_per_cnt_ + 1;
		if (tp->tezpur.rtt_decr_per_cnt_>k) {
			double tmp=0;
			int j;
			for(j=0;j<k;j++)
				tmp=tmp+tp->tezpur.rtt_decr_pers_[j];
			tp->tezpur.rtt_decr_per_ = tmp/k;
		}
		else	
		tp->tezpur.rtt_decr_per_ = tp->tezpur.rtt_decr_per_tot_/tp->tezpur.rtt_decr_per_cnt_;
	}
    


       // Check to See if RTT is Changed Abruptly 
      if ((tp->tezpur.prtt_-tp->tezpur.pprtt_)>=(tp->tezpur.pprtt_/2) && tp->tezpur.pprtt_>0 && (tp->tezpur.prtt_-tp->tezpur.crtt_)>=(tp->tezpur.prtt_/2)) 	   
	   tp->tezpur.rtt_abrupt_ =1;	
       else
          tp->tezpur.rtt_abrupt_ = 0;
       
        // Get the Local Average RTT 
	 k=3;//No. of RTTs to get recent most Average RTT
	 
         tp->tezpur.rtts_[(tp->tezpur.count_ % k)]=tp->tezpur.crtt_;//21/05/2004
	 tp->tezpur.count_ +=1;
         tp->tezpur.rtt_tot_ +=tp->tezpur.crtt_;
	 if (tp->tezpur.count_>k){ //21/05/2004
		 double rttmp =0;
		 __u32 j;
		 for(j=0;j<k;j++)
			 rttmp = rttmp + tp->tezpur.rtts_[j];
		 tp->tezpur.artt_ = rttmp/k;
	 }
	 else
           tp->tezpur.artt_ = tp->tezpur.rtt_tot_/tp->tezpur.count_;



          //  Calculate Threshold RTT     
         //if (tp->tezpur.base_rtt_==0)
      	   //  tp->tezpur.base_rtt_=tp->tezpur.edge_*2*(tp->tezpur.delay_/1000.0+0.000320); //1/10000000);
*/             
//Test
	/*
         if (tp->tezpur.rtt_thresh_ <= 0 && tp->tezpur.crtt_>0)
          {
	       //17/11/2004
             	
		//double tmp=(double)tp->tezpur.crtt_;
		//tp->tezpur.rtt_thresh_ = 10.0*tmp;
		//tp->tezpur.rtt_thresh_ = 10.0*(double)tp->tezpur.crtt_;
		//tp->tezpur.rtt_thresh_ = 1245;
		//printk(KERN_ALERT "\nThreshold RTT !");

		tp->tezpur.rtt_thresh_ = tp->tezpur.rtt_thin_* tp->tezpur.crtt_;
		printk(KERN_ALERT "\nThin = %u; CRTT = %u; Threshold RTT = %u !",(__u32)tp->tezpur.rtt_thin_,(__u32)tp->tezpur.crtt_,(__u32)tp->tezpur.rtt_thresh_);
           	//tp->tezpur.rtt_thresh_ = tp->tezpur.artt_; //05/11/2004	

           }
	   */

//Test
/*		
         //Sterik Comm *else
          //if (mflag==1) 
            //   tp->tezpur.rtt_thresh_ =tp->tezpur.base_rtt_+tp->tezpur.thrtt_const_*tp->tezpur.max_qdelay_;
 
	//if (tp->tezpur.rtt_thresh_==0 || mflag==2)
          //  {
	    //   mflag=2;
              // tp->tezpur.rtt_thresh_=tp->tezpur.thrtt_const_*tp->tezpur.artt_;
             //}Sterik Comm *

          //tp->tezpur.rtt_thresh_ =tp->tezpur.rtt_thin_*tp->tezpur.artt_;

         //if (tp->tezpur.rtt_thresh_<tp->tezpur.max_rtt_)
           //  tp->tezpur.rtt_thresh_=tp->tezpur.rtt_thper_*tp->tezpur.max_rtt_/100;

	// Get Maximum and Minimum RTTs 
        if (tp->tezpur.prtt_ < tp->tezpur.min_rtt_ || tp->tezpur.min_rtt_ == 0)
          	tp->tezpur.min_rtt_ = tp->tezpur.prtt_;
     

        if (tp->tezpur.crtt_ > tp->tezpur.max_rtt_ ){
            	tp->tezpur.max_rtt_ = tp->tezpur.crtt_;
		tp->tezpur.mxrtt_cwnd_ =tp->tezpur.cwnd_;
	}

         //tp->tezpur.rtt_thresh_ = (2)*tp->tezpur.min_rtt_;

        if (tp->tezpur.crtt_ > tp->tezpur.prtt_ && tp->tezpur.prtt_!=0)
           tp->tezpur.rtt_cnt_ -=1;
        else 
          if (tp->tezpur.crtt_ <= tp->tezpur.prtt_ || tp->tezpur.prtt_==0)
            tp->tezpur.rtt_cnt_  = 0;
        if (tp->tezpur.prev_cwnd1_ >=tp->tezpur.cwnd_ && tp->tezpur.crtt_ > tp->tezpur.prtt_ )
              tp->tezpur.rtt_cnt_ -=1;
        //if (tp->tezpur.rtt_thresh_ > tp->tezpur.artt_)
	  //     tp->tezpur.artt_ = tp->tezpur.rtt_thresh_; //21/05/2004	
	 if (tp->tezpur.artt_ > tp->tezpur.max_artt_)
		 tp->tezpur.max_artt_ = tp->tezpur.artt_;
      //  double i = t_rtt_;
     //if (tp->tezpur.id_>0)
     //printf("Tz_Id=%d ;rttthresh= %f; mnow =%f; ts echo = %f t_rtt = %f tp->tezpur.rtt= %f tp->tezpur.pprtt= %f minrtt= %f  mxrtt= %f RATIO =%f RTXCUR=%f PrevCwnd2=%f PrevCwnd1=%f!",tp->tezpur.id_,tp->tezpur.rtt_thresh_,mnow,tcph->ts_echo(),i,tp->tezpur.crtt_,tp->tezpur.pprtt_,tp->tezpur.min_rtt_,tp->tezpur.max_rtt_,(tp->tezpur.rtt_thresh_-tp->tezpur.crtt_)/tp->tezpur.crtt_,t_rtxcur_,tp->tezpur.prev_cwnd2_,tp->tezpur.prev_cwnd1_);
*/
//Test
	   printk(KERN_ALERT "\nTCP Tezpur Update RTT Exiting at END!");
		
	  return;
}



/* To distinguish loss due to corruption and congestion 
 * 02/06/2004 */
__u8 tezpur_congested(struct tcp_opt *tp)
{

// Returns 1 if the loss was due to congestion else 0
     
     
	if (tp->tezpur.cong_flag_ == 0)
         {

	 if(((tp->tezpur.crtt_ > (tp->tezpur.rtt_thresh_*2/3)) && (tp->tezpur.crtt_>tp->tezpur.prtt_ ) && tp->tezpur.prtt_>0) || (tp->tezpur.crtt_>tp->tezpur.rtt_thresh_ ))
			 return 1;
		 else
			 return 0;
	 }
 
	
      return tp->tezpur.cong_flag_;

}
 
double tezpur_get_cwnd_incr(struct tcp_opt *tp)
{

	//RTT Based Technique (2001-2002)

	//18/11/2004 
	if (tp->tezpur.rtt_thresh_ <= 0 || tp->tezpur.crtt_ <= 0)
	   return 1;
        else {
           double tmp,tmp1,tmp2; 
	   __u32 t;
            
           tmp1=12;
	   tmp2=4;
	   tmp=tmp1/tmp2;
	   printk("\n TCP Tezpur(Div Test): Numerator=%u ; Denominator=%u ; Ratio=%u !",(__u32)tmp1,(__u32)tmp2,(__u32)tmp);
	if (tp->tezpur.rtt_thresh_ > tp->tezpur.crtt_ )
	    return 1;
        else
            return -1;

	   t=(__u32)tp->tezpur.rtt_thresh_;
	   tmp1=(double)t;
	   t=(__u32)tp->tezpur.crtt_;
	   tmp1=tmp1-(double)t;
    	   //tmp2=(double)(max(tp->tezpur.crtt_,tp->tezpur.rtt_thresh_)); //New
    	   t=(__u32)(max(tp->tezpur.crtt_,tp->tezpur.rtt_thresh_)); //New
	   tmp2=(double)t;
    	   //tmp2=(double)tp->tezpur.rtt_thresh_; //New
	   tmp=tmp1/tmp2;
	   //tmp=tmp1;
	   printk("\n TCP Tezpur: CRTT=%u ; tmp1=%u ; tmp2=%u ; tmp=%u !",(__u32)tp->tezpur.crtt_,(__u32)(100000+tmp1),(__u32)tmp2,(__u32)(10000+tmp));
	   printk("\n TCP Tezpur: CRTT=%u ; tmp1=%d ; tmp2=%d ; tmp=%d !",(__u32)tp->tezpur.crtt_,tmp1,tmp2,tmp);
	   return tmp;
	   //return ((double)(tp->tezpur.rtt_thresh_-tp->tezpur.crtt_))/((double)max(tp->tezpur.crtt_,tp->tezpur.rtt_thresh_)); //New
         }

	//return (tp->tezpur.rtt_thresh_-tp->tezpur.crtt_)/min(tp->tezpur.crtt_,tp->tezpur.rtt_thresh_); //Old
   
}

/* Previously as void tz_opencwnd(struct sock *sk) */
void tezpur_cong_avoid(struct tcp_opt *tp)
{

   tp->tezpur.prev_cwnd2_ = tp->tezpur.prev_cwnd1_;
   tp->tezpur.prev_cwnd1_ = tp->tezpur.cwnd_;
   if (tp->tezpur.cwnd_max_ < tp->tezpur.cwnd_)
      {
       tp->tezpur.cwnd_max_ = tp->tezpur.cwnd_;
       tp->tezpur.mxcwnd_rtt_ = tp->tezpur.crtt_;
      }
 
  if (tp->tezpur.WinIncrOpt_ == 3) {

   tp->tezpur.cwndincr_ = tezpur_get_cwnd_incr(tp); //RTT Based cwnd increment technique
     //tezpur_get_cwnd_incr(tp); //RTT Based cwnd increment technique
  
  }  

   //tp->tezpur.cwndincr_ = 2;

    //tp->tezpur.cwnd_ +=2;
    //tp->tezpur.cwnd_ +=tp->tezpur.cwndincr_ ;
/*
	if (tp->tezpur.rtt_thresh_ <= 0 || tp->tezpur.crtt_ <= 0)
           tp->tezpur.cwnd_ += 1;
        else 
        if (tp->tezpur.rtt_thresh_ > tp->tezpur.crtt_ )
           tp->tezpur.cwnd_ += 1;
        else
           tp->tezpur.cwnd_ -= 1;
*/
	if (tp->tezpur.rtt_thresh_ <= 0 || tp->tezpur.crtt_ <= 0)
           tp->tezpur.cwnd_ += 1;
	else {
           double tmp,tmp1,tmp2; 
	   __u32 t;

	   t=(__u32)tp->tezpur.rtt_thresh_;
	   tmp1=(double)t;
	   t=(__u32)tp->tezpur.crtt_;
	   tmp1=tmp1-(double)t;
    	   t=(__u32)(min(tp->tezpur.crtt_,tp->tezpur.rtt_thresh_)); 
    	   //t=(__u32)(max(tp->tezpur.crtt_,tp->tezpur.rtt_thresh_)); 
	   tmp2=(double)t;

           tp->tezpur.cwnd_ +=(tmp1/tmp2);
	   printk("\n TCP Tezpur: CRTT=%u ; tmp1=%u ; tmp2=%u !",(__u32)tp->tezpur.crtt_,(__u32)tmp1,(__u32)tmp2);
           //tp->tezpur.cwndincr_ =(tmp1/tmp2);
        }

    if (tp->tezpur.cwnd_ < 1)
           tp->tezpur.cwnd_ = 1;

    tp->snd_cwnd = (__u32)tp->tezpur.cwnd_;	

     //if (tp->snd_cwnd>wnd_)
     //tp->snd_cwnd=wnd_;    //Keeping cwnd well within wnd

     //if (tp->snd_cwnd < 1)
       //    tp->snd_cwnd = 1;

     //if (tp->tezpur.id_>0)
   //printf("TcpTezpur:mxcwnd=%d mxcwrtt=%f cwndincr=%f Ssthres=%d;cwnd= %d!\n",(int)tp->tezpur.cwnd_max_,tp->tezpur.mxcwnd_rtt_,tp->tezpur.cwndincr_,(int)ssthresh_,(int)tp->tezpur.cwnd_); 
	   //printk(KERN_ALERT "\n TCP Tezpur: CRTT=%u ; tmp1=%d ; tmp2=%d !",(__u32)tp->tezpur.crtt_,(int)tmp1,(int)tmp2);

	printk(KERN_ALERT "\nTCP Tezpur CONG AVOID Called:CwndIncr=%u; TzCwnd=%u ; Sndcwnd =%u!",(__u32)tp->tezpur.cwndincr_,(__u32)tp->tezpur.cwnd_,tp->snd_cwnd);
}


/* Called to compute a smoothed rtt estimate. The data fed to this
 * routine either comes from timestamps, or from segments that were
 * known _not_ to have been retransmitted [see Karn/Partridge
 * Proceedings SIGCOMM 87]. The algorithm is from the SIGCOMM 88
 * piece by Van Jacobson.
 * NOTE: the next three routines used to be one big routine.
 * To save cycles in the RFC 1323 implementation it was better to break
 * it up into three procedures. -- erics
 */
/* Written on 30/11/2004 */
static void tezpur_threshold_rtt_estimator(struct tcp_opt *tp)
{
	long m = tp->tezpur.crtt_; /* RTT */

	/*	The following amusing code comes from Jacobson's
	 *	article in SIGCOMM '88.  Note that rtt and mdev
	 *	are scaled versions of rtt and mean deviation.
	 *	This is designed to be as fast as possible 
	 *	m stands for "measurement".
	 *
	 *	On a 1990 paper the rto value is changed to:
	 *	RTO = rtt + 4 * mdev
	 *
	 * Funny. This algorithm seems to be very broken.
	 * These formulae increase RTO, when it should be decreased, increase
	 * too slowly, when it should be incresed fastly, decrease too fastly
	 * etc. I guess in BSD RTO takes ONE value, so that it is absolutely
	 * does not matter how to _calculate_ it. Seems, it was trap
	 * that VJ failed to avoid. 8)
	 */
	if(m == 0)
		m = 1000;//In terms of Micro-Second;03/12/2004
	if (tp->tezpur.srtt_ != 0) {
		m -= (tp->tezpur.srtt_ >> 3);	/* m is now error in rtt est */
		tp->tezpur.srtt_ += m;		/* rtt = 7/8 rtt + 1/8 new */
		if (m < 0) {
			m = -m;		/* m is now abs(error) */
			m -= (tp->tezpur.mdev_ >> 2);   /* similar update on mdev */
			/* This is similar to one of Eifel findings.
			 * Eifel blocks mdev updates when rtt decreases.
			 * This solution is a bit different: we use finer gain
			 * for mdev in this case (alpha*beta).
			 * Like Eifel it also prevents growth of rto,
			 * but also it limits too fast rto decreases,
			 * happening in pure Eifel.
			 */
			if (m > 0)
				m >>= 3;
		} else {
			m -= (tp->tezpur.mdev_ >> 2);   /* similar update on mdev */
		}
		tp->tezpur.mdev_ += m;	    	/* mdev = 3/4 mdev + 1/4 new */
		if (tp->tezpur.mdev_ > tp->tezpur.mdev_max_) {
			tp->tezpur.mdev_max_ = tp->tezpur.mdev_;
			if (tp->tezpur.mdev_max_ > tp->tezpur.rttvar_)
				tp->tezpur.rttvar_ = tp->tezpur.mdev_max_;
		}
		//if (after(tp->tezpur.snd_una, tp->tezpur.rtt_seq_)) {
		if (after(tp->snd_una, tp->tezpur.rtt_seq_)) {
			if (tp->tezpur.mdev_max_ < tp->tezpur.rttvar_)
				tp->tezpur.rttvar_ -= (tp->tezpur.rttvar_-tp->tezpur.mdev_max_)>>2;
			tp->tezpur.rtt_seq_ = tp->snd_nxt;
			//tp->tezpur.rtt_seq_ = tp->tezpur.snd_nxt;
			tp->tezpur.mdev_max_ = TEZPUR_RTO_MIN;
		}
	} else {
		/* no previous measure. */
		tp->tezpur.srtt_ = m<<3;	/* take the measured time to be rtt */
		tp->tezpur.mdev_ = m<<1;	/* make sure rto = 3*rtt */
		tp->tezpur.mdev_max_ = tp->tezpur.rttvar_ = max(tp->tezpur.mdev_, TEZPUR_RTO_MIN);
		tp->tezpur.rtt_seq_ = tp->snd_nxt;
		//tp->tezpur.rtt_seq_ = tp->tezpur.snd_nxt;
	}

	tp->tezpur.rto_ = (tp->tezpur.srtt_ >> 3) + tp->tezpur.rttvar_;
	tp->tezpur.rtt_thresh_ = tp->tezpur.rto_ >> 1;

}

/*End of TCP Tezpur Functions*/
