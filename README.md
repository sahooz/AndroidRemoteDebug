## 概述
1. 主要工作： 把Java调试传输模式设置为kJdwpTransportSocket，通过socket转发调试指令和回复信息    
2. 主要问题： Command packet与协议定义的似乎不对应: ，以获取版本指令为例：
``` c
char version[11] = {
        0, 0, 0, 11,
        0, 0, 1, 1,
        0,
        1,
        1
    };

    send(sock_cli, version, 11, 0);
```
server socket接受到的却是： 000000000b00000101000101   
也就是在前面多出了一个byte的00，导致解释失败：Command not implemented: REQUEST: ?UNKNOWN? (length=0 id=0xb000001)    
3.  待完成：  
- 搞清楚上面的问题  
- 协议实现  
- 服务器推送与回复转发

## 参考
1. [Android远程调试的探索与实现](https://tech.meituan.com/2017/07/20/android-remote-debug.html)    
2. [深入 Java 调试体系第 3 部分:JDWP 协议及实现](https://www.ibm.com/developerworks/cn/java/j-lo-jpda3/index.html)
3. [Java Debug Wire Protocol Specification Details](https://download.oracle.com/otn_hosted_doc/jdeveloper/904preview/jdk14doc/docs/guide/jpda/jdwp-protocol.html#JDWP_VirtualMachine_Version)
4. [ndk_dlopen](https://github.com/Rprop/ndk_dlopen)
