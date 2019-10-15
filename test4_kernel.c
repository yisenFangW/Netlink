//
// Created by 方伟 on 2019-10-11.
//

#include <net/genetlink.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define TEST_GENL_NAME "TEST"
#define TEST_GENL_VERSION    0x1
#define NAMELEN 20

enum {
    CTRL_ATTR_UNDO,
    CTRL_ATTR_NAME,
    CTRL_ATTR_AGE,
    CTRL_ATTR_WEIGHT,
    __CTRL_ATTR_MAX_TEST,
};
#define CTRL_ATTR_MAX_TEST (__CTRL_ATTR_MAX_TEST - 1)

enum {
    CTRL_CMD_UNDO,
    CTRL_CMD_ADD,
    CTRL_CMD_DEL,
    __CTRL_CMD_MAX_TEST,
};
#define CTRL_CMD_MAX_TEST (__CTRL_CMD_MAX_TEST -1)

struct genl_family test_ctl = {
        .id = GENL_ID_GENERATE,
        .name = TEST_GENL_NAME,
        .version = TEST_GENL_VERSION,
        .maxattr = CTRL_ATTR_MAX_TEST,
        .netnsok = true,
};

static const struct nla_policy test_ctl_policy[CTRL_ATTR_MAX_TEST + 1] = {
        [CTRL_ATTR_NAME] = {.type = NLA_STRING},
        [CTRL_ATTR_AGE] = {.type = NLA_U16},
        [CTRL_ATTR_WEIGHT] = {.type = NLA_U16},
};

int test_ctl_func(struct sk_buff *skb, struct genl_info *info) {
    struct sk_buff *skbuff = NULL;
    char *name;
    int age, weight;
    void *hdr;
    int msg_size;

    printk("get process pid=%d  cmd=%d pid=%d type=%d len=%d\n",info->snd_pid,info->genlhdr->cmd,
            info->nlhdr->nlmsg_pid,info->nlhdr->nlmsg_type,info->nlhdr->nlmsg_len);

    int cmd = info->genlhdr->cmd;
    if(cmd == CTRL_CMD_ADD)
        printk("cmd is add!\n");
    else
        printk("cmd is del!\n");

    if(info->attrs[CTRL_ATTR_NAME]){
        name = nla_data(info->attrs[CTRL_ATTR_NAME]);
        printk("recv message from usr name is %s\n", name);
    }
    if(info->attrs[CTRL_ATTR_AGE]){
        age = nla_get_u16(info->attrs[CTRL_ATTR_AGE]);
        printk("recv message from usr age is %d\n", age);
    }
    if(info->attrs[CTRL_ATTR_WEIGHT]){
        weight = nla_get_u16(info->attrs[CTRL_ATTR_WEIGHT]);
        printk("recv message from usr weight is %d\n", weight);
    }
    return 0;
}

static struct genl_ops test_ctl_ops = {
        .cmd = CTRL_CMD_ADD,
        .doit = test_ctl_func,
        .policy = test_ctl_policy,
};

static int __init testnlk_init(void){
    if(genl_register_family(&test_ctl) != 0)
    {
        printk("register faimly error\n");
        return -1;
    }

    if(genl_register_ops(&test_ctl,&test_ctl_ops) !=  0)
    {
        printk("Register ops error\n");
        goto out;
    }

    return 0;
    out:
    genl_unregister_family(&test_ctl);
    return 0;
}

static void __exit testnlk_exit(void)
{
    genl_unregister_ops(&test_ctl,&test_ctl_ops);
    genl_unregister_family(&test_ctl);

}

module_init(testnlk_init);
module_exit(testnlk_exit);

MODULE_LICENSE("GPL");



























