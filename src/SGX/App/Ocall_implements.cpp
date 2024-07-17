#include "Ocall_implements.h"
#include "Enclave_u.h"

long ocall_sgx_clock(void)
{
	struct timespec tstart={0,0};
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	return tstart.tv_sec * 1000000 + tstart.tv_nsec/1000; // Return micro seconds
}

time_t ocall_sgx_time(time_t *timep, int t_len)
{
	return 	time(timep);
}

struct tm *ocall_sgx_localtime(const time_t *timep, int t_len)
{
	return localtime(timep);
}

struct tm *ocall_sgx_gmtime_r(const time_t *timep, int t_len, struct tm *tmp, int tmp_len)
{
	return gmtime_r(timep, tmp);
}

int ocall_sgx_gettimeofday(void *tv, int tv_size)
{
	return gettimeofday((struct timeval *)tv, NULL);
}

int ocall_sgx_getsockopt(int s, int level, int optname, char *optval, int optval_len, int* optlen)
{
	return getsockopt(s, level, optname, optval, (socklen_t *)optlen);
}

int ocall_sgx_setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
	return setsockopt(s, level, optname, optval, optlen);
}

int ocall_sgx_socket(int af, int type, int protocol)
{
	int retv;
	retv = socket(af, type, protocol);
	return retv;
}

int ocall_sgx_bind(int s, const void *addr, int addr_size)
{
	return bind(s, (struct sockaddr *)addr, addr_size);
}

int ocall_sgx_listen(int s, int backlog)
{
	return listen(s, backlog);
}

int ocall_sgx_connect(int s, const void *addr, int addrlen)
{
	int retv = connect(s, (struct sockaddr *)addr, addrlen);
	return retv;
}

int ocall_sgx_accept(int s, void *addr, int addr_size, int *addrlen)
{
	return accept(s, (struct sockaddr *)addr, (socklen_t *)addrlen);
}

int ocall_sgx_shutdown(int fd, int how)
{
	return shutdown(fd, how);
}

int ocall_sgx_read(int fd, void *buf, int n)
{
	return read(fd, buf, n);
}

int ocall_sgx_write(int fd, const void *buf, int n)
{
	return write(fd, buf, n);
}

int ocall_sgx_close(int fd)
{
	return close(fd);
}

int ocall_sgx_getenv(const char *env, int envlen, char *ret_str,int ret_len)
{
	const char *env_val = getenv(env);
	if(env_val == NULL){
		return -1;
	}
	memcpy(ret_str, env_val, strlen(env_val)+1);
	return 0;
}

void ocall_print_string(const char *str)
{
	printf("%s", str);
}

void ocall_sgx_exit(int e)
{
	exit(e);
}

int ocall_read_eneq0(char** eneq)
{
    ifstream eneq0("/var/lib/mysql/file/eneq0.txt");
    if(eneq0.fail())
        cout<<"FAIL!"<<endl;
    int i = 0;
    while(!eneq0.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq0 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq0.close();
    return i;
}
int ocall_read_eneq1(char** eneq)
{
    ifstream eneq1("/var/lib/mysql/file/eneq1.txt");
    if(eneq1.fail())
        cout<<"FAIL!"<<endl;
    int i = 0;
    while(!eneq1.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq1 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq1.close();
    return i;
}

int ocall_read_eneq2(char** eneq)
{
    ifstream eneq2("/var/lib/mysql/file/eneq2.txt");
    int i = 0;
    while(!eneq2.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq2 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq2.close();
    return i;
}

int ocall_read_eneq3(char** eneq)
{
    ifstream eneq3("/var/lib/mysql/file/eneq3.txt");
    if(eneq3.fail())
        cout<<"FAIL!"<<endl;
    int i = 0;
    while(!eneq3.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq3 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq3.close();
    return i;
}

int ocall_read_eneq4(char** eneq)
{
    ifstream eneq4("/var/lib/mysql/file/eneq4.txt");
    int i = 0;
    while(!eneq4.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq4 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq4.close();
    return i;
}

int ocall_read_eneq5(char** eneq)
{
    ifstream eneq5("/var/lib/mysql/file/eneq5.txt");
    if(eneq5.fail())
        cout<<"FAIL!"<<endl;
    int i = 0;
    while(!eneq5.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq5 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq5.close();
    return i;
}

int ocall_read_eneq6(char** eneq)
{
    ifstream eneq6("/var/lib/mysql/file/eneq6.txt");
    int i = 0;
    while(!eneq6.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq6 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq6.close();
    return i;
}

int ocall_read_eneq7(char** eneq)
{
    ifstream eneq7("/var/lib/mysql/file/eneq7.txt");
    if(eneq7.fail())
        cout<<"FAIL!"<<endl;
    int i = 0;
    while(!eneq7.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq7 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq7.close();
    return i;
}

int ocall_read_eneq8(char** eneq)
{
    ifstream eneq8("/var/lib/mysql/file/eneq8.txt");
    int i = 0;
    while(!eneq8.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq8 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq8.close();
    return i;
}
int ocall_read_eneq9(char** eneq)
{
    ifstream eneq9("/var/lib/mysql/file/eneq9.txt");
    int i = 0;
    while(!eneq9.eof())
    {
        string v;
        eneq[i]=(char*)malloc(25);
        eneq9 >> v;
        memcpy(eneq[i],v.c_str(),25);
        i++;
    }
    eneq9.close();
    return i;
}

ofstream re1;
ifstream enquery;
void ocall_open_result()
{
    re1.open("/var/lib/mysql/file/result.txt");
}
void ocall_write_result(int data)
{
    re1 << data << " ";
}
void ocall_writeendl_result()
{
    re1 << endl;
}
void ocall_close_result()
{
    re1.close();
}

void ocall_open_enquery()
{
    enquery.open("/var/lib/mysql/file/enquery.txt");
}

void ocall_read_s(char* s,int n)
{
    enquery >> s;
}

void ocall_close_enquery()
{
    enquery.close();
}