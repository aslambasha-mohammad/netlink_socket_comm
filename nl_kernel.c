#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include "common_hdr.h"

unsigned int U_PID = 0;
struct sock *nl_sock = NULL;

static int my_sendMsg_to_user(const char *msg, unsigned int msgLen, NL_MSG_TYPE msgType)
{
	struct sk_buff *skb_out = NULL;
	struct nlmsghdr *nlh = NULL;
	int res = 0;

	skb_out = nlmsg_new(msgLen, 0);
	if(!skb_out)
	{
		printk(KERN_DEBUG "Unable to create the nlmsg_new\n");
		return -1;
	}

	nlh = nlmsg_put(skb_out, 0, 0, msgType, msgLen, 0);
	if(!nlh)
	{
		printk(KERN_DEBUG "Unable to do nlmsg_put\n");
		return -1;
	}
	NETLINK_CB(skb_out).dst_group = 0;	//unicast
	memcpy(NLMSG_DATA(nlh), msg, msgLen);

	res = nlmsg_unicast(nl_sock, skb_out, U_PID);
	if(res < 0)
	{
		printk(KERN_DEBUG "Unable to send the Netlink msg to User space\n");
		return -1;
	}

	return 0;
}

static void my_callback_rcv(struct sk_buff *skb_in)
{
	struct nlmsghdr *nlh = NULL;
	char msg[128] = {'\0'};
	unsigned msg_len = 0;

	nlh = (struct nlmsghdr *)skb_in->data;
	switch(nlh->nlmsg_type)
	{
		case HELLO_MSG:
		{
			U_PID = nlh->nlmsg_pid;
			printk(KERN_DEBUG "***** Received '%s' from User space with PID = %d*****\n", (char *)NLMSG_DATA(nlh), U_PID);
			strcpy(msg, "Saying Hello back to user space from kernel");
			msg_len = strlen(msg);
			my_sendMsg_to_user(msg, msg_len, HELLO_MSG);
		}
		break;
		case BYE_MSG:
		{
			printk(KERN_DEBUG "***** Received '%s' from the User space *****\n", (char *)NLMSG_DATA(nlh));
                        strcpy(msg, "Saying Bye Bye back to user space from kernel");
                        msg_len = strlen(msg);
                        my_sendMsg_to_user(msg, msg_len, BYE_MSG);
		}
		break;
		default:
			printk(KERN_DEBUG "Undefined message type has been received\n");
	}
	return;
}

static int __init nl_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = my_callback_rcv,
	};

	nl_sock = netlink_kernel_create(&init_net, NETLINK_MSG, &cfg);
	if(nl_sock == NULL)
	{
		printk(KERN_DEBUG "Unable to create the Netlink socket\n");
		return -1;
	}

	printk(KERN_DEBUG "Successfully Inserted the Netlink Module\n");
	return 0;
}

static void __exit nl_exit(void)
{
	netlink_kernel_release(nl_sock);
	printk(KERN_DEBUG "Successfully Removed the Netlink Module\n");
	return;
}

module_init(nl_init);
module_exit(nl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aslam");
MODULE_DESCRIPTION("Sample netlink socket communication module");
MODULE_VERSION("1.0");
