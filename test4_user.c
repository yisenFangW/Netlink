//
// Created by 方伟 on 2019-10-11.
//
// user端编译方式 gcc test4_user.c -o user $(pkg-config --cflags --libs libnl-genl-3.0)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <errno.h>


#define HELP_STRING "usage:%s\n \
		add|del name age weight \n"

#define TEST_GENL_NAME "TEST"
#define TEST_GENL_VERSION    0x1
#define NAMELEN  20

enum {
    CTRL_ATTR_UNDO = 0,
    CTRL_ATTR_NAME,
    CTRL_ATTR_AGE,
    CTRL_ATTR_WEIGHT,
    __CTRL_ATTR_MAX_TEST,
};
#define CTRL_ATTR_MAX_TEST (__CTRL_ATTR_MAX_TEST - 1)

enum {
    CTRL_CMD_UNDO = 0,
    CTRL_CMD_ADD,
    CTRL_CMD_DEL,
    __CTRL_CMD_MAX_TEST,
};
#define CTRL_CMD_MAX_TEST (__CTRL_CMD_MAX_TEST -1)

struct user_key{
    char name[NAMELEN];
    unsigned int age;
    unsigned int weight;
};

void print_help(int argc, char **argv) {
    fprintf(stderr, HELP_STRING, argv[0]);
    exit(1);
}

static int noop_parse_cb(struct nl_msg *msg, void *args) {
    return NL_OK;
}

static inline int parse_options(struct nl_msg *msg, struct user_key* user) {
    struct nlattr* attr;
    attr = nla_nest_start(msg, 1);
    NLA_PUT_STRING(msg, CTRL_ATTR_NAME, user->name);
    NLA_PUT_U32(msg, CTRL_ATTR_AGE, user->age);
    NLA_PUT_U32(msg, CTRL_ATTR_WEIGHT, user->weight);
    nla_nest_end(msg, attr);
    return 0;
    nla_put_failure:
    nlmsg_free(msg);
    return 1;
}

int main(int argc, char **argv) {
    int err = EINVAL;
    int cmd;
    struct nl_sock *sock = NULL;
    int family = -1;
    struct nl_msg *msg = NULL;

    if (argc != 5)
        print_help(argc, argv);

    if (!strncmp(argv[1], "add", 3)) {
        cmd = CTRL_CMD_ADD;
    } else if (!strncmp(argv[1], "del", 3)) {
        cmd = CTRL_CMD_DEL;
    } else
        print_help(argc, argv);

    struct user_key *user;
    user = malloc(sizeof(struct user_key));
    if(!user){
        fprintf(stderr, "create memory failed.\n");
        return -1;
    }
    strncpy(user->name, argv[2], NAMELEN);
    user->age = atoi(argv[3]);
    user->weight = atoi(argv[4]);

    if (NULL == sock) {
        if (NULL == (sock = nl_socket_alloc())) {
            fprintf(stderr, "create handler error");
            return -1;
        }
        if (genl_connect(sock) < 0) {
            fprintf(stderr, "genl_connect failed.\n");
            goto fail_genl;
        }
    }

    if (family == -1 && (family = genl_ctrl_resolve(sock, TEST_GENL_NAME)) < 0) {
        fprintf(stderr, "resolve NL_NAME failed.\n");
        goto fail_genl;
    }

    if (NULL == (msg = nlmsg_alloc())) {
        fprintf(stderr, "Alloc nl_msg error.\n");
        goto fail_genl;
    }

    if (parse_options(msg, user)) {
        return -1;
    }

    genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, 0, cmd, TEST_GENL_VERSION);


    if (nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, noop_parse_cb, NULL) != 0)
        goto fail_genl;

    if (nl_send_auto_complete(sock, msg) < 0) {
        fprintf(stderr, "send msg error .\n");
        goto fail_genl;
    } else {
        prink("print success!\n");
    }

    nlmsg_free(msg);
    nl_socket_free(sock);
    return 0;

    fail_genl:
    nl_socket_free(sock);
    sock = NULL;
    return -1;
}
