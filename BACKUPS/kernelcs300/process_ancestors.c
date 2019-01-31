#include "process_ancestors.h"
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cred.h>

/* Implementing process_ancestors function: */
asmlinkage long sys_process_ancestors(struct process_info info_array[], 
                long size, 
                long *num_filled){
    
    struct task_struct *currTask = current;
    struct list_head    *currChildren;
    struct list_head    *currSiblings;
    struct list_head    *indexer;
    struct process_info	currPInfo;
    long index;
    long currFilled;
    
    printk("DEBUG: sys_process_ancestors called! infoarray = %p, Size = %ld, num_filled = %p\n", info_array, size, num_filled);

    if (size <= 0){
        printk("\nDEBUG: in sys_process_ancestors: size was <= 0!\n");
        return -EINVAL;
    }

    index = 0;

    while (index < size ){
        if (currTask->parent == currTask){
            break;
        }

        currPInfo.pid = (long) currTask->pid;
       	strcpy( currPInfo.name, currTask->comm );
        currPInfo.state = currTask->state;
        currPInfo.uid = (long) (currTask->cred)->uid.val;
        currPInfo.nvcsw = (long) currTask->nvcsw;
        currPInfo.nivcsw = (long) currTask->nivcsw;
        currPInfo.num_children = 0; /* Counting children is done immediately after this */
        currPInfo.num_siblings = 0; /* Counting siblings is done after counting children */
        
        currChildren = &(currTask->children);
        currSiblings = &(currTask->sibling);

        /* Getting the number of children: */
        list_for_each(indexer, currChildren){
            ++currPInfo.num_children;
        }
        /* Getting the number of siblings: */
        list_for_each(indexer, currSiblings){
            ++currPInfo.num_siblings;
        }
        --currPInfo.num_siblings; /* Delete itself, because otherwise it gets counted as it's own sibling */


        printk("\nDEBUG: At index %ld, size = %ld, pid = %ld, name = %s, state = %ld, uid = %ld, nvcsw = %ld,\n\
                        nivcsw = %ld, num_children = %ld, num_siblings = %ld.\n", index, size, currPInfo.pid,
                        currPInfo.name, currPInfo.state, currPInfo.uid, currPInfo.nvcsw, 
                        currPInfo.nivcsw, currPInfo.num_children, currPInfo.num_siblings);


        printk("\nDEBUG: &currFilled = %p", &currFilled);
        printk("\nDEBUG: num_flled = %p", num_filled);

        if (copy_to_user(num_filled, &currFilled, (unsigned long) sizeof(long int))){
            printk("DEBUG: in sys_process_ancestors: copying to num_filled failed! index = %ld, num_filled = %ld, size = %ld\n", index, num_filled, size);
            return -EFAULT;
        }
        if ( copy_to_user(&info_array[index], &currPInfo, (unsigned long) sizeof(struct process_info)) ){
            printk("DEBUG: in sys_process_ancestors: Copying to array failed! index = %ld, size = %ld\n", index, size);
            return -EFAULT;
        }
        
/*        printk("DEBUG: Just finished writing to index %ld\n", index);*/

        /* Write to the next index on the next loop: */
        index += 1;
        currFilled = index + 1;
        currTask = currTask->parent;

        
    }

    return 0;
}
