#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include "common_hdr.h"

int my_sendMsg_to_kernel(int sockId, const char *msgBuff, unsigned int msgLen, NL_MSG_TYPE msgType)
{
	struct sockaddr_nl kernel;
	struct nlmsghdr *nlh = NULL;
	struct iovec	iov;
	struct msghdr msg;
	int send_len = 0;

	nlh = (struct nlmsghdr *)calloc(1, NLMSG_SPACE(msgLen));
	if(!nlh)
	{
		printf("Unable to allocate the memory for nlh\n");
		return -1;
	}
	nlh->nlmsg_type = msgType;
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_len = NLMSG_SPACE(msgLen);
	memcpy(NLMSG_DATA(nlh), msgBuff, msgLen);

	switch(msgType)
	{
		case HELLO_MSG:
		{
			printf("***** Sending Hello msg to kernel from user space *****\n");
		}
		break;
		case BYE_MSG:
		{
			printf("***** Sending Bye Bye msg to kernel from user space *****\n");
		}
		break;
		default:
			printf("Undefined msgType received\n");
	}

	memset(&kernel, '\0', sizeof(struct sockaddr_nl));
	kernel.nl_family = AF_NETLINK;
	kernel.nl_pid = 0;
	kernel.nl_groups = 0;

	memset(&iov, '\0', sizeof(struct iovec));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, '\0', sizeof(struct msghdr));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_name = &kernel;
	msg.msg_namelen = sizeof(kernel);

	send_len = sendmsg(sockId, &msg, 0);
	if(send_len < 0)
	{
		printf("Failed to send the netlink msg to kernel\n");
		return -1;
	}
	return 0;
}

void my_rcvMsg_from_kernel(int sockId)
{
	char buff[1024] = {'\0'};
	struct sockaddr_nl kernel;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;
	int rcv_len = 0;

	nlh = (struct nlmsghdr *)buff;
	nlh->nlmsg_len = NLMSG_SPACE(1024);

	memset(&kernel, '\0', sizeof(struct sockaddr_nl));
	kernel.nl_family = AF_NETLINK;
	kernel.nl_pid = 0;
	kernel.nl_groups = 0;

	memset(&iov, '\0', sizeof(struct iovec));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, '\0', sizeof(struct msghdr));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_name = &kernel;
	msg.msg_namelen = sizeof(kernel);

	rcv_len = recvmsg(sockId, &msg, 0);
	if(rcv_len > 0)
	{
		printf("***** Received '%s' from kernel space\n", (char *)NLMSG_DATA(nlh));
		/*switch(nlh->nlmsg_type)
		{
			case HELLO_MSG:
			{
				printf("***** Received '%s' from kernel space\n", (char *)NLMSG_DATA(nlh));
			}
			break;
			case BYE_MSG:
			{
				printf("***** Received '%s' from kernel space\n", (char *)NLMSG_DATA(nlh));
			}
			break;
			default:
				printf("Received undefined msgType\n");
		}*/
	}
	return;
}

int main(int argc, char **argv)
{
	int nl_sock = 0;
	char msg[128] = {'\0'};
	unsigned int msg_len = 0;
	struct sockaddr_nl user;

	nl_sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_MSG);
	if(nl_sock < 0)
	{
		printf("Unable to create the netlink socket\n");
		return -1;
	}

	memset(&user, '\0', sizeof(struct sockaddr_nl));
	user.nl_family = AF_NETLINK;
	user.nl_pid = getpid();

	if(bind(nl_sock, (struct sockaddr *)&user, sizeof(struct sockaddr_nl)) < 0)
	{
		printf("Unable to bind the netlink socket\n");
		close(nl_sock);
		return -1;
	}

	strcpy(msg, "Hello kernel space");
	msg_len = strlen(msg);
	if(my_sendMsg_to_kernel(nl_sock, msg, msg_len, HELLO_MSG) == 0)
	{
		my_rcvMsg_from_kernel(nl_sock);
	}

	memset(msg, '\0', sizeof(msg));
	strcpy(msg, "Bye Bye kernel space");
	msg_len = strlen(msg);
	if(my_sendMsg_to_kernel(nl_sock, msg, msg_len, BYE_MSG) == 0)
	{
		my_rcvMsg_from_kernel(nl_sock);
	}

	close(nl_sock);
	return 0;
}
