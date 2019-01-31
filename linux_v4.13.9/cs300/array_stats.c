#include "array_stats.h"
#include <linux/uaccess.h>
#include <linux/kernel.h>

/* Implementing an array_stats function: */
asmlinkage long sys_array_stats( struct array_stats *stats,
                 long data[],
                 long size){
    long currMin = 0;
    long currMax = 0;    
    long currSum = 0;
    int i = 0;
    long dataval = 0;
    struct array_stats currStats;

    printk("DEBUG: sys_array_stats called with data = %p, size = %ld, stats = %p",
                 (void *) data, size, (void *) stats);
    
    if (size <= 0){
        printk("DEBUG: in sys_array_stats: size was <= 0!");
        return -EINVAL;
    }

    for (i = 0; i < size; ++i){

        if ( copy_from_user(&dataval, &data[i], (unsigned long) sizeof(long)) ){
            printk("DEBUG: in sys_array_stats: copy from data at index %d failed!", i);
            return -EFAULT;
        }

        if (i == 0){
            currMin = data[i];
            currMax = data[i];
            currSum = data[i];
        } else {
            currMin = data[i] < currMin ? data[i] : currMin;
            currMax = data[i] > currMax ? data[i] : currMax;
            currSum += data[i];
        }
    }

    currStats.min = currMin;
    currStats.max = currMax;
    currStats.sum = currSum;

    if (copy_to_user(stats, &currStats, (unsigned long) sizeof(struct array_stats))){
        printk("DEBUG: in sys_array_stats: copy to stats array failed!");
        return -EFAULT;
    }
      
    return 0;  

}
