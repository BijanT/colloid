#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/mm.h>
//#include <linux/memory-tiers.h>
#include <linux/delay.h>

extern int colloid_local_lat_gt_remote;
extern int colloid_nid_of_interest;

struct kobject *colloid_kobj = NULL;

#define LOCAL_NUMA 0

static ssize_t colloid_local_gt_remote_show(struct kobject *kobj,
        struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", colloid_local_lat_gt_remote);
}

static ssize_t colloid_local_gt_remote_store(struct kobject *kobj,
        struct kobj_attribute *attr, const char *buf, size_t count)
{
    int val;
    int err;

    err = kstrtoint(buf, 0, &val);
    if (err)
        return err;

    WRITE_ONCE(colloid_local_lat_gt_remote, val);
    return count;
}
static struct kobj_attribute colloid_latency_attr =
__ATTR(local_gt_remote, 0644, colloid_local_gt_remote_show, colloid_local_gt_remote_store);

static struct attribute *colloid_attr[] = {
    &colloid_latency_attr.attr,
};

static const struct attribute_group colloid_attr_group = {
    .attrs = colloid_attr,
};

static int colloid_perf_init(void)
{
    int err;

    WRITE_ONCE(colloid_nid_of_interest, LOCAL_NUMA);

    colloid_kobj = kobject_create_and_add("colloid", kernel_kobj);
    if (unlikely(!colloid_kobj)) {
        pr_err("Failed to create colloid kobj\n");
        return -ENOMEM;
    }

    err = sysfs_create_group(colloid_kobj, &colloid_attr_group);
    if (err) {
        pr_err("Failed to register colloid group\n");
        kobject_put(colloid_kobj);
        return err;
    }

    return 0;
}

static void colloid_perf_exit(void)
{
    if (colloid_kobj) {
        sysfs_remove_group(colloid_kobj, &colloid_attr_group);
        kobject_put(colloid_kobj);
    }
}

module_init(colloid_perf_init);
module_exit(colloid_perf_exit);
MODULE_LICENSE("GPL");
